#include <OneWire.h>
#include <SoftwareSerial.h>
 


// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library


////


OneWire  ds(2);  // on pin 10 (a 4.7K resistor is necessary)
SoftwareSerial esp8266(10,11);
float celsius, fahrenheit,celsius_web;
int n=0;

#define DEBUG true



void setup(void) {
  Serial.begin(9600); 
  esp8266.begin(9600); // your esp's baud rate might be different
  
 
  
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
  
}


void loop(void) {
  
  
 if(esp8266.available()) // check if the esp is sending a message 
  {


    byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
  //  Serial.println("No more addresses.");
   /// Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  

  for( i = 0; i < 8; i++) {  
    addr[i];

  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
//  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:

      type_s = 1;
      break;
    case 0x28:

      type_s = 0;
      break;
    case 0x22:
    //  Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
    //  Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);      
  
  delay(1000);    

//  delay(1000);    
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         

  for ( i = 0; i < 9; i++) {       
    data[i] = ds.read();
  }
  OneWire::crc8(data, 8); 
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; 
    if (data[7] == 0x10) {      
      raw = (raw & 0xFFF0) + 12 - data[6];    }
  } else {
    byte cfg = (data[4] & 0x60);

    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    
  }
  celsius = (float)raw / 16.0;
  celsius_web = celsius;
  
  ////////////////////////////////////////////////
  
  
  

////// peticion del navegador (broswer)  web ie  chrome  firefox 
    if(esp8266.find("+IPD"))
    {   
               

     delay(2000);
     
    //Serial.print(celsius);
     
     Serial.print("temperatura actual =");  
     Serial.println(celsius_web);  
     


    //counter
     n++; 
     //int to string
     String dato (n, DEC);  
     
     
     ///CONVERT  FLOAT TO CHAR  + CHAR TO STRING 
     
       char outstr[15];
      dtostrf(celsius_web,7, 3, outstr);   //float to char
      String valor = outstr;   // char to string
     
     
    String webpage;
     
     //////encabezado
     
      webpage = "HTTP/1.1 200 OK\r\n";
      webpage += "Content-Type: text/html\r\n";
      webpage += "Connection: close\r\n";
      //webpage += "Refresh: 8\r\n";
      webpage += "\r\n";
      webpage += "<!DOCTYPE HTML>\r\n";
      
      
      ///html
      
      webpage += "<html>\r\n";
      
      webpage +="<TITLE>SERVER WEB Arduino + ESP8266 </TITLE>";
      
      
     webpage += "<H1>\r\n";
     webpage += "SERVER WEB Arduino Nano + ESP8266  + DS18B20  PDA_Control "  ;
     webpage += "</H1>\r\n";
     
      
     webpage += "<H2>\r\n";
     webpage += "Refresh: "+ dato  ;
     webpage += "</H2>\r\n";
     
     webpage += "<H2>\r\n";
     webpage += "Temp Actual =  "+  valor ;
     webpage += "</H2>\r\n";
      
      webpage += "<H2>\r\n";
      webpage += "Current Temperature =  "+  valor ;
      webpage += "</H2>\r\n";
      
      
    
           
      ///fin html  
      webpage += "</html>\r\n";
     
     String cipsend = "AT+CIPSEND=0,";
     cipsend+=webpage.length();
     cipsend+="\r\n";
     
     sendData(cipsend,500,DEBUG);
     sendData(webpage,500,DEBUG);
     
     sendData("AT+CIPCLOSE=0\r\n",1500,DEBUG);
     
     
    }

  }
  
}


String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
      
   }
    
    return response;
  
}
