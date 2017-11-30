# Arduino MPPT Solar Charge Controller
An arduino based Maximum Power Point Tracking (MPPT) solar charge controller circuit, capable of charging
a 2V sealed lead acid battery and driving the following two loads:
* A 2V LED
* A Mobile Phone (rated at 5V down to 0.5A).
## System Specifications 
---
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
* HALJIA 5V 600mA USB DC to DC Booster Power Supply Board Module
* The reset of components can be found from the schematic below
## PLACE SCHEMATIC PICTURE HERE
## Solar Panel and Battery voltages sensing
In order to sense the solar panel voltage a voltage divider was used, as this voltage can be greater than 5V which is the maximum voltage that can be applied to the arduino. On the other hand, the maximum battery voltage, as described in the datasheet, is approximately 2.25V and thus we directly read it using a Kelvin connection (as shown in the schematic) for better accuracy.
## Buck Converter design
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
## Phone Charger
The circuit is capable of charging a mobile phone (at approximately 5V, 0.5A), whenever the user wants it, as long as the voltage across the 2V sealed lead acid battery is greater or equal to 2V (remember that the battery is fully charged at 2.25V). For this purpose, a HALJIA 5V 600mA USB DC to DC Booster Power Supply Board Module is used.
## LCD display
An IC/I2C/TWI Serial 2004 20x4 LCD is used in this project for displaying circuit data in real time. An example of the the data displayed on the LCD is shown in the Figure below.
## PLACE LCD DISPLAY PICTURE HERE -- in MPPT mode
The **first** column, displays the voltage across the solar panel as well as its output current and power (in W). The 
**second** column, displays the battery voltage, its charging state (as explained in the "*Circuit Operation* section") together with the percentage of the battery.
The **third** column, shows the PWM of the PMOS of the buck converter (in %) as well as the status of the 2V LED load.
Note, that in case the user selects to charge a mobile phone then the message *Charging Phone* is dispayed, if the battery voltage is 
greater or equal to 2V, otherwise the message *Not sufficient battery capacity to charge the phone* is printed on the LCD.
## Monitoring the system over the internet
With the use of the Ethernet Shield 2, and ThingSpeeak open IoT platform, the **solar panel output power** are plotted on ThingSpeeak open IoT platform on the internet. Note that, the arduino code can be modified to sent more data for plotting online, but this makes the circuit slower, as there is a minimum time required (approximately 15s) for each of those data to be uploaded on the internet.
## Python for plotting the solar panel and battery voltages
Plotting the solar panel and battery voltages in real time is achieved with Python. Note that, Python is communicating with the Arduino serially (with a baud rate of 115200).
## Controlling the circuit over the phone.
The user of this system has the ability to control the circuit over the mobile phone (remotely). This is achieved by using an application named *Blynk* (available both on the App and Google/Android stores) which communicates with the Arduino via the Ethernet Shield. There are three different buttons/options:
* Reset button. If pressed, the user remotely resets the circuit.
* MPPT button. If pressed, the circuit operates as a MPPT (driving the 2V LED load as explained in the "*Circuit Operation* section" )
* Phone button. if pressed and there is sufficient battery capacity the circuit charges a mobile phone.
The Blynk application with those three buttons is shown below.
## GIVE IMAGE OG BLYNK APP with the 3 buttons
## Soldering and Enclosure
## TO BE COMPLETED.

