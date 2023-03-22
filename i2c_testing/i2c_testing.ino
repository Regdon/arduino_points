#include <Wire.h>

//----------------------------------------------------------------------------------------
//General Methods

void i2cSend(int toID, int command) {
  Wire.beginTransmission(toID);
  Wire.write(command);
  Wire.endTransmission();

  Serial.println("[i2cSend] I2C command " + String(command) + " sent to I2C ID " + String(toID));

}

//----------------------------------------------------------------------------------------


void setup() {

  //Initiate I2C
  Wire.begin();
  delay(500);

}

void loop() {

  //Setting pin2 on each node to turnout
  i2cSend(0, B1000010);
  delay(200);
  i2cSend(0, B1010010);
  delay(200);  
  i2cSend(0, B1100010);
  delay(200);  
  i2cSend(0, B1110010);
  delay(1000);

  //Setting pin2 on each node to straight
  i2cSend(0, B0000010);
  delay(200);  
  i2cSend(0, B0010010);
  delay(200);  
  i2cSend(0, B0100010);
  delay(200);  
  i2cSend(0, B0110010);
  delay(1000);

}
