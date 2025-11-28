#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h>

#define LEFT_SIGNAL_PIN 2
#define RIGHT_SIGNAL_PIN 5
#define JOYSTICK_X 27   // GP27 = ADC1
#define JOYSTICK_Z 14
#define BLINK_DELAY 500
#define HALL_SENSOR_PIN 15
#define WHEEL_CIRCUMFERENCE 1277
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define LCD_ADDR 0x27
#define TIMEOUT_MS 3000

volatile uint64_t last_pulse_time = 0;
volatile float wheel_speed = 0.0f;

// --- LCD helper functions (from earlier) ---
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_init();
void lcd_clear();
void lcd_set_cursor(int row, int col);
void lcd_print(const char *str);

// Hall sensor ISR (only once!)
void hall_sensor_isr(uint gpio, uint32_t events) {
    uint64_t now_us = time_us_64();
    if (last_pulse_time != 0) {
        uint64_t delta = now_us - last_pulse_time;
        float delta_sec = delta / 1e6f;
        wheel_speed = WHEEL_CIRCUMFERENCE / delta_sec;
    }
    last_pulse_time = now_us;
}


#define LCD_ADDR 0x27   // I2C address of the LCD backpack
#define LCD_CHR  1      // Mode - Sending data
#define LCD_CMD  0      // Mode - Sending command

#define LCD_BACKLIGHT 0x08  // On
#define ENABLE 0x04         // Enable bit

// Low-level write
void lcd_write_byte(uint8_t data) {
    i2c_write_blocking(i2c0, LCD_ADDR, &data, 1, false);
}

// Toggle enable pin
void lcd_toggle_enable(uint8_t data) {
    sleep_us(500);
    lcd_write_byte(data | ENABLE);
    sleep_us(500);
    lcd_write_byte(data & ~ENABLE);
    sleep_us(500);
}

// Send command or data
void lcd_send(uint8_t data, uint8_t mode) {
    uint8_t high = mode | (data & 0xF0) | LCD_BACKLIGHT;
    uint8_t low  = mode | ((data << 4) & 0xF0) | LCD_BACKLIGHT;

    lcd_write_byte(high);
    lcd_toggle_enable(high);

    lcd_write_byte(low);
    lcd_toggle_enable(low);
}

// Public wrappers
void lcd_send_cmd(uint8_t cmd) {
    lcd_send(cmd, LCD_CMD);
}

void lcd_send_data(uint8_t data) {
    lcd_send(data, LCD_CHR);
}

// Initialize LCD
void lcd_init() {
    sleep_ms(50); // wait for LCD power up

    lcd_send_cmd(0x33); // Initialize
    lcd_send_cmd(0x32); // Set to 4-bit mode
    lcd_send_cmd(0x06); // Cursor move direction
    lcd_send_cmd(0x0C); // Turn cursor off
    lcd_send_cmd(0x28); // 2 line display
    lcd_send_cmd(0x01); // Clear display
    sleep_ms(5);
}

// Clear display
void lcd_clear() {
    lcd_send_cmd(0x01);
    sleep_ms(2);
}

// Set cursor position
void lcd_set_cursor(int row, int col) {
    int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_send_cmd(0x80 | (col + row_offsets[row]));
}

// Print string
void lcd_print(const char *str) {
    for (int i = 0; i < strlen(str); i++) {
        lcd_send_data(str[i]);
    }
}

int main() {
    stdio_init_all();

    // Initialize GPIOs for LEDs
    gpio_init(LEFT_SIGNAL_PIN);
    gpio_set_dir(LEFT_SIGNAL_PIN, GPIO_OUT);

    gpio_init(RIGHT_SIGNAL_PIN);
    gpio_set_dir(RIGHT_SIGNAL_PIN, GPIO_OUT);

    // Initialize joystick press-down (digital, active-low assumed)
    gpio_init(JOYSTICK_Z);
    gpio_set_dir(JOYSTICK_Z, GPIO_IN);
    gpio_pull_up(JOYSTICK_Z);

    // Initialize ADC for JOYSTICK_X
    adc_init();
    adc_gpio_init(JOYSTICK_X);   // enable ADC function on pin
    adc_select_input(1);         // GP27 is ADC1 (GP26=ADC0, GP27=ADC1, GP28=ADC2)

    bool left_on = false;
    bool right_on = false;
    bool left_state = false;
    bool right_state = false;

    uint32_t last_left_toggle = 0;
    uint32_t last_right_toggle = 0;
    // Init GPIO for hall sensor
    gpio_init(HALL_SENSOR_PIN);
    gpio_set_dir(HALL_SENSOR_PIN, GPIO_IN);
    gpio_pull_up(HALL_SENSOR_PIN);
    gpio_set_irq_enabled_with_callback(HALL_SENSOR_PIN, GPIO_IRQ_EDGE_FALL, true, &hall_sensor_isr);

    // Init I2C
    i2c_init(I2C_PORT, 100 * 1000); // 100kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Init LCD
    lcd_init();
    lcd_clear();

    while (true) {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        uint64_t now_us = time_us_64();
  
        // Read analog voltage from joystick X
        uint16_t raw = adc_read();                // 0–4095
        float voltage = (raw * 3.3f) / 4095.0f;   // convert to volts

        // Read press-down button
        bool press_down = (gpio_get(JOYSTICK_Z) == 0);

        // Decide left/right based on voltage thresholds
        bool left_pressed  = (voltage < 0.2f);    // near 0 V
        bool right_pressed = (voltage > 3.0f);    // near 3.3 V

        // Handle joystick input
        if (left_pressed) {
            left_on = true;
            right_on = false;
        } else if (right_pressed) {
            right_on = true;
            left_on = false;
        } else if (press_down) {
            left_on = false;
            right_on = false;
        }

        // Blink logic (non-blocking)
        if (left_on) {
            gpio_put(RIGHT_SIGNAL_PIN, 0);
            if (now_ms - last_left_toggle >= BLINK_DELAY) {
                left_state = !left_state;
                gpio_put(LEFT_SIGNAL_PIN, left_state);
                last_left_toggle = now_ms;
            }
        } else if (right_on) {
            gpio_put(LEFT_SIGNAL_PIN, 0);
            if (now_ms - last_right_toggle >= BLINK_DELAY) {
                right_state = !right_state;
                gpio_put(RIGHT_SIGNAL_PIN, right_state);
                last_right_toggle = now_ms;
            }
        } else {
            gpio_put(LEFT_SIGNAL_PIN, 0);
            gpio_put(RIGHT_SIGNAL_PIN, 0);
        }

        if (last_pulse_time == 0 || (now_us - last_pulse_time) > (TIMEOUT_MS * 1000)) {
            wheel_speed = 0.0f;
        }

        lcd_set_cursor(0, 0);
        lcd_print("Speed:");
        lcd_set_cursor(1, 0);
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2f km/h", wheel_speed * 3.6f);
        lcd_print(buffer);

        sleep_ms(10);
    }
}
