/*
  SmallCNCMachineController

  This is a 2 axis machine that will control a laser module (either .5 or 2 Watt) (40 Watt CO2 to Follow ;-) )
*/

const int DelayMSecs = 1;

const int MotorEnablePin = 13;

const int XAxisDirPin = 13;
const int XAxisStpPin = 13;
const int YAxisDirPin = 13;
const int YAxisStpPin = 13;

const int Forward = 0;
const int Backward = 1;

const int XAxis = 0;
const int YAxis = 1;

void setup() {

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
}

void loop() {



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

