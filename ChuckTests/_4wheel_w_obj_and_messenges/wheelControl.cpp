#include <Arduino.h>

#include "wheelControl.h"


wheelControl::wheelControl(){
  Hz = 10;
  Pgain = 0.21;
  Dgain = -0.01;
  pulsesPerRot = 45;
  RPMerror = 0;
  DerRPMerror = 0;
  lastCycleSpeedCheck = 0;
  speedCheckfloat;
  lastcycleRPM = 0;
  speedCheck = 0;
  forwardBackward = false;
  backToZero = false;
  cyclesPerRot = 3336;
  RPMPlanMot = 0;
  lastEncPos = 0;
  currentPlanMotPos = 0;
  lastPlanMotPos = 0;
  deltaPlanMotPos = 0;
  derPlanMotPos = 0;
  motorSpeed = 0;
  intmotorSpeed = 0;
  PWheelGain = 2.5;
  DWheelGain = -0.05;  
}

returnVariables wheelControl::calculate(float newDesiredRPM, float desiredAngle, float avgDeltaTime, unsigned long lastInterrupt, int encPos) {
  using namespace std;

  /*
 * The next section looks to see if the desired RPM is in the same direction or not. IF it is not, it will set the 
 *  desired RPM to 0 this pass and then next pass if will switch the forward and back variable and the desired RPM.
 *  By setting the desried RPM to 0, we also we restart the interrupt logic and go open loop on the initial speedcheck
 *  value (set later on). 
 */
    
    if (newDesiredRPM < 0)
    {
      if (lastDesiredRPM > 0) {
        desiredRPM = 0;
      }
      else {
        forwardBackward = HIGH;
        desiredRPM = -1 * newDesiredRPM; //This is to make the rpm positive for PWM calc
      }
    }
    else {
      if (lastDesiredRPM < 0) {
        desiredRPM = 0;
      }
      else {
        desiredRPM = newDesiredRPM;
        forwardBackward = LOW;
      }
    }

    lastDesiredRPM = newDesiredRPM;

  /* 
 *  This if statement is looking for the case that the wheel has stopped and we are not getting any more interrupts, 
 *  and averagedeltatime is not valid (old). So, in this case we will set the current RPM to zero. If not, then we will
 *  calcultate the RPM - the 60,000 is to convert from millisec to minutes.
 */
    if (( millis() - lastInterrupt) > 200) {
      curRPM = 0;
    }
    else {
      curRPM = (1 / ((float)avgDeltaTime * pulsesPerRot)) * 60000;
    }
// IF the wheel has stopped, we need to do a new start up sequence.

    if (curRPM == 0) {
      backToZero = true;
    }

// Calculate the delta (error) in RPM and in the derivative of RPM.

    RPMerror = desiredRPM - curRPM;
    DerRPMerror = curRPM - lastcycleRPM;
/*
 *  In the case that we are starting from zero, or our desired RPM is far from our current RPM, then we will use
 *  open loop control and set the PWM output to value that is close to the estimated PWM output for that RPM.
 *  The motor has more of a lag when it tries to slow down and hence needs a longer open loop control for this
 *  case (15 vs. 40). Also if the desired RPM is zero, then just set the PWM output to zero. The last "else"
 *  is the PD controller.
 */

    if ((avgDeltaTime > 1000)or (backToZero) or (RPMerror > 40.0) or (RPMerror < -15.0)) {
      if (RPMerror > 0) {
         speedCheckfloat = ((desiredRPM * 0.9) + 7) * 0.9; //these numbers came from testing on the stand
      }
      else {
         speedCheckfloat = ((desiredRPM * 0.9) + 7) * 1.1; // thesenumbers came from testing on the stand
      }
    }
    else if (desiredRPM == 0) {
      speedCheckfloat = 0;
    }
    else {
      speedCheckfloat = lastCycleSpeedCheck + RPMerror * Pgain + DerRPMerror * Dgain;
    }

     lastcycleRPM = curRPM;

// Limit the PWM output to 0 to 255.
   
    if (speedCheckfloat > 255) {
      speedCheckfloat = 255.0;
    }
     if (speedCheckfloat < -0.5) {
      speedCheckfloat = -0.5;
    }
    
    lastCycleSpeedCheck = speedCheckfloat;
    
// this will round the float version to the integer version of the variable.

    speedCheck = (long)(speedCheckfloat + 0.5);

/*
 * THis is the logic for the planetary motor - setting the angle of the wheel
 */

    deltaEncPos = (float)(encPos - lastEncPos); // change in encoder position is used for RPM
    
    currentPlanMotPos = lastPlanMotPos + (deltaEncPos * 360.0 / cyclesPerRot);  //calculates current position of wheel

    // We need to keep the angle between +- 180 deg
    
    if (currentPlanMotPos > 180.0) {
      currentPlanMotPos = currentPlanMotPos - 360.0;
    }
    
    if (currentPlanMotPos < -180.0) {
      currentPlanMotPos = currentPlanMotPos + 360.0;
    }
    lastPlanMotPos = currentPlanMotPos;  // used to calculate delta encoder pos, which is use to calcualte RPM
    
    RPMPlanMot = (deltaEncPos / cyclesPerRot) * (float)Hz * 60.0; //RPM calculation
    lastEncPos = encPos;
    float desiredWheelPos = desiredAngle; //Assumption that the wheel position is really the desired angle 
    float currentWheelPos = currentPlanMotPos; //Same type of assumption
    float deltaWheelPos = desiredWheelPos - currentWheelPos;  // this is the proportional error for the PD controller

//    this might be necessary depending upon overall wheel logic
     if (deltaWheelPos > 180.0) {
      deltaWheelPos = deltaWheelPos - 360.0;
    }
    
    if (deltaWheelPos < -180.0) {
      deltaWheelPos = deltaWheelPos + 360.0;
    }
    float RPM = RPMPlanMot;
    derPlanMotPos = RPM * 6;  //  360 deg/min / 60sec/min = deg/sec  I hope. this is the derivative error for the PD controller
    
    motorSpeed = deltaPlanMotPos * PWheelGain + derPlanMotPos * DWheelGain; // PD controller

    //Need to make it an integer and round to the nearest (hence the 0.5). The addition of 5 is due to you need that much to make the motor turn
    intmotorSpeed = (long)(abs(motorSpeed) +0.5) + 5; 

// limit to 255
    if (intmotorSpeed > 255) {  
      intmotorSpeed = 255;
    }
     if (motorSpeed >= 0) {
      PlanMotDir = LOW;
    }
    else {
      PlanMotDir = HIGH;
    }

// return the desired parameters  
  struct returnVariables returnStruct;
  returnStruct.speedCheck = speedCheck; 
  returnStruct.forwardBackward = forwardBackward;
  returnStruct.currentWheelRPM = curRPM;
  returnStruct.currentWheelAngle = currentPlanMotPos;//Changed to current
  returnStruct.motorSpeed = intmotorSpeed;
  returnStruct.planMotorDirection = PlanMotDir;
  returnStruct.backToZero = backToZero; 
  return returnStruct;   
}

