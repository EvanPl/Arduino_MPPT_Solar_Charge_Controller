# Arduino MPPT Solar Charge Controller with Phone charging capability
An arduino based Maximum Power Point Tracking (MPPT) solar charge controller, capable of charging
a 2V sealed lead acid battery and driving the following two loads:
* A 2V LED
* A Mobile Phone (rated at 5V down to 0.5A).
## System Specifications
* Solar Panel Power = 5W
* Battery Voltage = 2V
* Maximum Current = GIVE IT
* 
## Requirements for this Project
* Arduino Uno
* Arduino Ethernet Shield 2
* 2V Sealed Lead Acid battery
* 5W Solar Panel
* IC/I2C/TWI Serial 2004 20x4 LCD
* Adafruit INA219 Current Sensor Breakout
* The reset of components can be found from the schematic below
## PLACE SCHEMATIC PICTURE HERE
# Buck Converter design
The main part of the MPPT charge controller, is, as seen in the schematic, its buck converter.
For this project, the frequency of that converter was set to be 50kHz. Note that, in general, the
higher the switching frequency the smaller the size of the converter's capacitor and inductor, but
the higher the switching losses. NOW GIVE THE FIRMULAS USED TO DERIVE THE INDUCTOR AND THE CAPACITOR.
