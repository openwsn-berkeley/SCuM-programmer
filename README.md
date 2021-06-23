# SCuM Programmer

Load this firmware onto an nRF52840-DK to turn it into a programmer for SCuM!

The Single Chip micro-Mote (SCuM) is a 2x3mm2 single-chip standard-compatible Smart Dust chip, see https://www.crystalfree.org/

## Use

### program the nRF52840-DK

- download `scum-programmer.hex` from the https://github.com/openwsn-berkeley/SCuM-programmer/latest 
- plug in your nRF52840-DK into your computer, drive "JLINK"
- drag-and-drop `scum-programmer.hex` onto this drive
- the LEDs of the board go round and around

![](static/round_and_round.gif)

### interact with SCuM's serial port

_Coming soon!_

### load code onto SCuM

_Coming soon!_

### calibrate SCuM

_Coming soon!_

# Build

- install SEGGER Embedded Studio for ARM (Nordic Edition)
- open `scum-programmer/scum-programmer.emProject`