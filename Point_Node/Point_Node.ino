#include <Wire.h>
#include <Servo.h>

//Settings
const int CODE_VERSION = 1;
const char CODE_VERSION_DATE[] = "2022-11-06";
int node_id = 0;

//Constants
const int DATA_PIN = B0001111;
const int DATA_NODE = B0110000;
const int DATA_COMMAND = B1000000;
const int DATA_ID = B0111111;

const int STATE_STRAIGHT = -1;
const int STATE_UNKNOWN = 0;
const int STATE_TURNOUT = 1;

//Switch Pins
const int PIN_SWITCH_1 = 10;
const int PIN_SWITCH_2 = 11;

//Point Motor References - Node 00
const int POINT_UP_MAIN_CROSSOVER = B000010;
const int POINT_DOWN_MAIN_CROSSOVER = B000011;
const int POINT_DOWN_BAY_PLATFORM = B000100;
const int POINT_BAY_PLATFORM = B000101;
const int POINT_DOWN_TRAILING_CROSSOVER = B000110;
const int POINT_UP_TRAILING_CROSSOVER = B000111;

//Point Motor References - Node 01
const int POINT_YARD_INNER_RIGHT_1 = B010010;
const int POINT_YARD_OUTER_RIGHT_1 = B010011;
const int POINT_YARD_INNER_RIGHT_2 = B010100;
const int POINT_YARD_OUTER_RIGHT_2 = B010101;

//Point Motor References - Node 10
const int POINT_YARD_CENTER_IN = B100010;
const int POINT_YARD_CENTER_OUT = B100011;
const int POINT_YARD_CENTER_IN_CROSSOVER_1 = B100100;
const int POINT_YARD_CENTER_IN_CROSSOVER_2 = B100101;
const int POINT_YARD_CENTER_OUT_CROSSOVER_1 = B100110;
const int POINT_YARD_CENTER_OUT_CROSSOVER_2 = B100111;

//Point Motor References - Node 11
const int POINT_YARD_INNER_LEFT_1 = B110010;
const int POINT_YARD_OUTER_LEFT_1 = B110011;
const int POINT_YARD_INNER_LEFT_2 = B110100;
const int POINT_YARD_OUTER_LEFT_2 = B110101;

//---------------------------------------------------------------------------------------------------
//Class PointMotor

class PointMotor {
  public:
    PointMotor();
    PointMotor(int id, int angleCentre, int angleThrow);
    int point_exists();
    void setStraight();
    void setTurnout();
    void attach();
    int getState();
    int getID();
  private:    
    int _id;
    int _pin; 
    int _node;    
    int _angleCentre;
    int _angleThrow;
    int _state; 
    int _exists; 
    Servo _servo;

    void setServoAngle();
};

PointMotor::PointMotor() {
  _exists = 0;
}

PointMotor::PointMotor(int id, int angleCentre = 24, int angleThrow = 12) {
  _id = id;
  _pin = _id & DATA_PIN;
  _node = _id & DATA_NODE;
  _angleCentre = angleCentre;
  _angleThrow = angleThrow;
  _exists = 1;
}

int PointMotor::point_exists() {
  return _exists;
}

void PointMotor::attach() {
  _servo.attach(_pin);
  setStraight();
  delay(100);
}

void PointMotor::setStraight() {
  _state = STATE_STRAIGHT;
  setServoAngle();
}

void PointMotor::setTurnout() {
  _state = STATE_TURNOUT;
  setServoAngle();
}

int PointMotor::getState() {
  return _state;
}

int PointMotor::getID() {
  return _id;
}

void PointMotor::setServoAngle() {
  _servo.attach(_pin);
  _servo.write(_state * _angleThrow + _angleCentre);
  Serial.println("[SET_SERVO_ANGLE] Pin " + String(_pin) + " set to " + String(_state * _angleThrow + _angleCentre));
  delay(200);
  _servo.detach();
}

//---------------------------------------------------------------------------------------------------

PointMotor points[8] = {PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor()};

volatile int i2cCommand;
volatile int i2cCommandWaiting;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("[SETUP] Point Node, Code Version: " + String(CODE_VERSION));
  Serial.println("[SETUP] Point Node, Code Date: " + String(CODE_VERSION_DATE));

  //Read Switch pins 1 and 2 to get node_id
  pinMode(PIN_SWITCH_1, INPUT);
  pinMode(PIN_SWITCH_2, INPUT);
  if (digitalRead(PIN_SWITCH_1) == HIGH) {
    node_id = node_id + B010000;  
  }
  if (digitalRead(PIN_SWITCH_2) == HIGH) {
    node_id = node_id + B100000;
  }
  Serial.println("[SETUP] Node ID set to " + String(node_id) + " (Binary " + String(node_id, BIN) + ")");

  switch (node_id) {
    case B000000:
      points[0] = PointMotor(POINT_UP_MAIN_CROSSOVER, 24, 12);
      points[1] = PointMotor(POINT_DOWN_MAIN_CROSSOVER, 24, 12);
      points[2] = PointMotor(POINT_DOWN_BAY_PLATFORM, 24, 12);
      points[3] = PointMotor(POINT_BAY_PLATFORM, 24, 12);
      points[4] = PointMotor(POINT_DOWN_TRAILING_CROSSOVER, 24, 12);
      points[5] = PointMotor(POINT_UP_TRAILING_CROSSOVER, 24, 12);
      break;
      
    case B010000:
      points[0] = PointMotor(POINT_YARD_INNER_RIGHT_1, 24, 12);
      points[1] = PointMotor(POINT_YARD_OUTER_RIGHT_1, 24, 12);
      points[2] = PointMotor(POINT_YARD_INNER_RIGHT_2, 24, 12);
      points[3] = PointMotor(POINT_YARD_OUTER_RIGHT_2, 24, 12);
      break;
      
    case B100000:
      points[0] = PointMotor(POINT_YARD_CENTER_IN, 24, 12);
      points[1] = PointMotor(POINT_YARD_CENTER_OUT, 24, 12);
      points[2] = PointMotor(POINT_YARD_CENTER_IN_CROSSOVER_1, 24, 12);
      points[3] = PointMotor(POINT_YARD_CENTER_IN_CROSSOVER_2, 24, 12);
      points[4] = PointMotor(POINT_YARD_CENTER_OUT_CROSSOVER_1, 24, 12);
      points[5] = PointMotor(POINT_YARD_CENTER_OUT_CROSSOVER_2, 24, 12);
      break;
      
    case B110000:    
      points[0] = PointMotor(POINT_YARD_INNER_LEFT_1, 24, 12);
      points[1] = PointMotor(POINT_YARD_OUTER_LEFT_1, 24, 12);
      points[2] = PointMotor(POINT_YARD_INNER_LEFT_2, 24, 12);
      points[3] = PointMotor(POINT_YARD_OUTER_LEFT_2, 24, 12);  
      break;
  }

  //Setup I2C
  Wire.begin(node_id);
  Wire.onReceive(ReceiveEvent);

  Serial.println("[Setup] Point Node Started. i2c Address " + String(node_id) + ".");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(50);

  if (i2cCommandWaiting == 1) {
    processMessage(i2cCommand);
    i2cCommandWaiting = 0;
  }
  
}

void ReceiveEvent(int howMany) {
  int command = 0;
  command = command + Wire.read();
  Serial.println("[RECEIVE_EVENT] i2c command received " + String(command, BIN) + ", variable howMany = " + String(howMany));
  i2cCommand = command;
  i2cCommandWaiting = 1;
}

void processMessage(int msg) {

  int cmd = msg & DATA_COMMAND;
  int id = msg & DATA_ID;

  for (int i = 0; i <= 7; i++) {
    if (points[i].point_exists() == 1) {
      continue;
    } else {
      if (points[i].getID() == id) {
        if (cmd == DATA_COMMAND) {
          points[i].setStraight();
        } else {
          points[i].setTurnout();
        }
        break;
      }      
    }
  }
}
