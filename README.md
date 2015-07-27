# 2015

May 14, 2015
------------
Errors on GPS/IMU board:
-IMU RX and TX lines crossed - need IMU RX into FTDI TX and IMU TX into FTDI RX
-TEST pin on FTDI must be connected to ground

Errors corrected in Eagle files. Solder bridge added on board to ground TEST pin. RX and TX lines will be swapped on IMU interconnect cable to board. 


Successfully tested channel 2 of new front end with pulser input. Verified timing pulse, preamplifier pulse, amplified pulse (A206 out), and discriminator pulse. Next, need to interface with A/D and begin testing peak detector.




May 15, 2015
------------
SPI configuration for peak detector A/D:
-MSB first
-8 MHz clock (clock divider = 2)
-Clock polarity = 0, clock phase = 0 (SPI mode 0)
-Data addresses: 000X = channel 0; 001X = channel 1; 010X = channel 2; 011X = channel 3; 1111 = temperature
-Manual mode register (address 0x04): bit 7 = 0; bits 6:5 = channel select; bit 4 = X; bits 3:1 = range select (0 to 10 V = 101, or to use as configured = 000); bit 0 = select temperature sensor (overrides channel and range select). Thus:
Channel 0 read = 000X1010 or 000X0000 if range pre-configured
Channel 1 read = 001X1010 or 001X0000
Channel 2 read = 010X1010 or 010X0000
Channel 3 read = 011X1010 or 011X0000
Temperature read = 0XXXXXX1
-Configuration registers:
Range select = 01010XXX (0 to 10 V)
Channel 0 range address = 0x10
Channel 1 range address = 0x11
Channel 2 range address = 0x12
Channel 3 range address = 0x13

Internal control register address = 0x06
Bits 7:4 = 0
Bit 3 = AL_PD control
Bit 2 = internal Vref enable
Bit 1 = temperature sensor enable
Bit 0 = 0
Thus, write: 0000X110
-Programming sequence:
>Device powers up
>Set control register: write 0000X110 to address 0x06
>Set range select registers: write 01010XXX to 0x10, 0x11, 0x12, 0x13
>Write manual mode register for desired channel reads
-SCLK: brown; MOSI: orange; MISO: yellow; CS: red

SPI interfacing with A/D successfully tested. Temperature and channel reads working correctly. Was not able to set channel ranges using range registers, but including the range bits in each manual mode read command works.

All four channels tested and confirmed operational with pulser input. All output pulses are present and change appropriately with different input pulse heights. A/D can accurately read all four peak detector channels. Still need to test peak detector resets, fine tune gains and discriminator thresholds for each channel, and test with real detector input.




July 26th, 2015
---------------
The following minor fixes were required to allow proper operation of the preprocessor board hardware. These changes should be implemented directly in a future revision of the board if further use of the design is desired.

-ATmega32u4 VBUS pin (pin 7) must be connected to +5V.
-FIFO FL/RT pin must be connected to +5V.
-Unused FIFO input pin (D8) should be connected to +5V or GND. Not required but recommended. 2015 fix ties D8 to GND.