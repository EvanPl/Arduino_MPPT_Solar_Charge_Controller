# Arduino MPPT Solar Charge Controller with Phone charging capability
An arduino based Maximum Power Point Tracking (MPPT) solar charge controller, capable of charging
a 2V sealed lead acid battery and driving the following two loads:
* A 2V LED
* A Mobile Phone (rated at 5V down to 0.5A).
## System Specifications
* Solar Panel Power = 5W
* Solar Panel Open Circuit Voltage (Voc) = 10.75V
* Battery Voltage = 2V
* Battery Capacity = 2.5Ah
* Switching Frequency = 50kHz
## Requirements for this Project
* Arduino Uno
* Arduino Ethernet Shield 2
* Ethernet cable
* 2V Sealed Lead Acid battery
* 5W Solar Panel
* IC/I2C/TWI Serial 2004 20x4 LCD
* Adafruit INA219 Current Sensor Breakout
* The reset of components can be found from the schematic below
## PLACE SCHEMATIC PICTURE HERE
# Solar Panel and Battery voltages sensing
In order to sense the solar panel voltage a voltage divider was used, as this voltage can be greater than 5V which is the maximum voltage that can be applied to the arduino. On the other hand, the maximum battery voltage, as described in the datasheet, is approximately 2.25V and thus we directly read it using a Kelvin connection (as shown in the schematic) for better accuracy.
# Buck Converter design
The main part of the MPPT charge controller, is, as seen in the schematic, its buck converter.
For this project, the frequency of that converter was set to be 50kHz. Note that, in general, the
higher the switching frequency the smaller the size of the converter's capacitor and inductor, but
the higher the switching losses. At this stage, the calculation of the inductor and the capacitor is described below:
### Inductor Calculation
Assuming a 0V diode drop and that the switch (the PMOS of the buck converter) is an ideal one (zero ON resistance, infinite OFF resistance and zero switching time), in order to maintain the converter in continuous mode it is true that:
![alt text](https://github.com/EvanPl/Arduino-MPPT-Solar-Charge-Controller/blob/master/Images/Inductor%20Calculation.PNG)

Note that a much bigger inductor than that found from the equation was selected as the inductor size controls the current slope. As a result, a smaller inductor means that the circuit is closer to discontinuous mode (which is undesired here).
### Capacitor Calculation
The capacitor is chosen to keep the ripple of the output voltage Vo to an acceptable value. Typically, ΔVο (peak-to-peak output voltage ripple) is limited to about 5% of the nominal output voltage. Thus, it is true that:
![alt text](https://github.com/EvanPl/Arduino-MPPT-Solar-Charge-Controller/blob/master/Images/Capacitor%20Calculation.PNG)
### MOSFET Selection
The MOSFET selected in the design can withdstand both the maximum load current (approx. 3A) as well as the maximum voltage which is 10.75V.
