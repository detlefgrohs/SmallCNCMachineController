/*
  SmallCNCMachineController

  This is a 2 axis machine that will control a laser module (either .5 or 2 Watt) (40 Watt CO2 to Follow ;-) )
*/

DelayMSecs = 1;

MotorEnablePin = 13;

XAxisDirPin = 13;
XAxisStpPin = 13;
YAxisDirPin = 13;
YAxisStpPin = 13;

enum StepDirection {
  Forward,
  Backward
}

enum Axis {
  X,
  Y
}

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

void StepMotor(Axis axis, StepDirection stepDirection) {
  switch(axis) {
    case Axis.X: 
      StepXAxis(stepDirection);
      break;
    case Axis.Y:
      StepYAxis(stepDirection);
      break;
  }
}

void StepXAxis(StepDirection stepDirection) {
  SingleStepMotor(XAxisStpPin, XAxisDirPin, stepDirection);
}

void StepYAxis(StepDirection stepDirection) {
  SingleStepMotor(YAxisStpPin, YAxisDirPin, stepDirection);
}

void SingleStepMotor(int stpPin, int dirPin, StepDirection stepDirection) {
  if (stepDirection == StepDirection.Forward && digitalRead(dirPin))
    digitalWrite(dirPin, LOW);
  if (stepDirection == StepDirection.Backward && !digitalRead(dirPin))
    digitalWrite(dirPin, HIGH);

  digitalWrite(stpPin, High);
  delay(DelayMSecs);
  digitalWrite(stpPin, Low);
  delay(DelayMSecs);
}

