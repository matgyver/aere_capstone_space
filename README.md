# AerE 4610/4620 Space Design Capstone
This repo contains the slides and example code used for showing students how to use GPIO pins on a Raspberry Pi 5. Examples are provided for both Python and C/C++. The relay board is used to control valves that can be used to move a simulated spacecraft in the underwater tank. 

## Requirements
The Python examples use *gpiozero* to perform all of the low level work with RPi 5 GPIO chip (RP1). For C/C++, the examples use *libgpiod*. Both of these should be installed on the Raspberry Pi 5 with more recent versions of Raspberry Pi OS. However, they can also installed using the commands below.

```bash
sudo apt update
sudo apt install python3-gpiozero
sudo apt install libgpiod-dev gpiod
```

All Python scripts were tested with Python version 3.11.2 and should work with 3.11 or higher.

# Usage

## Python
Any of the Python scripts can be run in the terminal. It is highly recommended you do not use an IDE like Thonny. This may cause issues and then lockout the GPIO pins from other programs from accessing them. To run these in the terminal, use can use the following:

```bash
python gpio.py
```
or
```bash
python3 gpio.py
```
If you get an error about accessing the GPIO pins, try using sudo.

```bash
sudo python gpio.py
```

## C/C++
Any C/C++ code will need to be compiled. These programs are typically fairly small, so compile time should be very short. The examples I did were all compiled with GCC. G++ could also be used, just rename the files to *cpp* extension and then run with g++. Since we are using the *libgpiod* library, you must link your code to that library during compiling. You can use the following to compile the examples I have.

```bash
gcc gpio.c -lgpiod -o gpio
```

To run this, in the terminal, enter this.

```bash
sudo ./relay_wasd_toggle
```