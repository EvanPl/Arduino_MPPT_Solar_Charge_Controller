# Arduino MPPT Solar Charge Controller
An Arduino based Maximum Power Point Tracking (MPPT) solar charge controller circuit, capable of charging
a 2V sealed lead acid battery and driving the following two loads:
* A 2V LED
* A Mobile Phone (rated at 5V down to 0.5A).
## &#x1F537; `System Specifications` 
* Solar Panel Power = 5W
* Solar Panel Open Circuit Voltage (Voc) = 10.75V (Under STC)
* Battery Voltage = 2V
* Battery Capacity = 2.5Ah
* Switching Frequency = 50kHz
## &#x1F537; `Requirements for this Project`
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
## &#x1F537; `Solar Panel and Battery voltages sensing`
In order to sense the solar panel voltage a voltage divider was used, as this voltage can be greater than 5V which is the maximum voltage that can be applied to the Arduino. On the other hand, the maximum battery voltage, as described in the datasheet, is approximately 2.25V and thus we directly read it using a Kelvin connection (as shown in the schematic) for better accuracy.
## &#x1F537; `Buck Converter design`
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
The MOSFET selected in the design can withstand both the maximum load current (approx. 3A) as well as the maximum voltage which is 10.75V.
## &#x1F537; `Phone Charger`
The circuit is capable of charging a mobile phone (at approximately 5V, 0.5A), whenever the user wants it, as long as the voltage across the 2V sealed lead acid battery is greater or equal to 2V (remember that the battery is fully charged at 2.25V). For this purpose, a HALJIA 5V 600mA USB DC to DC Booster Power Supply Board Module is used.
## &#x1F537; `LCD display`
An IC/I2C/TWI Serial 2004 20x4 LCD is used in this project for displaying circuit data in real time. An example of the the data displayed on the LCD is shown in the Figure below.
## PLACE LCD DISPLAY PICTURE HERE -- in MPPT mode
The **first** column, displays the voltage across the solar panel as well as its output current and power (in W).

The **second** column, displays the battery voltage, its charging state (as explained in the "*Circuit Operation* section") together with the percentage of the battery.

The **third** column, shows the PWM of the PMOS of the buck converter (in %) as well as the status of the 2V LED load.

Note, that in case the user selects to charge a mobile phone then the message *Charging Phone* is displayed, if the battery voltage is greater or equal to 2V, otherwise the message *Not sufficient battery capacity to charge the phone* is printed on the LCD.
## &#x1F537; `Monitoring the system over the internet`
With the use of the Ethernet Shield 2, and ThingSpeeak open IoT platform, the **solar panel output power** are plotted on ThingSpeeak open IoT platform on the internet. Note that, the arduino code can be modified to sent more data for plotting online, but this makes the circuit slower, as there is a minimum time required (approximately 15s) for each of those data to be uploaded on the internet.
## &#x1F537; `Python for plotting the solar panel and battery voltages`
Plotting the solar panel and battery voltages in real time is achieved with Python. Note that, Python is communicating with the Arduino serially (with a baud rate of 115200).
## &#x1F537; `Controlling the circuit over the phone`
The user of this system has the ability to control the circuit over the mobile phone (remotely). This is achieved by using an application named *Blynk* (available both on the App and Google/Android stores) which communicates with the Arduino via the Ethernet Shield. There are three different buttons/options:
* **Reset** button. If pressed, the user remotely resets the circuit.
* **MPPT** button. If pressed, the circuit operates as a MPPT (driving the 2V LED load as explained in the "*Circuit Operation* section" )
* **Phone** button. if pressed and there is sufficient battery capacity the circuit charges a mobile phone.

The Blynk application with those three buttons is shown below.
## GIVE IMAGE OG BLYNK APP with the 3 buttons
## &#x1F537; `Circuit Operation`
There are three circuuit states:
#### 1. Reset
State, for resetting the circuit. The circuit enters this state when the user either presses the *Reset* button on the circuit or on the phone (as mentioned in the *Controlling the circuit over the phone* section)
#### 2. MPPT
State, in which the circuit operates as a maximum power point tracking solar charge controller driving a 2V LED load. The circuit enters this state when the user either presses the *MPPT* button on the circuit or on the phone (as mentioned in the *Controlling the circuit over the phone* section). This charge controller operates as follows:
* The circuit reads the solar panel and battery voltages, the solar panel output current and power (*read_data* function).
* Then, the circuit decides on which sub state to enter (*mode_select* function). There are five different sub states given below:

   1. ***no_bat***. This is the sub state entered when the measured battery voltage is lower than the minimum one (1.6V). In such case, both      the buck converter and the loads are off, waiting for an appropriate battery to be connected.

   2. ***error/>Max***. In this case, a battery of higher voltage than the maximum (2.25V) is connected to the circuit and thus the same electrical conditions as with the *no_bat* sub state are applied.

   3. ***no_sun***. In this case there is no sufficient sunlight to charge the battery (or remain it fully charged) and thus the buck      converter is OFF. As far as the 2V LED load is concerned, if the battery voltage is between its minimum and maximum values then this load   will turn ON, discharging the battery, something which, apart from the LCD, is indicated by a red LED turned on (powered from the Arduino). Clearly, if the battery is discharged below 1.6V, then we enter *no_bat* sub state, waiting for the solar intensity to increase sufficiently and recharge the battery.

   4. ***bulk***. This is the first of the two charging sub states and the one where the the MPPT algorithm, **Perturb and observe**, takes place. The purpose of this algorithm (whose flowchart is shown in Figure 3, below) is to continuously change the PWM signal applied to the MOSFET of the buck converter and lock to the one which provides the largest output solar power (which is also being tracked continuously). Figure 4, below, shows a typical power versus voltage curve of a solar panel where the maximum power point is at its peak (between points 2 and 3). Figure 5, shows the power and current versus volgage of the solar panel used in this system, as obtained after putting a variable resistor across its terminal and monitoring its output voltage and current. As it can be seen from the graph, under the tested solar intensity conditions, the maximum power that can be extracted from the panel is 518.3mW.

The bulk sub state occurs when there is sufficient sunlight and the battery voltage is between 1.6V and 2V (float voltage). Last but not least, the initial PWM signal used in the Perturb and Observe algorithm is 30%.

  ![alt text](https://github.com/EvanPl/Arduino-MPPT-Solar-Charge-Controller/blob/master/Images/Preturb%20and%20Observe%20Flowchart%20and%20P%20vs.%20V%20for%20a%20solar%20panel.PNG)
  ![alt text](https://github.com/EvanPl/Arduino-MPPT-Solar-Charge-Controller/blob/master/Images/Power%20and%20Current%20vs.%20Voltage%20of%20the%20panel%20used%20in%20this%20project.PNG)
    
   5. ***Float***. In this charging sub state the voltage on the battery is greater or equal to 2V (which means that the battery is 97.7%, or more, charged) and there is sufficient sunlight to charge the battery. As soon as we enter this state the PWM signal is set to 30%, to allow the solar panel to either charge the battery in a slower rate or keep it fully charged at all times.
   
   **Note that, in both charging states (bulk and Float) a green LED turns on (powered from the Arduino), indicating that the battery is charging.**
   
   **Moreover, each time a new PWM signal in the MOSFET of the buck converter is applied, a delay of three seconds follows to ensure that the circuit settles electrically. Also, individual functions are used for uploading data online (*up_online* function), controlling the LED 2V load (*load_state* function),  printing data on the LCD (*lcd_print* function), controlling the LED (powered from the Arduino) charging indicators (*led_out* function) and plotting voltages using python (*print_for_python* function).**   
#### 3. Phone Charging
In this state the circuit charges a mobile phone as explained in the *Phone Charger* section. The circuit enters this state when the user either presses the *Phone Charger* button on the circuit or the *Phone* button the phone (as mentioned in the *Controlling the circuit over the phone* section). Note that, in this state the both the buck converter and the 2V LED load are OFF.
## &#x1F537; `Soldering and Enclosure`
## TO BE COMPLETED.
