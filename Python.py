import serial  # import Serial Library
import numpy # needed fro array math
import matplotlib.pyplot as plt  #import matplotlib library
from drawnow import * # " * " means import everything

bat_v=[]
solar_v=[]
cnt=0 #counter which will count up to 40 in order to make sure that we constantly plot the last (newest) 40 voltage points
wait=True


arduinoData=serial.Serial ('com4',115200)

plt.ion() #Tell Python (actually matplotlib) that we are going to be plotting live data

def makeFig(): #function that plots the battery and the solar panel voltages
    plt.ylim(1.5,2.26) #set the y limit to be from 1.5 to 2.26V (as the maximum battery voltage is 2.25V and minimum is approximately 1.6V
    plt.plot(bat_v, "bo-",label="Bat_V")
    plt.legend(loc="upper right")
    plt.grid(True)
    plt.title("Battery and Solar Panel Voltages")
    plt.xlabel("Points in time")
    plt.ylabel("Battery Voltage (V)")
    plt2=plt.twinx()
    plt2.plot(solar_v,"o-",color="orange",label="Solar_V")
    plt2.legend(loc="upper left")
    plt.ylim(0,12) #set the y limit of the solar panel to be 12V as the maximum voltage we can get from the solar panel is the open-circuit voltage which is equal to 10.65V
    plt2.set_ylabel("Solar Panel Voltage (V)")
    

while (wait):
    while (arduinoData.in_waiting==0):
        pass #if there is no data then WE DO NOTHING until data comes
    arduinostring_temp=arduinoData.readline() #we read data from the serial port and put them in arduinostring
    arduinostring=arduinostring_temp.decode("utf-8")
    #print (arduinostring)    
    arduino_array=arduinostring.split(",")
    tempbat_v=float(arduino_array[0])
    tempsolar_v=float(arduino_array[1])
    print(tempbat_v,",",tempsolar_v)  
    bat_v.append(tempbat_v) #an array which basically increases with time
    solar_v.append(tempsolar_v) #an array which basically increases with time
    drawnow(makeFig) #for plotting live data

    cnt+=1
    if (cnt>=40): #this way we make sure that we are constatly plotting the newest 40 voltages 
        bat_v.pop(0)
        solar_v.pop(0)
     


    
        
