from machine import Timer, PWM
import math, random

# Setup PWM channels
pwm1 = [
    PWM(1, freq=random.randint(35,100)*1000, duty_u16=8192),
    PWM(5, freq=random.randint(35,100)*1000, duty_u16=8192),
    PWM(9, freq=random.randint(35,100)*1000, duty_u16=8192)
]

pwm2 = [
    PWM(13, freq=random.randint(55,100)*1000, duty_u16=8192),
    PWM(28, freq=random.randint(55,100)*1000, duty_u16=8192),
    PWM(22, freq=random.randint(55,100)*1000, duty_u16=8192),
    PWM(18, freq=random.randint(55,100)*1000, duty_u16=8192)
]

# Global states for pulse progress
pulse_index1 = 0
pulse_index2 = 0

def pulse_step(pwm_obj, index):
    """One step of a sine-wave pulse"""
    duty = int(math.sin(index / 6 * math.pi) * 500 + 500)
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

# Initialize timers to run concurrently
timer_one = Timer()
timer_three = Timer()

# Run both timers periodically without blocking
timer_one.init(freq=10, mode=Timer.PERIODIC, callback=BlinkLED)   # 10 Hz pulse steps
timer_three.init(freq=15, mode=Timer.PERIODIC, callback=BlinkLED2) # 15 Hz pulse steps
