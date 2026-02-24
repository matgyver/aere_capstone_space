'''
Relay Cycle
Iterate through each one using a for loop
Prof. Nelson
'''
from gpiozero import LED
from time import sleep

RELAY_PINS = range(24)
DELAY = 1

relays = [LED(pin) for pin in RELAY_PINS]

def cycle_relays():
    
    '''
    Function: Cycle Relays
    Input: none
    Returns: None
    Iterates through each relay on the board
    '''
    for relay in relays:
        relay.on()
        sleep(DELAY)
        relay.off()

while True:
    cycle_relays()
