// ssihealth
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/05/16
// Copyright (C) 2007-13 University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include <PinChangeInt.h>
#include <eHealth.h>

#define CMD_START "start"
#define CMD_STOP "stop"
#define CMD_ECG "ecg"
#define CMD_GSR "gsr"
#define CMD_AIR "air"
#define CMD_TMP "tmp"
#define CMD_BPM "bpm"
#define CMD_OXY "oxy"
#define CMD_POS "pos"

int cont = 0;

boolean isConnected = 0;
boolean readCommand = false;
boolean hasCommand = false;

int nCmdBuffer = 20;
int nCmdBufferUsed = 0;
char cmdBuffer[20];

boolean useECG = false;
boolean useGSR = false;
boolean useAIR = false;
boolean useTMP = false;
boolean useBPM = false;
boolean useOXY = false;
boolean usePOS = false;

float ECG = 0;
float GSR = 0;
float AIR = 0;
float TMP = 0;
float BPM = 0;
float OXY = 0;
uint8_t POS = 0;

int HIGHfreq = 100;
int LOWfreq = 10;
unsigned long lastHIGH = 0;
unsigned long lastLOW = 0;

// The setup routine runs once when you press reset:
void setup () {
  
  Serial.begin(115200); 
  eHealth.initPositionSensor();
  eHealth.initPulsioximeter();  
  PCintPort::attachInterrupt(6, readPulsioximeter, RISING);
  
  nCmdBufferUsed = 0;
  readCommand = false;
  hasCommand = false;
  isConnected = false;
}

// Include always this code when using the pulsioximeter sensor
void readPulsioximeter() {  
  cont ++;
  if (cont == 50) { // Get only of one 50 measures to reduce the latency
    eHealth.readPulsioximeter();  
    cont = 0;
  }
}

// SerialEvent occurs whenever a new data comes in:
void serialEvent () {
       
  while (Serial.available()) {

    if (!hasCommand) {
      
      char c = Serial.read();
    
      if (c == '<') {        
        nCmdBufferUsed = 0;
        readCommand = true;
        hasCommand = false;
      
      } else if (readCommand) {      
      
        if (c == '>') {            
          hasCommand = true;
          readCommand = false;            
        } else if (nCmdBufferUsed < nCmdBuffer-1) {                        
          cmdBuffer[nCmdBufferUsed++] = c;            
        }            
        
        cmdBuffer[nCmdBufferUsed] = '\0';          
      }
    }
  }    
}  

// parse command string:
void parseCommand () {
  
  if (strcmp (cmdBuffer, CMD_START) == 0) {     
    ECG = 0;
    GSR = 0;
    AIR = 0;
    TMP = 0;
    BPM = 0;
    OXY = 0;
    POS = 0;
    isConnected = true;
  } else if (strcmp (cmdBuffer, CMD_STOP) == 0) {     
    isConnected = false;    
    useECG = false;
    useGSR = false;
    useAIR = false;
    useTMP = false;
    useBPM = false;
    useOXY = false;
    usePOS = false;
  } else if (strcmp (cmdBuffer, CMD_ECG) == 0) {
    useECG = true;  
  } else if (strcmp (cmdBuffer, CMD_GSR) == 0) {
    useGSR = true;  
  } else if (strcmp (cmdBuffer, CMD_AIR) == 0) {
    useAIR = true;  
  } else if (strcmp (cmdBuffer, CMD_TMP) == 0) {
    useTMP = true;  
  } else if (strcmp (cmdBuffer, CMD_BPM) == 0) {
    useBPM = true;  
  } else if (strcmp (cmdBuffer, CMD_OXY) == 0) {
    useOXY = true;  
  } else if (strcmp (cmdBuffer, CMD_POS) == 0) {
    usePOS = true;  
  } 
  
  hasCommand = false;
}

// The loop routine runs over and over again forever:
void loop () {
  
  // wait for connection:      
  if (hasCommand) {      
    parseCommand ();
  }
 
 if (isConnected) {

    // read sensor values:
    
    unsigned long time = millis();
    
    if (time >= (lastHIGH + (1000/HIGHfreq))){
      
      lastHIGH = time;
      
      if (useECG) {
        ECG = eHealth.getECG();      
      }
      
      if (time >= (lastLOW + (1000/LOWfreq))){
        
        lastLOW = time;                
       
        if (useGSR) {        
          GSR = eHealth.getSkinConductanceVoltage ();
        }
        if (useAIR) {
          AIR = eHealth.getAirFlow ();
        }
        if (useTMP) {
          TMP = eHealth.getTemperature ();
        }
        if (useBPM) {
          BPM = eHealth.getBPM ();
        }
        if (useOXY) {
          OXY = eHealth.getOxygenSaturation ();
        }
        //if (usePOS) {
        //  POS = eHealth.getOxygenSaturation ();
        //}

      }
       
      // send sensor values:       
       
      Serial.print (ECG);
      Serial.print (";");
      Serial.print (GSR); 
      Serial.print (";");
      Serial.print (AIR);
      Serial.print (";");
      Serial.print (TMP);
      Serial.print (";");
      Serial.print (BPM);
      Serial.print (";");
      Serial.print (OXY);      
      //Serial.print (";");
      //Serial.print (POS);            
      Serial.println ();
      
    } 
  } 
}

