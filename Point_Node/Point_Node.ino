#include <Wire.h>
#include <Servo.h>

//Settings
const int CODE_VERSION = 7;
const char CODE_VERSION_DATE[] = "2025-04-23";
int node_id = 0;
const int TEST_MODE = 0;

//Constants
const int DATA_PIN = B0001111;
const int DATA_NODE = B0110000;
const int DATA_COMMAND = B1000000;
const int DATA_ID = B0111111;

const int STATE_STRAIGHT = -1;
const int STATE_UNKNOWN = 0;
const int STATE_TURNOUT = 1;

const int I2C_MAX_COMMANDS = 10;

//Switch Pins
const int PIN_SWITCH_1 = 10;
const int PIN_SWITCH_2 = 11;

//Point Motor References - Node 00
const int LEEDS_OUTER_LOOP_POINTS = B000010;
const int LEEDS_FACING_CROSSOVER_INNER_POINT = B000011;
const int LEEDS_FACING_CROSSOVER_OUTER_POINT = B000100;
const int LEEDS_TRAILING_CROSSOVER_OUTER_POINT = B000101;
const int LEEDS_TRAILING_CROSSOVER_INNER_POINT = B000110;
const int LEEDS_INNER_LOOP_POINTS = B000111;

//Point Motor References - Node 01
const int POINT_YORK_INNER_LOOP = B010010;
const int POINT_YORK_CROSSOVER_FACING_A = B010011;
const int POINT_YORK_CROSSOVER_FACING_B = B010100;
const int POINT_YORK_CROSSOVER_TRAILING_A = B010101;
const int POINT_YORK_CROSSOVER_TRAILING_B = B010110;
const int POINT_YORK_OUTER_LOOP = B010111;

//Point Motor References - Node 10
const int YORK_SIDING = B100010;
const int VIADUCT_CROSSOVEER_YORK = B100011;
const int VIADUCT_CROSSOVER_LEEDS = B100100;
const int LEEDS_SIDING_POINTS_B = B100101;
const int LEEDS_SIDING_POINTS_C = B100110;

//Point Motor References - Node 11
const int POINT_FRONT_INNER_MAIN_CROSSOVER = B110010;
const int POINT_FRONT_OUTER_MAIN_CROSSOVER = B110011;
const int POINT_FRONT_INNER_TURNOUT = B110100;
const int POINT_FRONT_INNER_TRAILING_CROSSOVER = B110101;
const int POINT_FRONT_OUTER_TRAILING_CROSSOVER = B110110;
const int POINT_FRONT_BAY_PLATFORM = B110111;

//Servo controler
Servo _servo;

//---------------------------------------------------------------------------------------------------
//Class PointMotor

class PointMotor {
  public:
    PointMotor();
    PointMotor(int id, int angleCentre, int angleThrow, int reverse);
    int point_exists();
    void setStraight();
    void setTurnout();
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
    int _reverse; 

    void setServoAngle();
};

PointMotor::PointMotor() {
  _exists = 0;
}

PointMotor::PointMotor(int id, int angleCentre = 24, int angleThrow = 12, int reverse = 1) {
  _id = id;
  _pin = _id & DATA_PIN;
  _node = _id & DATA_NODE;
  _angleCentre = angleCentre;
  _angleThrow = angleThrow;
  _exists = 1;
  _reverse = reverse;
  Serial.println("[POINT_MOTOR_CREATE] Created Point Motor ID " + String(id));
}

int PointMotor::point_exists() {
  return _exists;
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
  _servo.write(_state * _reverse * _angleThrow + _angleCentre);
  Serial.println("[SET_SERVO_ANGLE] Pin " + String(_pin) + " set to " + String(_state * _reverse * _angleThrow + _angleCentre));
  delay(200);
  _servo.detach();
}

//---------------------------------------------------------------------------------------------------

PointMotor points[8] = {PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor(), PointMotor()};

volatile int i2cCommand[10];
int i2cCommandSetNext;
int i2cCommandReadNext;


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
    //Right hand board. Leeds end points
      points[0] = PointMotor(LEEDS_OUTER_LOOP_POINTS, 24, 12); //0010 -- 2
      points[1] = PointMotor(LEEDS_FACING_CROSSOVER_INNER_POINT, 24, 12); //0011 -- 3
      points[2] = PointMotor(LEEDS_FACING_CROSSOVER_OUTER_POINT, 24, 12); //0100 -- 4
      points[3] = PointMotor(LEEDS_TRAILING_CROSSOVER_OUTER_POINT, 24, 12); //0101 -- 5
      points[4] = PointMotor(LEEDS_TRAILING_CROSSOVER_INNER_POINT, 24, 12); //0110 -- 6
      points[5] = PointMotor(LEEDS_INNER_LOOP_POINTS, 24, 12); //0111 -- 7
      break;

    //Left hand board. York end points
    case B010000:
      points[0] = PointMotor(POINT_YORK_INNER_LOOP, 24, 12); //0010 -- 2
      points[1] = PointMotor(POINT_YORK_CROSSOVER_FACING_A, 24, 12, -1); //0011 -- 3 -- Reversed
      points[2] = PointMotor(POINT_YORK_CROSSOVER_FACING_B, 24, 12); //0100 -- 4
      points[3] = PointMotor(POINT_YORK_CROSSOVER_TRAILING_A, 24, 12); //0101 -- 5
      points[4] = PointMotor(POINT_YORK_CROSSOVER_TRAILING_B, 24, 12, -1); //0110 -- 6 -- Reversed
      points[5] = PointMotor(POINT_YORK_OUTER_LOOP, 24, 12, -1); //0111 -- 7 -- Reversed
      break;
      
    case B100000:
    //Right hand board. Bit of everything points
      points[0] = PointMotor(YORK_SIDING, 24, 12); //0010 -- 2
      points[1] = PointMotor(VIADUCT_CROSSOVEER_YORK, 24, 12); //0011 -- 3
      points[2] = PointMotor(VIADUCT_CROSSOVER_LEEDS, 24, 12); //0100 -- 4
      points[3] = PointMotor(LEEDS_SIDING_POINTS_B, 24, 12); //0101 -- 5
      points[4] = PointMotor(LEEDS_SIDING_POINTS_C, 24, 12); //0110 -- 6
      break;

    case B110000:    
    //Unused
      //points[0] = PointMotor(POINT_YARD_INNER_LEFT_1, 24, 12);
      //points[1] = PointMotor(POINT_YARD_OUTER_LEFT_1, 24, 12);
      //points[2] = PointMotor(POINT_YARD_INNER_LEFT_2, 24, 12);
      //points[3] = PointMotor(POINT_YARD_OUTER_LEFT_2, 24, 12);  
      break;
  }

  //Setup I2C
  Wire.begin(node_id);
  Wire.onReceive(ReceiveEvent);

  Serial.println("[Setup] Point Node Started. i2c Address " + String(node_id) + ".");

  if (TEST_MODE == 1) {
    Serial.println("[Setup] Single servo (pin 3) test mode activated");
  }

  if (TEST_MODE == 2) {
    Serial.println("[Setup] All servo test mode activated");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(50);

  if (TEST_MODE == 1) {
    test_single_servo();
    return;
  }

  if (TEST_MODE == 2) {
    test_all_servo();
    return;
  }

  if (i2cCommand[i2cCommandReadNext] != 0) {
    processMessage(i2cCommand[i2cCommandReadNext]);
    i2cCommand[i2cCommandReadNext] = 0;
    i2cCommandReadNext ++;
    if (i2cCommandReadNext == I2C_MAX_COMMANDS) {
      i2cCommandReadNext = 0;
    }
  }
  
}

void ReceiveEvent(int howMany) {
  int command = 0;
  while (Wire.available()) {
      char c = Wire.read();
      command += int(c);
      //Serial.println(int(c));
  }  
  //command = command + Wire.read();
  Serial.println("[RECEIVE_EVENT] i2c command received " + String(command, BIN) + ", variable howMany = " + String(howMany));

  i2cCommand[i2cCommandSetNext] = command;
  i2cCommandSetNext ++;
  if (i2cCommandSetNext == I2C_MAX_COMMANDS) {
    i2cCommandSetNext = 0;
  }
}

void processMessage(int msg) {

  int cmd = msg & DATA_COMMAND;
  int id = msg & DATA_ID;

  for (int i = 0; i <= 7; i++) {
    if (points[i].point_exists() != 1) {
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

void test_single_servo() {
  int cmd;
  
  //Set point to turnout
  cmd = DATA_COMMAND + node_id + 6;
  Serial.println("[SINGLE SERVO TEST] Setting point on pin 6 to turnout. (i2c command: " + String(cmd, BIN) + ")");
  processMessage(cmd);
  delay(1000);
  
  //Set point to straight
  cmd = node_id + 6;
  Serial.println("[SINGLE SERVO TEST] Setting point on pin 6 to straight. (i2c command: " + String(cmd, BIN) + ")");
  processMessage(cmd);
  delay(1000); 
}

void test_all_servo() {
  int cmd;

  //Set all points to turnout one at a time
  for (int i = 0; i <= 7; i++) {
      cmd = DATA_COMMAND + node_id + i + 2;
      Serial.println("[ALL SERVO TEST] Setting point on pin " + String(i + 2) + " to turnout. (i2c command: " + String(cmd, BIN) + ")");
      processMessage(cmd);
      delay(200);
  }

  //Set all points to straight one at a time
  for (int i = 0; i <= 7; i++) {
      cmd = node_id + i + 2;
      Serial.println("[ALL SERVO TEST] Setting point on pin " + String(i + 2) + " to straight. (i2c command: " + String(cmd, BIN) + ")");
      processMessage(cmd);
      delay(200);
  }
}
