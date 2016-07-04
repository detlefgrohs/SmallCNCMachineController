
/*
  SmallCNCMachineController

  This is a 2 axis machine that will control a laser module (either .5 or 2 Watt) (40 Watt CO2 to Follow ;-) )
*/
#include <LinkedList.h>
#include <TaskScheduler.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
//#include <Adafruit_MotorShield.h>

class Coordinate {
  public:
    long X;
    long Y;

    long int asLongArray[2];

    void toLongArray();
};

const int DelayuSecs = 100;

const int HeartbeatLedPin = 13;

const int EnableSteppers = 12;

const int XAxisStepperDirPin = 11;
const int XAxisStepperStpPin = 10;

const int YAxisStepperDirPin = 9;
const int YAxisStepperStpPin = 8;

// ======================================================================

// TaskScheduler 
Scheduler TaskRunner;

// Callback Prototypes
void SerialController();
void Controller();

// Tasks
Task SerialControllerTask(500, TASK_FOREVER, &SerialController);
Task ControllerTask(250, TASK_FOREVER, &Controller);

// ======================================================================

// Message Handling Functioning
String CurrentMessage = "";
LinkedList<String> *MessageQueue = new LinkedList<String>();

// ======================================================================
//Adafruit_MotorShield AFMStop(0x60); // Default address, no jumpers

//Adafruit_StepperMotor *XAxisStepper = AFMStop.getStepper(200, 1);
//Adafruit_StepperMotor *YAxisStepper = AFMStop.getStepper(200, 2);

void XAxisForward() {  
  //XAxisStepper->onestep(FORWARD, SINGLE);
}
void XAxisBackward() {  
  //XAxisStepper->onestep(BACKWARD, SINGLE);
}
void YAxisForward() {  
  //YAxisStepper->onestep(FORWARD, SINGLE);
}
void YAxisBackward() {  
  //YAxisStepper->onestep(BACKWARD, SINGLE);
}

//AccelStepper XAxisController(XAxisForward, XAxisBackward);
AccelStepper XAxisController(AccelStepper::DRIVER, XAxisStepperStpPin, XAxisStepperDirPin, 0, 0, true);
//AccelStepper YAxisController(YAxisForward, YAxisBackward);
AccelStepper YAxisController(AccelStepper::DRIVER, YAxisStepperStpPin, YAxisStepperDirPin, 0, 0, true);

MultiStepper XYAxis;

//long Coordinates[2];
void Coordinate::toLongArray() {
  asLongArray[0] = X;
  asLongArray[1] = Y;
}

void setup() {

  pinMode(HeartbeatLedPin, OUTPUT);

  // Setup the Serial communications
  Serial.begin(115200);   // What should this actually be?
  while (!Serial)
    ; // Wait for the serial port to connect

  // Setup the tasks
  TaskRunner.init();
  TaskRunner.addTask(SerialControllerTask);
  TaskRunner.addTask(ControllerTask);

  SerialControllerTask.enable();
  ControllerTask.enable();

  // A4988 Stepper Driver
  pinMode(EnableSteppers, OUTPUT);
  pinMode(XAxisStepperDirPin, OUTPUT);
  pinMode(XAxisStepperStpPin, OUTPUT);
  pinMode(YAxisStepperDirPin, OUTPUT);
  pinMode(YAxisStepperStpPin, OUTPUT);

  // Disable Steppers by Default
  digitalWrite(EnableSteppers, HIGH); 
  
  //
  //AFMStop.begin();

  // Setup steppers
  XAxisController.setMinPulseWidth(500);
  //XAxisController.setMaxSpeed(200.0);
  //XAxisController.setAcceleration(100.0);
  
  YAxisController.setMinPulseWidth(500);
  //YAxisController.setMaxSpeed(200.0);
  //YAxisController.setAcceleration(100.0);

  XYAxis.addStepper(XAxisController);
  XYAxis.addStepper(YAxisController);
}

void loop() {
  TaskRunner.execute();
  XYAxis.run();
}

void SerialController() {
    while (Serial.available()) {
      char inChar = Serial.read();

      if ((inChar == 10) || (inChar == 13))
      {
        if (CurrentMessage.length() > 0) {
          // Push the message to the end of the queue
          if (CurrentMessage.startsWith("M124")) {
            // Emergency Stop
            MessageQueue->clear();
            Serial.println("Emergency Stop.");
            MotorsOff();
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

int MessageNumber(String message) {
  return message.substring(1).toInt();
}

Coordinate getCurrentCoordinates() {
  Coordinate coordinate;
  coordinate.X = XAxisController.currentPosition();
  coordinate.Y = YAxisController.currentPosition();
  return coordinate;
}

void printCoordinates(Coordinate coordinate) {
      Serial.print("X:");
      Serial.print(coordinate.X);
      Serial.print(" Y:");
      Serial.print(coordinate.Y);
      Serial.println();
}

bool MessageM(String message) {
  switch (MessageNumber(message)) {
    case 17:
      Serial.println("Motors on.");
      MotorsOn();
      return true;
    case 18:
      Serial.println("Motors off.");
      MotorsOff();
      return true;
    case 114:
      printCoordinates(getCurrentCoordinates());
      return true;
  }
  return false;
}

Coordinate parseCoordinate(String message, Coordinate currentCoordinate) {
  int indexOfX = message.indexOf("X");
  int indexOfY = message.indexOf("Y");

  if (indexOfX > -1)
    currentCoordinate.X = message.substring(indexOfX + 1).toInt();

  if (indexOfY > -1)
    currentCoordinate.Y = message.substring(indexOfY + 1).toInt();

  return currentCoordinate;
}

bool MessageG(String message) {
  Coordinate coordinate;
  
  switch (MessageNumber(message)) {
    case 0:
      coordinate = parseCoordinate(message, getCurrentCoordinates());

      Serial.print("Rapid Move ");
      printCoordinates(coordinate);

      XAxisController.setMaxSpeed(1000.0);
      YAxisController.setMaxSpeed(1000.0);

      coordinate.toLongArray();
      XYAxis.moveTo(coordinate.asLongArray);
      return true;
    case 1:
      coordinate = parseCoordinate(message, getCurrentCoordinates());

      Serial.print("Move ");
      printCoordinates(coordinate);

      XAxisController.setMaxSpeed(100.0);
      YAxisController.setMaxSpeed(100.0);

      coordinate.toLongArray();
      XYAxis.moveTo(coordinate.asLongArray);
      return true;
  }
  return false;
}

bool MessageV(String message) {
  Serial.println("Version 0.0 Mk 1");
  return true;
}

void Controller() {
  // Heartbeat
  if (digitalRead(HeartbeatLedPin)) digitalWrite(HeartbeatLedPin, LOW);
  else                              digitalWrite(HeartbeatLedPin, HIGH);

  // Process Messages
  if (MessageQueue->size() > 0 && !IsMachineMoving()) {
    // Pop the first message in the message queue
    String message = MessageQueue->remove(0);

    bool processed = false;
    if (message.startsWith("M"))      processed = MessageM(message);
    else if (message.startsWith("G")) processed = MessageG(message);
    else if (message.startsWith("V")) processed = MessageV(message);
   
    if (!processed) {
      Serial.print("Unprocessed Message: '");
      Serial.print(message);
      Serial.println("'");
    }
  }
}

boolean IsMachineMoving() {
  if ((XAxisController.distanceToGo() == 0) &&
      (YAxisController.distanceToGo() == 0))
    return false;

  return true;
}

void MotorsOff() {
  //XAxisController.disableOutputs();
  //YAxisController.disableOutputs();

  digitalWrite(EnableSteppers, HIGH); 
}

void MotorsOn() {
  //XAxisController.enableOutputs();
  //YAxisController.enableOutputs();

  digitalWrite(EnableSteppers, LOW);
}

void LaserOff() {

}

void MachineOff() {
  MotorsOff();
  LaserOff();
}

