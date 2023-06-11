#include <Wire.h>

//Settings
const int CODE_VERSION = 2;
const char CODE_VERSION_DATE[] = "2023-04-10";

//Resistance Values
const int RESISTANCES[16] = {
  1024, 938, 868, 806, 752, 706, 665, 628, 595, 565, 535, 514, 490, 470, 452, 435
};

//Shift Register Positions
//Some are revesed to fix a hardware bug on the circuit board
const int SHIFT_POSITIONS[16] = {
  31, 29, 25, 27, 23, 21, 17, 19, 15, 13, 9, 11, 7, 5, 1, 3
};

//Constants
const int DATA_NODE = B0110000;
const int DATA_COMMAND = B1000000;

//Arduino Pins
const int PIN_ANALOG_LISTEN = A0;
const int PIN_PWM_LISTEN = A1;
const int PIN_SER_IN = 8;
const int PIN_SRCK = 9;
const int PIN_RCK = 10;
const int PIN_G = 11;

//Point States
const int STATE_STRAIGHT = -1;
const int STATE_UNKNOWN = 0;
const int STATE_TURNOUT = 1;

//Point Motor References - Node 00
const int POINT_YARD_INNER_LEFT_1 = B000010;
const int POINT_YARD_INNER_LEFT_2 = B000011;
const int POINT_YARD_OUTER_LEFT_1 = B000100;
const int POINT_YARD_OUTER_LEFT_2 = B000101;

//Point Motor References - Node 01
const int POINT_YARD_CENTER_OUT_CROSSOVER_1 = B010010;
const int POINT_YARD_CENTER_OUT_CROSSOVER_2 = B010011;
const int POINT_YARD_CENTER_IN_CROSSOVER_1 = B010100;
const int POINT_YARD_CENTER_IN_CROSSOVER_2 = B010101;
const int POINT_YARD_CENTER_IN = B010110;
const int POINT_YARD_CENTER_OUT = B010111;

//Point Motor References - Node 10
const int POINT_YARD_INNER_RIGHT_1 = B100010;
const int POINT_YARD_INNER_RIGHT_2 = B100011;
const int POINT_YARD_OUTER_RIGHT_1 = B100100;
const int POINT_YARD_OUTER_RIGHT_2 = B100101;

//Point Motor References - Node 11
const int POINT_FRONT_INNER_MAIN_CROSSOVER = B110010;
const int POINT_FRONT_OUTER_MAIN_CROSSOVER = B110011;
const int POINT_FRONT_INNER_TURNOUT = B110100;
const int POINT_FRONT_INNER_TRAILING_CROSSOVER = B110101;
const int POINT_FRONT_OUTER_TRAILING_CROSSOVER = B110110;
const int POINT_FRONT_BAY_PLATFORM = B110111;

//----------------------------------------------------------------------------------------
//General Methods

void i2cSend(int toID, int command) {
  Wire.beginTransmission(toID);
  Wire.write(command);
  Wire.endTransmission();

  Serial.println("[i2cSend] I2C command " + String(command, BIN) + " sent to I2C ID " + String(toID, BIN));

}

//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
//Class Point

class Point {
  public:
    Point();
    Point(int pointNumber, int id, int defaultState);
    void setDefault();
    void toggleState();
    void setState(int state);
    int checkTrigger(int analogValue);
    int getI2CID();
    int getI2CCommand();
    int getShiftPositon();
    int getState();
    int getShiftPosition();
    
  private:
    int _id;
    int _triggerAnalog;
    int _defaultState; 
    int _currentState; 
    int _shiftPosition;
    int _pointNumber;
};

Point::Point() {
  //Nothing
}

Point::Point(int pointNumber, int id, int defaultState) {
  _pointNumber = pointNumber;
  _id = id;
  _defaultState = defaultState;
  
  _shiftPosition = SHIFT_POSITIONS[_pointNumber];
  _triggerAnalog = RESISTANCES[_pointNumber];
}

void Point::setDefault() {
  setState(_defaultState);
}

void Point::toggleState() {
  setState(_currentState * -1);
}

void Point::setState(int state) {
  _currentState = state;
  Serial.println("[Point->setState] Setting Point ID " + String(_id, BIN) + " to state " + String(_currentState));

  //Send I2C Command
  i2cSend(getI2CID(), getI2CCommand());  
}

int Point::checkTrigger(int analogValue) {

  if (abs(_triggerAnalog - analogValue) < 10) {
    Serial.println("[Point->checkTrigger] Trigger detected on for Point ID " + String(_id, BIN));
    toggleState();    
    return 1;
  }

  return 0;  
}

int Point::getI2CID() {
  return DATA_NODE & _id;
}

int Point::getI2CCommand() {
  int command = 0;

  if (_currentState == STATE_STRAIGHT) {
    command = B1000000;    
  }

  return command + _id;
}

int Point::getShiftPosition() {
  return _shiftPosition;
}

int Point::getState() {
  return _currentState;
}

//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
//Class Shift Register
class ShiftRegister {
  public:
    ShiftRegister(int SER_IN, int SRCK, int RCK, int G);
    void transmit();
    void setBit(long pos, long value);
    void init();
    void setPWM();
    
  private:
    int _SER_IN;
    int _SRCK;
    int _RCK; 
    int _G;
    long _data;
};

ShiftRegister::ShiftRegister(int SER_IN, int SRCK, int RCK, int G) {
  _SER_IN = SER_IN;
  _SRCK = SRCK;
  _RCK = RCK;
  _G = G;
}

void ShiftRegister::transmit() {

  long dataBuffer = _data;
  int state = 0;

  Serial.println("[ShiftRegister->transmit] Outputting to Shift Registers " + String(_data, BIN));

  //Send bits to Shift Register
  for (int i = 0; i < 32; i ++) {
    state = dataBuffer & 1;

    if (state == 1) {
      digitalWrite(_SER_IN, HIGH);
    } else {
      digitalWrite(_SER_IN, LOW);
    }
    delay(5);

    //Pulse clock
    digitalWrite(_SRCK, HIGH);
    delay(5);
    digitalWrite(_SRCK, LOW);  

    dataBuffer = dataBuffer >> 1;
  }  

  //Push to output
  delay(5);
  digitalWrite(_RCK, HIGH);
  delay(5);
  digitalWrite(_RCK, LOW);     
}

void ShiftRegister::setBit(long pos, long value) {
  long mask;

  long bitpos;

  bitpos = pos - 1;
  
  if (value == 0) {
    mask = ~(1L << bitpos);
    _data &= mask;
  } else {
    mask = 1L << bitpos;
    _data |= mask;
  }

  Serial.println("[ShiftRegister->setBit] Shift Register setting bit " + String(pos) + " to " + String(value) + " result " + String(_data, BIN) + " mask: " + String(mask, BIN));
}

void ShiftRegister::init() {
  pinMode(_SER_IN, OUTPUT);
  pinMode(_SRCK, OUTPUT);
  pinMode(_RCK, OUTPUT);
  //pinMode(_G, OUTPUT);

  digitalWrite(_SER_IN, LOW);
  digitalWrite(_SRCK, LOW);
  digitalWrite(_RCK, LOW);
  //digitalWrite(_G, LOW);    
}

void ShiftRegister::setPWM() {
  int value = analogRead(PIN_PWM_LISTEN);
  analogWrite(_G, value / 4);
  Serial.println("[ShiftRegister->setBit] LED PWM Output: " + String(value / 4) + "/255");
}


//----------------------------------------------------------------------------------------

int inputProcessed = 0;
int updateOutput = 0;
ShiftRegister shiftRegister(PIN_SER_IN, PIN_SRCK, PIN_RCK, PIN_G);

Point points[20] = {
  //Node 1 Points
  Point(0, POINT_FRONT_INNER_MAIN_CROSSOVER, STATE_STRAIGHT),
  Point(0, POINT_FRONT_OUTER_MAIN_CROSSOVER, STATE_STRAIGHT),
  Point(1, POINT_FRONT_INNER_TURNOUT, STATE_STRAIGHT),
  Point(2, POINT_FRONT_BAY_PLATFORM, STATE_TURNOUT),
  Point(3, POINT_FRONT_OUTER_TRAILING_CROSSOVER, STATE_STRAIGHT),
  Point(3, POINT_FRONT_INNER_TRAILING_CROSSOVER, STATE_STRAIGHT),
  
  //Node 2 Points
  Point(4, POINT_YARD_INNER_RIGHT_1, STATE_STRAIGHT),
  Point(5, POINT_YARD_OUTER_RIGHT_1, STATE_STRAIGHT),
  Point(6, POINT_YARD_INNER_RIGHT_2, STATE_STRAIGHT),
  Point(7, POINT_YARD_OUTER_RIGHT_2, STATE_STRAIGHT),
      
  //Node 3 Points
  Point(8, POINT_YARD_CENTER_IN, STATE_STRAIGHT),
  Point(9, POINT_YARD_CENTER_OUT, STATE_STRAIGHT),
  Point(10, POINT_YARD_CENTER_IN_CROSSOVER_1, STATE_STRAIGHT),
  Point(10, POINT_YARD_CENTER_IN_CROSSOVER_2, STATE_TURNOUT),
  Point(11, POINT_YARD_CENTER_OUT_CROSSOVER_1, STATE_STRAIGHT),
  Point(11, POINT_YARD_CENTER_OUT_CROSSOVER_2, STATE_TURNOUT),

  //Node 4 Points
  Point(12, POINT_YARD_INNER_LEFT_1, STATE_STRAIGHT),
  Point(13, POINT_YARD_OUTER_LEFT_1, STATE_STRAIGHT),
  Point(14, POINT_YARD_INNER_LEFT_2, STATE_STRAIGHT),
  Point(15, POINT_YARD_OUTER_LEFT_2, STATE_TURNOUT)    
};

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("[setup] Point Controler, Code Version: " + String(CODE_VERSION));
  Serial.println("[setup] Point Controler, Code Date: " + String(CODE_VERSION_DATE));

  //Initiate I2C
  Wire.begin(1);

  //Initiate Shift Register Control
  shiftRegister.init();
  shiftRegister.transmit();

  delay(500);

  //Detect LED PWM
  shiftRegister.setPWM();
  
  //Setup Points
  for (int i = 0; i < 20; i++) {
    points[i].setDefault();
    updateShift(points[i].getShiftPosition(), points[i].getState());
    delay(100);
  } 
  
  updateOutput = 1;

  Serial.println("[setup] Complete");
}

void loop() {
  // put your main code here, to run repeatedly:
  analogListen();

  if (updateOutput == 1) {
    shiftRegister.transmit();
    updateOutput = 0;
  }

  delay(50);
  //shiftRegister.setPWM();
}

void analogListen() {
  int value = analogRead(PIN_ANALOG_LISTEN);

  if (value < 5) {
    inputProcessed = 0;
    return;
  }

  if (inputProcessed == 1) {
    return;
  }

  for (int i = 0; i < 20; i++) {
    if (points[i].checkTrigger(value) == 1) {
      updateShift(points[i].getShiftPosition(), points[i].getState());
      inputProcessed = 1;
      updateOutput = 1;     
      return;
    }
  }
  
  Serial.println("[analogListen] Unknown Voltage Divider Output: " + String(value));
    
}

void updateShift(int shiftPosition, int state) {
  if (state == STATE_STRAIGHT) {
    shiftRegister.setBit(shiftPosition, 1);
    shiftRegister.setBit(shiftPosition + 1, 0);
  } else {
    shiftRegister.setBit(shiftPosition, 0);
    shiftRegister.setBit(shiftPosition + 1, 1);
  } 
}
