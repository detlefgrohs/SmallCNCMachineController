#include <TaskScheduler.h>

/*
  SmallCNCMachineController

  This is a 2 axis machine that will control a laser module (either .5 or 2 Watt) (40 Watt CO2 to Follow ;-) )
*/

const int DelayMSecs = 1;

const int LedPin = 13;

const int MotorEnablePin = 12;

const int XAxisDirPin = 11;
const int XAxisStpPin = 10;
const int YAxisDirPin = 9;
const int YAxisStpPin = 8;

const int Forward = 0;
const int Backward = 1;

const int XAxis = 0;
const int YAxis = 1;

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

Scheduler TaskRunner;

void setup() {

  pinMode(LedPin, OUTPUT);

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
  Serial.begin(9600);   // What should this actually be?
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
      int inChar = Serial.read();

      switch(inChar) {
        case char('O'):
          Serial.println("Enable Motors");
          digitalWrite(MotorEnablePin, LOW);
          break;
        case char('o'):
          Serial.println("Disable Motors");
          digitalWrite(MotorEnablePin, HIGH);
          break;
        case char('X'):
          Serial.println("Jog X Forward");
          StepXAxis(Forward);
          break;
        case char('x'):
          Serial.println("Jog X Backward");
          StepXAxis(Backward);
          break;
        case char('L'):
          Circuit();
          break;
        default:
          Serial.print("Unprocessed: ");
          Serial.println(inChar);
          break;
      }
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
  if (digitalRead(LedPin))
    digitalWrite(LedPin, LOW);
  else
    digitalWrite(LedPin, HIGH);
    
}

void XAxisController() {

}

void YAxisController() {

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

