//(c)2014 Andreas Seiderer
//using DHT sample by ladyada, public domain
//using HMC588L Demo by dipmicro electronics, 2014 ( http://www.dipmicro.com/store/GY271-MOD )
/*
 * GY271 module (dipmicro part DE4196
 *  Arduino GND -> GY271/HMC5883L GND
 *  Arduino 3.3V -> GY271/HMC5883L VCC
 *  Arduino A4 (SDA) -> GY271/HMC5883L SDA
 *  Arduino A5 (SCL) -> GY271/HMC5883L SCL 
*/

#include <Wire.h> //I2C Arduino Library

#define HMC5883L_ADDR 0x1E //0011110b, I2C 7bit address of HMC5883

bool haveHMC5883L = false;

bool detectHMC5883L ()
{
  // read identification registers
  Wire.beginTransmission(HMC5883L_ADDR); //open communication with HMC5883
  Wire.write(10); //select Identification register A
  Wire.endTransmission();
  Wire.requestFrom(HMC5883L_ADDR, 3);
  if(3 == Wire.available()) {
    char a = Wire.read();
    char b = Wire.read();
    char c = Wire.read();
    if(a == 'H' && b == '4' && c == '3')
      return true;
  }

  return false;
}


#include "DHT.h"

#define DHTPIN 2
#define IRPIN A0
#define MAGPIN A1

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); 
  //Serial.println("DHTxx test!");
  
  //Initialize I2C communications
  Wire.begin();
  // lower I2C clock http://www.gammon.com.au/forum/?id=10896
  TWBR = 78;  // 25 kHz 
  TWSR |= _BV (TWPS0);  // change prescaler  
 
  //dht 22
  dht.begin();
}

void loop() {
  
  //magnetometer
  bool detect = detectHMC5883L();
  if(!haveHMC5883L) 
  {
    if(detect) 
    {
      haveHMC5883L = true;
      //Serial.println("We have HMC5883L, moving on");
      // Put the HMC5883 IC into the correct operating mode
      Wire.beginTransmission(HMC5883L_ADDR); //open communication with HMC5883
      Wire.write(0x02); //select mode register
      Wire.write(0x00); //continuous measurement mode
      Wire.endTransmission();
    }
    else
    {  
      //Serial.println("No HMC5883L detected!");
      //delay(2000);
      return;
    }
  }
  else
  {
    if(!detect) {
      haveHMC5883L = false;
      //Serial.println("Lost connection to HMC5883L!");
      //delay(2000);
      return;
    }
  }
  
  int x,y,z; //triple axis data

  //Tell the HMC5883 where to begin reading data
  Wire.beginTransmission(HMC5883L_ADDR);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();

 //Read data from each axis, 2 registers per axis
  Wire.requestFrom(HMC5883L_ADDR, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
    z = Wire.read()<<8; //Z msb
    z |= Wire.read(); //Z lsb
    y = Wire.read()<<8; //Y msb
    y |= Wire.read(); //Y lsb
  }
  
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  float ir = analogRead(IRPIN);

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    Serial.print(ir);
    Serial.print(";");
    Serial.print(t);
    Serial.print(";");
    Serial.print(h);
    Serial.print(";");
      
    //Magnetometer: Print out values of each axis
    Serial.print(x);
    Serial.print(";");
    Serial.print(y);
    Serial.print(";");
    Serial.println(z);
    
    delay(500);
  }
}
