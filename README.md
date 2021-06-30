# SCuM Programmer

Load this firmware onto an nRF52840-DK to turn it into a programmer for SCuM!

The Single Chip micro-Mote (SCuM) is a 2x3mm2 single-chip standard-compatible Smart Dust chip, see https://www.crystalfree.org/

## Use

### program the nRF52840-DK

_Note_: you only need to do this once.

- download `scum-programmer.hex` from the https://github.com/openwsn-berkeley/SCuM-programmer/releases/latest/
- plug in your nRF52840-DK into your computer, drive "JLINK" appears
- drag-and-drop `scum-programmer.hex` onto this drive
- when the LEDs of the board go round-and-round, you're set!

![](static/round_and_round.gif)

### interact with SCuM's serial port

* Connect SCuM's UART to the following pins on the nRF52840-DK

| DK      | SCuM                     |
| ------- | ------------------------ |
| `P0.02` | UART TX (SCuM transmits) |
| `P0.26` | UART RX (SCuM receives)  |
| `GND`   | `GND`                    |

* open the serial port corresponding to your nRF52840-DK using a serial terminal (e.g. TeraTerm), using **19200 baud**.

### load code onto SCuM

_Coming soon!_

### calibrate SCuM

_Coming soon!_

# Build

- install SEGGER Embedded Studio for ARM (Nordic Edition)
- open `scum-programmer/scum-programmer.emProject`
