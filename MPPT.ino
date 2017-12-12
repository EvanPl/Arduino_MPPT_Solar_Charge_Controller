#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> //allows for communication with the I2C library
#include <Adafruit_INA219.h>
#include "ThingSpeak.h"
#include <SPI.h> //for communicating with Python
#include <Ethernet2.h>
#include <BlynkSimpleEthernet2.h>
char auth[] = "ab4fe6f03bde4991a1ac35a820ec3cf2";


byte mac[]={0x90,0xA2,0xDA,0x11,0x24,0x9E}; //for storing and displaying the data online(on thingspeak)
EthernetClient client; //for storing and displaying the data online(at thingspeak)
unsigned long myChannelid=374049; //this is the channel ID used for communication with ThingSpeak
char *myWriteAPIKey="LRFYHE5HP5UWWAAL"; // this API key is used to route data to my channel

#define Vb_float 2.0 //this is the float voltage of the batter
#define Vb_max 2.25 //this is the maximum battery voltage
#define Vb_min 1.6 //this is the minimum battery voltage
#define PWM_INC 1 //used in the Preturb and observe algorithm for tracking the MPPT point.
#define PWM_MIN 0 //in %
#define PWM_PIN 9 //Q1 MOSFET is connected to this pin
#define load 8 // load is connected to this pin
#define MPPT 6 // "MPPT" state pin
#define Phone 5 // "Phone Charging" state pin
#define Boost 2 // Pin controlling the NMOS of the boost converter

Adafruit_INA219 ina219; //used for measuring the solar panel output current
int PWM_FULL=1023;
int PWM_MAX=100;
enum charger_mode {no_bat,error,no_sun,Float,bulk} charger_state; //holds all the MPPT charger states
int i; //counter
int x; //used in the set_pwm() function
int delta=PWM_INC; //used in the Preturb and observe algorithm for tracking the MPPT point.
int percentage_charged=0; //holds the percentage charged of the battery
float solar_volt=0; //holds the solar panel voltage
float bat_volt=0; //hold the battery voltage
float solar_curr=0; //holds the current prduced by the solar panel
float solar_digital=0; //holds the digital value from which the solar voltage is calculated
float bat_digital=0; //holds the digital value from which the battery voltage is calculated
const int samples_num=150; //holds the number of the samples taken to calculate the battery and the solar panel voltages
const int R1=10000, R2=4700; //resistor used for measuring correctly the battery and the solar panel voltage
float cur_sol_watts=0; //current solar input watts
float old_sol_watts=0; //previous solar input watts
float old_sol_volts=0; //previous solar input volts
boolean load_status=0; //variable for writing on the LCD if the load is ON or OFF (note that the laod is initially OFF)
int RED=4, GREEN=3; //RED is ON when battery is dischargin (LOAD is ON) or it blinks where either in no_sun,error,or no_bat state
float PWM=30; //set the initial vlaue of the PWM cycle to be 30%
unsigned long currentMillis=0;
long previousMillis=0;

// We set the LCD address to 0x27 for a 20 chars,4 line display
//Set the pins on the I2C chip used for LCD connenctions:
                    //addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


//LCD ICONS
byte battery_icons[6][8]=
{{
  0b01110,
  0b11011,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
},
{
  0b01110,
  0b11011,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
},
{
  0b01110,
  0b11011,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
},
{
  0b01110,
  0b11011,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
},
{
  0b01110,
  0b11011,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
},
{
  0b01110,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
}};
#define PWM_ICON 7
byte _PWM_icon[8]=
{
  0b11101,
  0b10101,
  0b10101,
  0b10101,
  0b10101,
  0b10101,
  0b10101,
  0b10111,
};

#define SOLAR_ICON 6
byte _SOLAR_icon[8] = //icon for termometer
{
  0b11111,
  0b10101,
  0b11111,
  0b10101,
  0b11111,
  0b10101,
  0b11111,
  0b00000
};



void setup() {   
  Serial.begin(115200);
  Ethernet.begin(mac);
  ThingSpeak.begin(client);
  //Blynk.begin(auth); // For controlling the Arduino over the phone
  
  pinMode(Phone,INPUT); // "Phone Charging" state pin
  pinMode(MPPT,INPUT); // "MPPT" state pin
  pinMode(Boost,OUTPUT); //boost converter PMOS pin
  pinMode(PWM_PIN,OUTPUT);
  pinMode(load,OUTPUT);
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  Timer1.initialize(20);
  Timer1.pwm(PWM_PIN,0);
  lcd.begin(20,4); //initialize the lcd
  lcd.clear();
  //This is for indicating the battery FULLNESS (which has 6 stages (0-5))
  for (int batchar = 0; batchar <   6; ++batchar) {
    lcd.createChar(batchar, battery_icons[batchar]);
  }
  //Create the solar panel and the pwm lcd icons
  lcd.createChar(PWM_ICON,_PWM_icon);
  lcd.createChar(SOLAR_ICON,_SOLAR_icon);
  digitalWrite(RED,LOW);
  digitalWrite(GREEN,LOW);  
  digitalWrite(load,LOW); // make initially load to be OFF 
  ina219.begin(); // used for measuring the solar panel current
  ina219.setCalibration_16V_400mA(); //for 16V, 400mA range (high precision)
  digitalWrite(Phone,HIGH); // make charge "Phone Charging" state OFF
  digitalWrite(MPPT,LOW); // make "MPPT" state ON -> the default initial state is the "MPPT" state 
  digitalWrite(Boost,LOW); // turn the usb boost converter OFF 
  digitalWrite(7,HIGH);
}
//_________________________________________________________________________________

void loop() {
  //Blynk.run();
  read_data(); // Function that reads the battery and the solar panel voltages as well as the solar input current
  if ((digitalRead(MPPT)==LOW)){
    digitalWrite(Phone,HIGH); // make charge "Phone Charging" state OFF
    digitalWrite(MPPT,LOW); // make "MPPT" state ON
    digitalWrite(Boost,LOW); // turn the usb boost converter OFF
    mode_select(); // use that to decide on the charging substate
    set_charger();
    delay(3000);
    up_online();
    load_state();
    lcd_print();
    led_out();
    print_for_python();
  }
  if  ((digitalRead(Phone)==LOW)){
    digitalWrite(Phone,LOW); //make charge "Phone Charging" state ON
    digitalWrite(MPPT,HIGH); //make "MPPT" state OFF    
    if ( (bat_volt>=Vb_min) && (bat_volt<=Vb_max) ) digitalWrite(Boost,HIGH); //enable the boost converter in order to charge the phone as long as the correct battery is connected and there is sufficient battery capacity
    else digitalWrite(Boost,LOW); //Now we are in the "Phone charging" state but there is no sifficient battery capacity
    PWM=0; // We will also turn off the PMOS of the buck converter
    set_pwm;
    lcd_phone_charge();
    digitalWrite(GREEN,LOW);
    digitalWrite(RED,HIGH); // battery is discharging thus turn red led dishcarging indicator ON   
  } 
}
//_________________________________________________________________________________

//Function for setting the PWM of the PMOS of the buck converter
void set_pwm(void){
  if (PWM<PWM_MAX) {
    x=(int)( PWM_FULL *( (100-PWM) /100)); //because of the PMOS driver when the PWM signal is high the PMOS is OFF and VICE versa thus we write 100-PWM
    Timer1.pwm(PWM_PIN,x,20); //set frequency of PWM to be 50kHz
  }
  else if ( PWM >= PWM_MAX ) {
    PWM=PWM_MAX;
    Timer1.pwm(PWM_PIN,0,20); //set frequency of PWM to be 50kHz
  }
}
//_________________________________________________________________________________

//Measure Vbat, Vsolar, Solar current, Current Solar watts.
void read_data(void){
  
  //Measure input solar current using thr ACS712 current sensor
  //solar_curr_digital=analogRead(A3); //ACS712 sensor for measuring the solar panel current is connected to pin A3. 
  //solar_curr=(511-solar_curr_digital)*50/1023;

  solar_curr=ina219.getCurrent_mA();
  solar_curr=-solar_curr;


  
  //Measure solar and battery voltages
  for (i=0;i<samples_num;i++){
    solar_digital+=analogRead(A0); //A2 is the pin where the solar panel potential devider is connected to.
    bat_digital+=analogRead(A1); //A1 is the pin where the battery potential devider is connected to.
    delay(1); //take the samples with an interval of 32ms apart from each other (because 62 is equivalent to 1sec)
  }
  solar_digital=solar_digital/samples_num; 
  bat_digital=bat_digital/samples_num;  
 
  solar_volt=(4.835777*solar_digital*3.1276)/1000;
  bat_volt=(bat_digital*2)/409.6;
  bat_volt=bat_volt-0.1;
  //Now, we will calculate the percentage that the battery is charged to
  percentage_charged=map(bat_volt*100,160,225,0,100);  
  if (percentage_charged<0) percentage_charged=0;  

  
  //Current Solar watts
  cur_sol_watts=solar_volt*solar_curr;
  

}
//_________________________________________________________________________________

//Function for selecting the MPPT substate
void mode_select(void){


    if (bat_volt<Vb_min) charger_state=no_bat; //if measured battery voltage is less than the minimum battery voltage it means that no battery is connected to the circuit
    else if (bat_volt>Vb_max+0.25) charger_state=error; //if measured battery voltage is larger that the maximum battery voltage it means that we have connected a larger battery indicating an error
    
    else if ( (solar_volt<=bat_volt+0.8)) charger_state=no_sun; //no sufficient sunlight to charge the battery
 
    else if ( (bat_volt>=Vb_min) && (bat_volt<=Vb_max) && (solar_volt>bat_volt+0.8) ){ //now the measured battery voltage is in the expected range and also there is sufficient sun to charge the battery. Note that we account for the 0.8V voltage drop across the diode connecting the solar panel to the system. NOTE THAT THIS Vdrop might be 0V if we use back to back MOSFETS 

      if (bat_volt>=Vb_float) charger_state=Float;//enter to the float state
      else charger_state=bulk; //enter the bulk state
    
    }  
}
//_________________________________________________________________________________

//The following function is responsible for turning ON and OFF the load
//BATTERY FULLY CHARGED = 2.25V
//BATTERY FULLY DISCHARGED at <1.6V 
void load_state(void){
  if ((bat_volt>=Vb_min)&&(solar_volt<=bat_volt+0.8 && (bat_volt<=Vb_max))){
    digitalWrite(load,HIGH); //load is ON when battery is sufficiently charged and we don't have much sun
    load_status=1;
  }
  else if ((bat_volt<Vb_min)||(solar_volt>bat_volt+0.8) || (bat_volt>Vb_max)){
    digitalWrite(load,LOW); // load is OFF when battery is discharged by a lot OR if there is sufficient sun  
    load_status=0;
    
  }
}
//_________________________________________________________________________________

//Note that we have five states: no_battery, error(Vbat>Vmax),no_sun, bulk and float.
void set_charger(void){

  switch (charger_state){

    case no_bat:
      PWM=0; //turn OFF MOSFET Q1 (maybe the back to back mosfets as well?)
      set_pwm();
      break; 

    case error:
      PWM=0; //turn OFF MOSFET Q1 (maybe the back to back mosfets as well?)
      set_pwm();
      break;
  
    case no_sun:
      PWM=0; //turn OFF MOSFET Q1 (maybe the back to back mosfets as well?)
      set_pwm();
      break;

    case bulk: //Preturb and Observe Algorithm
      if (PWM==0) PWM=30; //if we come from PWM=0 state (no_sun,no_bat,error states) then we initialize PWM to 30;
      if (cur_sol_watts-old_sol_watts>0){
        if (solar_volt-old_sol_volts>0){
          PWM-=delta;
        }
        else if (solar_volt-old_sol_volts<0){
          PWM+=delta;
        }
      }
      else if (cur_sol_watts-old_sol_watts<0){
        if (solar_volt-old_sol_volts>0){
          PWM+=delta;
        }
        else if (solar_volt-old_sol_volts<0){
          PWM-=delta;
        }
      }     
      old_sol_watts=cur_sol_watts;
      old_sol_volts=solar_volt;
      set_pwm();
      break;
    
    case Float:
      /*currentMillis=millis();   
      if (currentMillis-previousMillis>1000){
        PWM+=1;
        previousMillis=currentMillis;
        if (PWM<=100) {
          Serial.print(solar_volt);
          Serial.print("         ");
          Serial.println(PWM);  
        }
      }*/
      PWM=30; //supply just a little current to the battery from the panel
      set_pwm();
      break;
    default:
      PWM=0;
      set_pwm();
      break;
  }
}
//_________________________________________________________________________________

//The following function is responsible for printing data on the LCD while in "Phone Charging" state
void lcd_phone_charge(void){
  lcd.clear();
  if ( (bat_volt>=Vb_min) && (bat_volt<=Vb_max) ){
    lcd.setCursor(3,1);
    lcd.print("Charging Phone");
  }
  else{
    lcd.setCursor(0,0);
    lcd.print("Not suf. bat cap");
    //lcd.setCursor(0,1);
    //lcd.print("Please recharge the battery");
  }
}
//_________________________________________________________________________________

//The following function is responsible for printing data on the LCD while in "MPPT" state
void lcd_print(void){
  // display the constant stuff on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SOL");
  lcd.setCursor(4, 0);
  lcd.write(SOLAR_ICON);
  lcd.setCursor(8, 0);
  lcd.print("BAT");
  lcd.setCursor(15,0);
  lcd.print("PWM");
  lcd.setCursor(15,2);
  lcd.print("Load");
  lcd.setCursor(19,0);
  lcd.write(PWM_ICON);

  lcd.setCursor(0,1);
  lcd.print(solar_volt);
  lcd.print("V");
  lcd.setCursor(0,2);
  lcd.print(solar_curr);
  if (solar_curr>=1000) lcd.print("A");
  else lcd.print("mA");
  lcd.setCursor(0,3);
  lcd.print(cur_sol_watts);
  if (cur_sol_watts>=1000) lcd.print("W");
  else lcd.print("mW");


  
  if (charger_state==no_bat){
    lcd.setCursor(8,2);
    lcd.print("N/A");
    lcd.setCursor(8,3);
    lcd.print(percentage_charged);
    lcd.print("%");
    lcd.setCursor(8,1);
    lcd.print("N/A");
  }
  else if (charger_state==error){
    lcd.setCursor(8,2);
    lcd.print("N/A");
    lcd.setCursor(8,3);
    //lcd.print(percentage_charged);    
    //lcd.print("%");
    lcd.print("N/A%");
    lcd.setCursor(8,1);
    lcd.print("N/A");
  }
  else if (charger_state==no_sun){
    lcd.setCursor(8,2);
    lcd.print("NoSun");
    lcd.setCursor(9,3);
    lcd.print(percentage_charged);
    lcd.print("%");   
    lcd.setCursor(8,1);
    lcd.print(bat_volt);
    lcd.print("V"); 
  }
  else if (charger_state==bulk){
    lcd.setCursor(8,2);
    lcd.print("Bulk");
    lcd.setCursor(9,3);
    lcd.print(percentage_charged);
    lcd.print("%");
    lcd.setCursor(8,1);
    lcd.print(bat_volt);
    lcd.print("V");    
  }
  else if (charger_state==Float){
    lcd.setCursor(8,2);
    lcd.print("Float");
    lcd.setCursor(9,3);
    lcd.print(percentage_charged);
    lcd.print("%");
    lcd.setCursor(8,1);
    lcd.print(bat_volt);
    lcd.print("V");
  }
  lcd.setCursor(12,0);
  lcd.print((char)(percentage_charged*5/100)); //this is for printing the battery icon with lines idicating the %charged
  lcd.setCursor(15,1);
  lcd.print((int)PWM);
  lcd.print("%");
  lcd.setCursor(15,3);
  if (load_status==0) lcd.print("OFF");
  else lcd.print("ON");  
}
//_________________________________________________________________________________

//Function which controls the led charging and discharging indicators
void led_out(void){
  if (load_status==1) { //LOAD is ON thus the battery is getting discharged
    digitalWrite(GREEN,LOW);
    digitalWrite(RED,HIGH);  
  }
  else{ //LOAD is OFF thus the battery is getting charged
    if (charger_state==bulk || charger_state==Float){ //then the battery is charging
      digitalWrite(GREEN,HIGH);
      digitalWrite(RED,LOW);     
    }
    else{ //load off but there is no sufficient sunlight
      digitalWrite(GREEN,LOW);
      digitalWrite(RED,LOW);
    }
  }
}
//_________________________________________________________________________________

//Uploading the solar panel output wattage every 20s (as the command takes 20s to be executed)
void up_online(void){
  ThingSpeak.writeField(myChannelid, 1, (int) cur_sol_watts, myWriteAPIKey);
}
//_________________________________________________________________________________

//plotting the solar panel and battery voltages using python
void print_for_python(void){
  Serial.print(bat_volt,4);
  Serial.print(",");
  Serial.println(solar_volt,4);
}
