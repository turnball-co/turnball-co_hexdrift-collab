from machine import Timer, PWM
import board, time, math
#import pwmio
import random
import array as arr

pwm = [PWM(1, freq=random.randint(5,100)*1000, duty_u16=8192) #/random.randint(1,3))
       , PWM(5, freq=random.randint(5,100)*1000, duty_u16=8192) #/random.randint(1,3))
       , PWM(9, freq=random.randint(5,100)*1000, duty_u16=8192) #/random.randint(1,3))
       , PWM(13, freq=random.randint(5,100)*1000, duty_u16=8192) #/random.randint(1,3))
       , PWM(28, freq=random.randint(5,100)*1000, duty_u16=8192) #/random.randint(1,3))
       , PWM(22, freq=random.randint(5,100)*1000, duty_u16=8192) #/random.randint(1,3))
       , PWM(18, freq=random.randint(5,100)*1000, duty_u16=8192)] #/random.randint(1,3))]
# leds = [PWM.PWMOut(board.GP1, frequency=1000),
# PWM.PWMOut(board.GP5, frequency=1000),
# PWM.PWMOut(board.GP9, frequency=1000),
# PWM.PWMOut(board.GP13, frequency=1000),
# PWM.PWMOut(board.GP28, frequency=1000),
# PWM.PWMOut(board.GP22, frequency=1000),
# PWM.PWMOut(board.GP18, frequency=1000)]

# a = [Pin(1, Pin.OUT), Pin(5, Pin.OUT), Pin(9, Pin.OUT), Pin(13, Pin.OUT), Pin(28, Pin.OUT), Pin(22, Pin.OUT), Pin(18, Pin.OUT)]
# leds = arr.array('leds', a)
get_voltage = 3.3 / 65535

timer_one = Timer()
timer_two = Timer()
state = 1
def pulse(l, t):
    print(l)
    for i in range(20):
        l.duty_u16(int(math.sin(i / 10 * math.pi) * 500 + 500))
        time.sleep_ms(t)

def BlinkLED(timer_one):
    freq = random.randint(5000,62500)
    duty = random.randint(0, 63000)
    bulb = pwm[random.randint(0,len(pwm)-1)]
    print(bulb)
    pulse(bulb, 500)
    bulb.deinit()
#     playMe.freq(freq)
#     playMe.duty_u16(duty)
    bulb.init()
#    pwm[bulb].init()
    
def ChangeState(timer_two):
    global state
    timer_one.init(freq=0.02, mode=Timer.PERIODIC, callback=BlinkLED)
    
    if state == 1:
     state = state+1
    elif state == 2:
     state = state+1
    elif state == 3:
     state = 1
    else:
      state = 1
      timer_one.init(freq=0.06, mode=Timer.PERIODIC, callback=BlinkLED)
 
# Initialize the timer one for first time
timer_one.init(freq=random.randint(1,6), mode=Timer.PERIODIC, callback=BlinkLED)
state = state+1

timer_two.init(freq=0.08, mode=Timer.PERIODIC, callback=ChangeState)
