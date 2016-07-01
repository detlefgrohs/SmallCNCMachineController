#include <LinkedList.h>
#include <TaskScheduler.h>

/*
  SmallCNCMachineController

  This is a 2 axis machine that will control a laser module (either .5 or 2 Watt) (40 Watt CO2 to Follow ;-) )
*/

const int DelayMSecs = 1;

const int HeartbeatLedPin = 13;

const int MotorEnablePin = 12;

const int XAxisDirPin = 11;
const int XAxisStpPin = 10;
const int YAxisDirPin = 9;
const int YAxisStpPin = 8;

const int Forward = 0;
const int Backward = 1;

const int XAxis = 0;
const int YAxis = 1;

// ======================================================================

// TaskScheduler 
Scheduler TaskRunner;

// Callback Prototypes
void SerialController();
void Controller();
void XAxisController();
void YAxisController();

// Tasks
Task SerialControllerTask(500, TASK_FOREVER, &SerialController);
Task ControllerTask(250, TASK_FOREVER, &Controller);
Task XAxisControllerTask(1, TASK_FOREVER, &XAxisController);
Task YAxisControllerTask(1, TASK_FOREVER, &YAxisController);

// ======================================================================

// Message Handling Functioning
String CurrentMessage = "";
LinkedList<String> *MessageQueue = new LinkedList<String>();

// ======================================================================

// Movement stuff...
int XAxisStepCurrentPosition = 0;
int XAxisStepDesiredPosition = 0;
int XAxisRate = 1;
int XAxisCount = 0;

int YAxisStepCurrentPosition = 0;
int YAxisStepDesiredPosition = 0;
int YAxisRate = 1;
int YAxisCount = 0;

// ======================================================================

void setup() {

  pinMode(HeartbeatLedPin, OUTPUT);

  pinMode(MotorEnablePin, OUTPUT);

  pinMode(XAxisDirPin, OUTPUT);
  pinMode(XAxisStpPin, OUTPUT);
  pinMode(YAxisDirPin, OUTPUT);
  pinMode(YAxisStpPin, OUTPUT);

  // Disable the Motors by Default
  digitalWrite(MotorEnablePin, HIGH);

  // Set starting state of all pins...
  digitalWrite(XAxisDirPin, LOW);
  digitalWrite(XAxisStpPin, LOW);
  digitalWrite(YAxisDirPin, LOW);
  digitalWrite(YAxisStpPin, LOW);

  // Setup the Serial communications
  Serial.begin(115200);   // What should this actually be?
  while (!Serial)
    ; // Wait for the serial port to connect

  // Setup the tasks
  TaskRunner.init();
  TaskRunner.addTask(SerialControllerTask);
  TaskRunner.addTask(ControllerTask);
  TaskRunner.addTask(XAxisControllerTask);
  TaskRunner.addTask(YAxisControllerTask);

  SerialControllerTask.enable();
  ControllerTask.enable();
  XAxisControllerTask.enable();
  YAxisControllerTask.enable();
}

void loop() {
  TaskRunner.execute();
}

void SerialController() {
    while (Serial.available()) {
      char inChar = Serial.read();

      if ((inChar == 10) || (inChar == 13))
      {
        if (CurrentMessage.length() > 0) {
          // Push the message to the end of the queue
          if (CurrentMessage.startsWith("M126")) {
            // Emergency Stop
            MessageQueue->clear();
            Serial.println("Emergency Stop.");
          } else {
            MessageQueue->add(CurrentMessage);
          }
          CurrentMessage = "";
        }
      }
      else
        // Append the current character to the current message
        CurrentMessage += String(inChar);
  }
}

void Circuit() {
  int steps = 200;

  for(int count = steps; count > 0; count--)
    StepXAxis(Forward);
  for(int count = steps; count > 0; count--)
    StepYAxis(Forward);
  for(int count = steps; count > 0; count--)
    StepXAxis(Backward);
  for(int count = steps; count > 0; count--)
    StepYAxis(Backward);
}

void Controller() {
  // Heartbeat
  if (digitalRead(HeartbeatLedPin))
    digitalWrite(HeartbeatLedPin, LOW);
  else
    digitalWrite(HeartbeatLedPin, HIGH);

  if (MessageQueue->size() > 0 && !IsMachineMoving()) {
    // Pop the first message in the message queue
    String message = MessageQueue->remove(0);

    if (message.startsWith("M0")) {
      Serial.println("Motors off.");
      digitalWrite(MotorEnablePin, HIGH);
    } else if (message.startsWith("M114")){
      Serial.print("X:");
      Serial.print(XAxisStepCurrentPosition);
      Serial.print(" Y:");
      Serial.print(YAxisStepCurrentPosition);
      Serial.println();
    } else if (message.startsWith("M1")){
      Serial.println("Motors on.");
      digitalWrite(MotorEnablePin, LOW);
    } else if (message.startsWith("G0")){
      int moveX = message.substring(message.indexOf("X") + 1).toInt();
      int moveY = message.substring(message.indexOf("Y") + 1).toInt();
      
      Serial.print("Rapid Move X:");
      Serial.print(moveX);
      Serial.print(" Y:");
      Serial.print(moveY);
      Serial.println();

      XAxisStepDesiredPosition = moveX;
      XAxisCount = 0;
      XAxisRate = 1;
      
      YAxisStepDesiredPosition = moveY;
      YAxisCount = 0;
      YAxisRate = 1;
    } else if (message.startsWith("G1")){
      int moveX = message.substring(message.indexOf("X") + 1).toInt();
      int moveY = message.substring(message.indexOf("Y") + 1).toInt();
      
      Serial.print("Move X:");
      Serial.print(moveX);
      Serial.print(" Y:");
      Serial.print(moveY);
      Serial.println();

      XAxisStepDesiredPosition = moveX;
      XAxisCount = 0;
      XAxisRate = 10;
      
      YAxisStepDesiredPosition = moveY;
      YAxisCount = 0;
      YAxisRate = 10;
    } else {
      Serial.print("Unprocessed Message: '");
      Serial.print(message);
      Serial.println("'");
    }
  }
}

boolean IsMachineMoving() {
  if ((XAxisStepDesiredPosition != XAxisStepCurrentPosition) ||
      (YAxisStepDesiredPosition != YAxisStepCurrentPosition))
    return true;

  return false;
}

void XAxisController() {
  if (XAxisStepDesiredPosition != XAxisStepCurrentPosition) {
    XAxisCount--; // Decrement the count so we know the next pulse that we need...
    if (XAxisCount <= 0) {
      XAxisCount = XAxisRate; // Set the count to the rate...

      // Set the direction
      if ((XAxisStepDesiredPosition > XAxisStepCurrentPosition) && digitalRead(XAxisDirPin))
        digitalWrite(XAxisDirPin, LOW);
      if ((XAxisStepDesiredPosition < XAxisStepCurrentPosition) && !digitalRead(XAxisDirPin))
        digitalWrite(XAxisDirPin, HIGH);

      // Now step the motor and update the position on the return to low...
      if (digitalRead(XAxisStpPin)) {
        digitalWrite(XAxisStpPin, LOW);

        if (XAxisStepDesiredPosition > XAxisStepCurrentPosition)
          XAxisStepCurrentPosition ++;
        else
          XAxisStepCurrentPosition --;
      } else {
        digitalWrite(XAxisStpPin, HIGH);
      }
    }
  }
}

void YAxisController() {
  if (YAxisStepDesiredPosition != YAxisStepCurrentPosition) {
    YAxisCount--; // Decrement the count so we know the next pulse that we need...
    if (YAxisCount <= 0) {
      YAxisCount = YAxisRate; // Set the count to the rate...

      // Set the direction
      if ((YAxisStepDesiredPosition > YAxisStepCurrentPosition) && digitalRead(YAxisDirPin))
        digitalWrite(YAxisDirPin, LOW);
      if ((YAxisStepDesiredPosition < YAxisStepCurrentPosition) && !digitalRead(YAxisDirPin))
        digitalWrite(YAxisDirPin, HIGH);

      // Now step the motor and update the position on the return to low...
      if (digitalRead(YAxisStpPin)) {
        digitalWrite(YAxisStpPin, LOW);

        if (YAxisStepDesiredPosition > YAxisStepCurrentPosition)
          YAxisStepCurrentPosition ++;
        else
          YAxisStepCurrentPosition --;
      } else {
        digitalWrite(YAxisStpPin, HIGH);
      }
    }
  }
}

void StepMotor(int axis, int stepDirection) {
  switch(axis) {
    case XAxis: 
      StepXAxis(stepDirection);
      break;
    case YAxis:
      StepYAxis(stepDirection);
      break;
  }
}

void StepXAxis(int stepDirection) {
  SingleStepMotor(XAxisStpPin, XAxisDirPin, stepDirection);
}

void StepYAxis(int stepDirection) {
  SingleStepMotor(YAxisStpPin, YAxisDirPin, stepDirection);
}

void SingleStepMotor(int stpPin, int dirPin, int stepDirection) {
  if (stepDirection == Forward && digitalRead(dirPin))
    digitalWrite(dirPin, LOW);
  if (stepDirection == Backward && !digitalRead(dirPin))
    digitalWrite(dirPin, HIGH);

  digitalWrite(stpPin, HIGH);
  delay(DelayMSecs);
  digitalWrite(stpPin, LOW);
  delay(DelayMSecs);
}

