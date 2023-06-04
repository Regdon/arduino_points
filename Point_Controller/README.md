# Point Controller
An Adruino script to act as the brains for the points system. It reacts to user input, displays the state of each point and sends i2c commands to the relevent Point Node

## Pin Out
### Analog
Analog Listen: A0 - to detect user input
PWM Listen: A1 - to adjust route LED brightness

SDL: A4 - i2c
SCL: A5 - i2c

### Digital
SER_IN: 8 - Shift Register
SRCK: 9 - Shift Register 
RCK: 10 - Shift Register
G: 11 - Shift Register

## RESISTANCES[]
These are analog read measurements taken from A0 for each of the voltage divider inputs.

## SHIFT_POSITIONS[]
To account for an error in the PCB, this is used to manipulate the positions of the individual bits passed to the shift registers