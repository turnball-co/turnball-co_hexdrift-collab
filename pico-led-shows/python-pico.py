from machine import Timer, PWM
import math, random

# Setup PWM channels
pwm1 = [
    PWM(1, freq=random.randint(55,100)*1000, duty_u16=8192),
    PWM(5, freq=random.randint(55,100)*1000, duty_u16=8192),
    PWM(9, freq=random.randint(55,100)*1000, duty_u16=8192)
]

pwm2 = [
    PWM(13, freq=random.randint(70,100)*1000, duty_u16=8192),
    PWM(28, freq=random.randint(70,100)*1000, duty_u16=8192),
    PWM(22, freq=random.randint(70,100)*1000, duty_u16=8192),
    PWM(18, freq=random.randint(70,100)*1000, duty_u16=8192)
]

# Global states for pulse progress
pulse_index1 = 0
pulse_index2 = 0

def pulse_step(pwm_obj, index, min_brightness=30000, max_brightness=60000):
    """
    One step of a sine-wave pulse constrained to a brightness range.
    min_brightness and max_brightness are duty_u16 values (0–65535).
    """
    # Normal sine wave between 0 and 1
    sine_val = (math.sin(index / 6 * math.pi) + 1) / 2  
    
    # Scale sine wave into [min_brightness, max_brightness]
    duty = int(min_brightness + sine_val * (max_brightness - min_brightness))
    pwm_obj.duty_u16(duty)

def BlinkLED(timer):
    global pulse_index1
    bulb = random.choice(pwm2)
    pulse_step(bulb, pulse_index1)
    pulse_index1 = (pulse_index1 + 1) % 12   # loop back after 20 steps

def BlinkLED2(timer):
    global pulse_index2
    bulb = random.choice(pwm1)
    pulse_step(bulb, pulse_index2)
    pulse_index2 = (pulse_index2 + 1) % 24
    
# Example: start LEDs at max brightness
for bulb in pwm1 + pwm2:
    bulb.duty_u16(60000)   # start bright

# Initialize timers to run concurrently
timer_one = Timer()
timer_three = Timer()

# Run both timers periodically without blocking
timer_one.init(freq=10, mode=Timer.PERIODIC, callback=BlinkLED)   # 10 Hz pulse steps
timer_three.init(freq=15, mode=Timer.PERIODIC, callback=BlinkLED2) # 15 Hz pulse steps
