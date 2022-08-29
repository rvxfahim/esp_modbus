/*
  Modbus-Arduino Example - Test Analog Input (Modbus IP ESP8266)
  Read Analog sensor on Pin ADC (ADC input between 0 ... 1V)
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

//ModbusIP object
ModbusIP mb;
String instruc="";
long ts;
int tankPerc=0;
bool forward=true;
uint16_t cbLed(TRegister* reg, uint16_t val) {
  //Attach ledPin to LED_COIL register
  ledcWrite(0, val);
  Serial.println(val);
  return val;
}
uint16_t cbString(TRegister* reg, uint16_t val){
    // Serial.println((char)mb.Hreg(2));
    instruc ="";
    for(int z=2; z<9;z++)
    {   
        if((mb.Hreg(z)>=65 && mb.Hreg(z)<=89) ||(mb.Hreg(z)>=97 && mb.Hreg(z)<=122))
        instruc.concat((char)mb.Hreg(z));
        else
        instruc.concat(" ");
        
    }
    instruc.concat((char)val);
    instruc.replace(" ","");
    Serial.println(instruc);
    Serial.println(instruc.length());
    return val;
}

void setup() {
    Serial.begin(115200);
 
    WiFi.begin("SSID", "Password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    ts = millis();
    
    //pinMode(2,OUTPUT);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(2, 0);
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    mb.server();		//Start Modbus IP
    mb.addIreg(1);      // 16 bit input register Tank information 3x300002
    mb.addHreg(1);      // 16 bit Holding register for LED Brightness value (writeable by HMI) 4x400002 
    mb.onSetHreg(1, cbLed);
    for (size_t i = 2; i < 10; i++)     //4x400003-4x400010
    {
       mb.addHreg(i);
    }   
   
   mb.onSetHreg(9,cbString);
   
}

void loop() {
   //Call once inside loop() - all magic here
   mb.task();

   //Read each 20 milliseconds
   if (millis() > ts + 20) {
       ts = millis();
       //Setting raw value (0-1024)
       if (forward)
       {
       mb.Ireg(1, tankPerc++); //store in input register 3x300002
        if(tankPerc==1024)
        forward =false;
       }
       else
        {  
            mb.Ireg(1, tankPerc--);

            if (tankPerc==0)
            {
                forward=true;
            } 
        }
   }

   delay(10);
}
