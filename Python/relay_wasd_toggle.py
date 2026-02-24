'''
Toggle based on Keyboard
Turn on GPIO0..GPIO3 using WASD keyboard
Prof. Nelson
'''
import sys
import termios
import tty
import select
from gpiozero import LED

# ---- Configuration ----
RELAY_PINS = list(range(24))   # GPIO0–GPIO23
ACTIVE_LOW = False             # change to True if relay board is active LOW

# Create relay objects
relays = [LED(pin, active_high=not ACTIVE_LOW) for pin in RELAY_PINS]

# Map keyboard keys to relay indices
key_map = {
    'w': 0,
    'a': 1,
    's': 2,
    'd': 3
}

# Track state of each relay
relay_state = [False] * len(relays)

# ---- Make sure all relays are turned OFF ----
# ---- Terminal setup ----
old_settings = termios.tcgetattr(sys.stdin)
tty.setcbreak(sys.stdin.fileno())

print("Startuip reset: all relays OFF")
print("WASD toggle demo controlling GPIO0–GPIO3")
print("Press Ctrl+C to exit")

try:
    while True:
        if select.select([sys.stdin], [], [], 0)[0]:
            key = sys.stdin.read(1).lower()

            if key in key_map:
                idx = key_map[key]

                # Toggle state
                relay_state[idx] = not relay_state[idx]

                if relay_state[idx]:
                    relays[idx].on()
                    print(f"Relay {idx} ON")
                else:
                    relays[idx].off()
                    print(f"Relay {idx} OFF")

finally:
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)

    # Safety: turn everything off
    for r in relays:
        r.off()
