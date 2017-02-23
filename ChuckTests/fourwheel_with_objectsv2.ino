
/* This software will control the four hub motors of the wheels and the four turn motors for the wheels. 
 *  Also it will feed back current speed and direction for each wheel. For now the getDesiredInfo()and writeCurrentInfo()mehtodws will be 
 *  a shell until that software has been completed. We could also easilty make this into a loop and do it for wheels 0 through 3.
 */
//#include "returnedVariables.h"
struct returnVariables {
  long speedCheck; 
  bool forwardBackward;
  float currentWheelRPM;
  float currentWheelAngle;
  int motorSpeed;
  bool planMotorDirection;
  bool backToZero;   
};


//#include "wheelControl.h"


class wheelControl {
  public:
  float Pgain;
  float Dgain;
  int pulsesPerRot;
  float RPMerror;
  float DerRPMerror;
  float lastCycleSpeedCheck;
  float speedCheckfloat;
  float lastcycleRPM;
  float curRPM;
  float lastDesiredRPM;
  float desiredRPM;
  int speedCheck;
  bool forwardBackward;
  bool backToZero;
  
  // inits for planetary motor
  
  float deltaEncPos;
  float cyclesPerRot;
  float RPMPlanMot;
  int lastEncPos;
  float currentPlanMotPos;
  float lastPlanMotPos;
  float deltaPlanMotPos;
  float derPlanMotPos;
  float motorSpeed;
  int intmotorSpeed;
  float PWheelGain;
  float DWheelGain;
  bool PlanMotDir;
  int Hz;

returnVariables calculate(float a, float b, float c, unsigned long d, int e);
wheelControl(); 
};

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


//#include "pinDef.h"


//Inits for pin setup.   These need to be determined when we set up the teensy, along with the interrupt numbers

int vrPinWheel0 = 3;   //hub motor controller pwm pin
int zfPinWheel0 = 24;   //hub motor controller dir pin
int hubIntWheel0 = 13;

int encAWheel0 = 14; 
int encBWheel0 = 33;   
int pwmPinWheel0 = 4;
int dirPinWheel0 = 25;

int vrPinWheel1 = 5;   //hub motor controller pwm pin
int zfPinWheel1 = 26;   //hub motor controller dir pin
int hubIntWheel1 = 15;

int encAWheel1 = 16;  
int encBWheel1 = 34;  
int pwmPinWheel1 = 6;
int dirPinWheel1 = 27;

int vrPinWheel2 = 7;    //hub motor controller pwm pin
int zfPinWheel2 = 28;    //hub motor controller dir pin
int hubIntWheel2 = 17;

int encAWheel2 = 18;  
int encBWheel2 = 35;  
int pwmPinWheel2 = 8;
int dirPinWheel2 = 29;

int vrPinWheel3 = 9;  //hub motor controller pwm pin
int zfPinWheel3 = 30;  //hub motor controller dir pin
int hubIntWheel3 = 19;

int encAWheel3 = 20; 
int encBWheel3 = 36;  
int pwmPinWheel3 = 10;
int dirPinWheel3 = 31;

// Inits for wheel 0 HUB motor interrupt routine



//Inits for all methods

int Hz = 10; //cycle time for main processing
unsigned long beginingTicWheel0 = millis();
unsigned long beginingTic = millis();
int countWheel0 = 0;
float SumWheel0 = 0;
unsigned long CurrentTimeWheel0 = 0;
unsigned long LastIntTimeWheel0 = millis();
boolean FirstPassWheel0 = true;
boolean backToZeroWheel0 = false;
float AvgDeltaTimeWheel0 = 10000001;
int iWheel0;
float DeltaTimeWheel0[40];

/*
 * This method is called when the interrupt for wheel 0's hub (hall sensor) goes high. It is calculating the average time 
 * between interrupts to get an RPM. However, it needs to handle the case when the wheel is stopped and starts up. Hence 
 * the first interrupt after it has been stopped can't be used to calculate a time bewtween interrups (we need two), so  
 * teh first time we will calculate a time between intrrupts and get an RPM is during the second interrupt.
 */

void counterWheel0() {
  // Need first pass logic to set the last interrupt time to get an accurate time between interrupts on the second pass
  if ((FirstPassWheel0 or backToZeroWheel0)) {
    LastIntTimeWheel0 = millis();
    AvgDeltaTimeWheel0 = 10000001; // this is just a really large number to signify the wheel is not moving.
    FirstPassWheel0 = false;
    backToZeroWheel0 = false;
  }
  else {

    /* Here we are calculating the time bewteen this interrupt and the last and storing that value in an array, with
        index of the variable count (which starts at 0). Then each time we need to get an average time between interrupts
        which is what the main loop needs, and since we don't know when that we be run conpared to the interrups. We also need to
        increment the counter for the next interrupt.
    */
    CurrentTimeWheel0 = millis();
    DeltaTimeWheel0[countWheel0] = (float)(CurrentTimeWheel0 - LastIntTimeWheel0);
    
    LastIntTimeWheel0 = CurrentTimeWheel0;
    for (iWheel0 = 0; iWheel0 <= countWheel0; iWheel0++) {
      SumWheel0 = SumWheel0 + DeltaTimeWheel0[iWheel0];  
    }
    AvgDeltaTimeWheel0 = SumWheel0 /(float)(countWheel0 + 1);
    SumWheel0 = 0;
    countWheel0++;
  }

}

// Inits for planetary motor interrupt routine wheel 0

int encposWheel0 = 0;

/*
 *  This function gets called when the interrupt encoder A for wheel 0 changes. It looks at the position of both
 *  encoders to determine the direction of the motor.
 */
void encoderWheel0(){

  if (digitalRead(encAWheel0) == digitalRead(encBWheel0)){
    encposWheel0++;
  }
  else {
    encposWheel0--;
  }
}

/*
 * This method is called when the interrupt for wheel 1's hub (hall sensor) goes high. It is calculating the average time 
 * between interrupts to get an RPM. However, it needs to handle the case when the wheel is stopped and starts up. Hence 
 * the first interrupt after it has been stopped can't be used to calculate a time bewtween interrups (we need two), so  
 * teh first time we will calculate a time between intrrupts and get an RPM is during the second interrupt.
 */

// Inits for wheel 1 interrupt routine
int countWheel1 = 0;
float SumWheel1 = 0;
unsigned long CurrentTimeWheel1 = 0;
unsigned long LastIntTimeWheel1 = millis();
boolean FirstPassWheel1 = true;
boolean backToZeroWheel1 = false;
float AvgDeltaTimeWheel1 = 10000001;
int iWheel1;
float DeltaTimeWheel1[40];

void counterWheel1() {
  // Need first pass logic to set the last interrupt time to get an accurate time between interrupts on the second pass
  if ((FirstPassWheel1 or backToZeroWheel1)) {
    LastIntTimeWheel1 = millis();
    AvgDeltaTimeWheel1 = 10000001; // this is just a really large number to signify the wheel is not moving.
    FirstPassWheel1 = false;
    backToZeroWheel1 = false;
  }
  else {

    /* Here we are calculating the time bewteen this interrupt and the last and storing that value in an array, with
        index of the variable count (which starts at 0). Then each time we need to get an average time between interrupts
        which is what the main loop needs, and since we don't know when that we be run conpared to the interrups. We also need to
        increment the counter for the next interrupt.
    */
    CurrentTimeWheel1 = millis();
    DeltaTimeWheel1[countWheel1] = (float)(CurrentTimeWheel1 - LastIntTimeWheel1);
    
    LastIntTimeWheel1 = CurrentTimeWheel1;
    for (iWheel1 = 0; iWheel1 <= countWheel1; iWheel1++) {
      SumWheel1 = SumWheel1 + DeltaTimeWheel1[iWheel1];  
    }
    AvgDeltaTimeWheel1 = SumWheel1 /(float)(countWheel1 + 1);
    SumWheel1 = 0;
    countWheel1++;
  }

}

// Inits for planetary motor interrupt routine wheel 0

int encposWheel1 = 0;

/*
 *  This function gets called when the interrupt encoder A for wheel 0 changes. It looks at the position of both
 *  encoders to determine the direction of the motor.
 */
void encoderWheel1(){

  if (digitalRead(encAWheel1) == digitalRead(encBWheel1)){
    encposWheel1++;
  }
  else {
    encposWheel1--;
  }
}

/*
 * This method is called when the interrupt for wheel 2's hub (hall sensor) goes high. It is calculating the average time 
 * between interrupts to get an RPM. However, it needs to handle the case when the wheel is stopped and starts up. Hence 
 * the first interrupt after it has been stopped can't be used to calculate a time bewtween interrups (we need two), so  
 * teh first time we will calculate a time between intrrupts and get an RPM is during the second interrupt.
 */

// Inits for wheel 2 interrupt routine
int countWheel2 = 0;
float SumWheel2 = 0;
unsigned long CurrentTimeWheel2 = 0;
unsigned long LastIntTimeWheel2 = millis();
boolean FirstPassWheel2 = true;
boolean backToZeroWheel2 = false;
float AvgDeltaTimeWheel2 = 10000001;
int iWheel2;
float DeltaTimeWheel2[40];

void counterWheel2() {
  // Need first pass logic to set the last interrupt time to get an accurate time between interrupts on the second pass
  if ((FirstPassWheel2 or backToZeroWheel2)) {
    LastIntTimeWheel2 = millis();
    AvgDeltaTimeWheel2 = 10000001; // this is just a really large number to signify the wheel is not moving.
    FirstPassWheel2 = false;
    backToZeroWheel2 = false;
  }
  else {

    /* Here we are calculating the time bewteen this interrupt and the last and storing that value in an array, with
        index of the variable count (which starts at 0). Then each time we need to get an average time between interrupts
        which is what the main loop needs, and since we don't know when that we be run conpared to the interrups. We also need to
        increment the counter for the next interrupt.
    */
    CurrentTimeWheel2 = millis();
    DeltaTimeWheel2[countWheel2] = (float)(CurrentTimeWheel2 - LastIntTimeWheel2);
    
    LastIntTimeWheel2 = CurrentTimeWheel2;
    for (iWheel2 = 0; iWheel2 <= countWheel2; iWheel2++) {
      SumWheel2 = SumWheel2 + DeltaTimeWheel2[iWheel2];  
    }
    AvgDeltaTimeWheel2 = SumWheel2 /(float)(countWheel2 + 1);
    SumWheel2 = 0;
    countWheel2++;
  }

}

// Inits for planetary motor interrupt routine wheel 0

int encposWheel2 = 0;

/*
 *  This function gets called when the interrupt encoder A for wheel 0 changes. It looks at the position of both
 *  encoders to determine the direction of the motor.
 */
void encoderWheel2(){

  if (digitalRead(encAWheel2) == digitalRead(encBWheel2)){
    encposWheel2++;
  }
  else {
    encposWheel2--;
  }
}

/*
 * This method is called when the interrupt for wheel 3's hub (hall sensor) goes high. It is calculating the average time 
 * between interrupts to get an RPM. However, it needs to handle the case when the wheel is stopped and starts up. Hence 
 * the first interrupt after it has been stopped can't be used to calculate a time bewtween interrups (we need two), so  
 * teh first time we will calculate a time between intrrupts and get an RPM is during the second interrupt.
 */

// Inits for wheel 3 interrupt routine
int countWheel3 = 0;
float SumWheel3 = 0;
unsigned long CurrentTimeWheel3 = 0;
unsigned long LastIntTimeWheel3 = millis();
boolean FirstPassWheel3 = true;
boolean backToZeroWheel3 = false;
float AvgDeltaTimeWheel3 = 10000001;
int iWheel3;
float DeltaTimeWheel3[40];

void counterWheel3() {
  // Need first pass logic to set the last interrupt time to get an accurate time between interrupts on the second pass
  if ((FirstPassWheel3 or backToZeroWheel3)) {
    LastIntTimeWheel3 = millis();
    AvgDeltaTimeWheel3 = 10000001; // this is just a really large number to signify the wheel is not moving.
    FirstPassWheel3 = false;
    backToZeroWheel3 = false;
  }
  else {

    /* Here we are calculating the time bewteen this interrupt and the last and storing that value in an array, with
        index of the variable count (which starts at 0). Then each time we need to get an average time between interrupts
        which is what the main loop needs, and since we don't know when that we be run conpared to the interrups. We also need to
        increment the counter for the next interrupt.
    */
    CurrentTimeWheel3 = millis();
    DeltaTimeWheel3[countWheel3] = (float)(CurrentTimeWheel3 - LastIntTimeWheel3);
    
    LastIntTimeWheel3 = CurrentTimeWheel3;
    for (iWheel3 = 0; iWheel3 <= countWheel3; iWheel3++) {
      SumWheel3 = SumWheel3 + DeltaTimeWheel3[iWheel3];  
    }
    AvgDeltaTimeWheel3 = SumWheel3 /(float)(countWheel3 + 1);
    SumWheel3 = 0;
    countWheel3++;
  }

}

// Inits for planetary motor interrupt routine wheel 3

int encposWheel3 = 0;

/*
 *  This function gets called when the interrupt encoder A for wheel 0 changes. It looks at the position of both
 *  encoders to determine the direction of the motor.
 */
void encoderWheel3(){

  if (digitalRead(encAWheel3) == digitalRead(encBWheel3)){
    encposWheel3++;
  }
  else {
    encposWheel3--;
  }
}

wheelControl* wheel0Control;
wheelControl* wheel1Control;
wheelControl* wheel2Control;
wheelControl* wheel3Control;

void setup() {
  // put your setup code here, to run once:
  // this sets the pins we are going to use and sets up the serial output for debugging.
  // setup for wheel 0

  // for hub motor
  pinMode(vrPinWheel0, OUTPUT); // this tells the teensy OS that these pins will be sued for output.
  pinMode(zfPinWheel0, OUTPUT);
  attachInterrupt(hubIntWheel0, counterWheel0, RISING); // this attaches interrupt 1 to the counterWO method (wheel 0 hub motor)

   //for planetary motor 

  pinMode(pwmPinWheel0, OUTPUT);
  pinMode(dirPinWheel0, OUTPUT);
  pinMode(encAWheel0, INPUT);
  pinMode(encBWheel0, INPUT);
  attachInterrupt(encAWheel0, encoderWheel0, CHANGE);
  
 // setup for wheel 1, hub motor
  
  pinMode(vrPinWheel1, OUTPUT); // this tells the teensy OS that these pins will be sued for output.
  pinMode(zfPinWheel1, OUTPUT);
  attachInterrupt(hubIntWheel1, counterWheel1, RISING); // this attaches interrupt 1 to the counterWO method (wheel 0 hub motor)

  //for planetary motor 

  pinMode(pwmPinWheel1, OUTPUT);
  pinMode(dirPinWheel1, OUTPUT);
  pinMode(encAWheel1, INPUT);
  pinMode(encBWheel1, INPUT);
  attachInterrupt(encAWheel1, encoderWheel1, CHANGE);
  

  // setup for wheel 2, hub motor
  
  pinMode(vrPinWheel2, OUTPUT); // this tells the teensy OS that these pins will be sued for output.
  pinMode(zfPinWheel2, OUTPUT);
  attachInterrupt(hubIntWheel2, counterWheel1, RISING); // this attaches interrupt 1 to the counterWO method (wheel 0 hub motor)

  //for planetary motor 

  pinMode(pwmPinWheel2, OUTPUT);
  pinMode(dirPinWheel2, OUTPUT);
  pinMode(encAWheel2, INPUT);
  pinMode(encBWheel2, INPUT);
  attachInterrupt(encAWheel2, encoderWheel2, CHANGE);
  
 // setup for wheel 3, hub motor
  
  pinMode(vrPinWheel3, OUTPUT); // this tells the teensy OS that these pins will be sued for output.
  pinMode(zfPinWheel3, OUTPUT);
  attachInterrupt(hubIntWheel3, counterWheel1, RISING); // this attaches interrupt 1 to the counterWO method (wheel 0 hub motor)

  //for planetary motor 

  pinMode(pwmPinWheel3, OUTPUT);
  pinMode(dirPinWheel3, OUTPUT);
  pinMode(encAWheel3, INPUT);
  pinMode(encBWheel3, INPUT);
  attachInterrupt(encAWheel3, encoderWheel0, CHANGE);
  

// setup for all wheels  
  
  Serial.begin(250000); // sets up a serial interface to print to the monitor for debugging
  // inputString.reserve(200);
  interrupts();  //Allows for intrrupts to call their associated methods

// need to intstaniate the four wheels
  wheel0Control = new wheelControl();
  wheel1Control = new wheelControl();
  wheel2Control = new wheelControl();
  wheel3Control = new wheelControl();
}

//Inits for main loop
   // we need a struct for the return variables when we call the wheel control methods

  
//  wheelControl.returnVariables returnVariablesWheel0;
//  wheelControl.returnVariables returnVariablesWheel0;
//  wheelControl.returnVariables returnVariablesWheel0;
//  wheelControl.returnVariables returnVariablesWheel0;

  float AvgerageDeltaTimeWheel0 = 10000001;
  float AvgerageDeltaTimeWheel1 = 10000001;
  float AvgerageDeltaTimeWheel2 = 10000001;
  float AvgerageDeltaTimeWheel3 = 10000001;

  unsigned long lastInterruptTimeWheel0;
  unsigned long lastInterruptTimeWheel1;
  unsigned long lastInterruptTimeWheel2;
  unsigned long lastInterruptTimeWheel3;

  int encoderPosWheel0 = 0; 
  int encoderPosWheel1 = 0;
  int encoderPosWheel2 = 0; 
  int encoderPosWheel3 = 0;

void loop() {
    /*
     We want the main loop to run at a specific frequency (normally you would set this in a realtime OS, but Arduino
     doesn't have that). So we use the system clock and keep checking to see if enough time has passed, and if so then
     execute the main loop.
  */
  if (millis() - beginingTic  > 1000 / Hz)  
  {

    beginingTic = millis();// reset the the last time this was executed.
    
// this method will get the needed data from the serial transfer software. When it gets written.

 //  serialtransfer.getDesiredInfo(newdesiredRPMWeel0, desiredAngleWheel0, newdesiredRPMWeel1, desiredAngleWheel1,new desiredRPMWeel2, desiredAngleWheel2,
 //           newdesiredRPMWeel3, desiredAngleWheel3);

/*
 * We need to diable interrupts during the time this loop reads the values that are manuipulated by the interrupt
 * software. This will keep us from getting corrupted data on the off chance that both threads are trying to
 * read/write the same variable.
 */
    noInterrupts();
    float AverageDeltaTimeWheel0 = AvgDeltaTimeWheel0;
    lastInterruptTimeWheel0 = LastIntTimeWheel0;
    countWheel0 = 0;
    encoderPosWheel0 = encposWheel0;

    float AverageDeltaTimeWheel1 = AvgDeltaTimeWheel1;
    lastInterruptTimeWheel1 = LastIntTimeWheel1;
    countWheel1 = 0;
    encoderPosWheel1 = encposWheel1;

    float AverageDeltaTimeWheel2 = AvgDeltaTimeWheel2;
    lastInterruptTimeWheel2 = LastIntTimeWheel2;
    countWheel2 = 0;
    encoderPosWheel2 = encposWheel2;

    float AverageDeltaTimeWheel3 = AvgDeltaTimeWheel3;
    lastInterruptTimeWheel3 = LastIntTimeWheel3;
    countWheel3 = 0;
    encoderPosWheel3 = encposWheel3;
    
    interrupts();
    float newdesiredRPMWheel0, newdesiredRPMWheel1, newdesiredRPMWheel2, newdesiredRPMWheel3;
    newdesiredRPMWheel0 = newdesiredRPMWheel1 = newdesiredRPMWheel2 = newdesiredRPMWheel3 = 0;
    float desiredAngleWheel0, desiredAngleWheel1, desiredAngleWheel2, desiredAngleWheel3;
    desiredAngleWheel0 = desiredAngleWheel1 = desiredAngleWheel2 = desiredAngleWheel3 = 0;
  // call the method for the wheel 0 calcuations
    struct returnVariables returnVariablesWheel0 = wheel0Control->calculate(newdesiredRPMWheel0, desiredAngleWheel0, AverageDeltaTimeWheel0,
                                  lastInterruptTimeWheel0, encoderPosWheel0);
    
  // call the method for the wheel 1 calcuations

    struct returnVariables returnVariablesWheel1 = wheel1Control->calculate(newdesiredRPMWheel1, desiredAngleWheel1, AverageDeltaTimeWheel1, 
                                  lastInterruptTimeWheel1, encoderPosWheel1);
    
      // call the method for the wheel 2 calcuations
    struct returnVariables returnVariablesWheel2 = wheel2Control->calculate(newdesiredRPMWheel2, desiredAngleWheel3, AverageDeltaTimeWheel2, 
                                  lastInterruptTimeWheel2, encoderPosWheel2);
    
      // call the method for the wheel 3 calcuations
    struct returnVariables returnVariablesWheel3 = wheel3Control->calculate(newdesiredRPMWheel3, desiredAngleWheel3, AverageDeltaTimeWheel3, 
                                  lastInterruptTimeWheel3, encoderPosWheel3);

    // Output data to pins
    
     analogWrite(vrPinWheel0, returnVariablesWheel0.speedCheck);
     digitalWrite(zfPinWheel0, returnVariablesWheel0.forwardBackward);
     analogWrite(pwmPinWheel0, returnVariablesWheel0.motorSpeed);
     digitalWrite(dirPinWheel0, returnVariablesWheel0.planMotorDirection);
 
     analogWrite(vrPinWheel1, returnVariablesWheel1.speedCheck);
     digitalWrite(zfPinWheel1, returnVariablesWheel1.forwardBackward);
     analogWrite(pwmPinWheel1, returnVariablesWheel1.motorSpeed);
     digitalWrite(dirPinWheel1, returnVariablesWheel1.planMotorDirection);

     analogWrite(vrPinWheel2, returnVariablesWheel2.speedCheck);
     digitalWrite(zfPinWheel2, returnVariablesWheel2.forwardBackward);
     analogWrite(pwmPinWheel2, returnVariablesWheel2.motorSpeed);
     digitalWrite(dirPinWheel2, returnVariablesWheel2.planMotorDirection);

     analogWrite(vrPinWheel3, returnVariablesWheel3.speedCheck);
     digitalWrite(zfPinWheel3, returnVariablesWheel3.forwardBackward);
     analogWrite(pwmPinWheel3, returnVariablesWheel3.motorSpeed);
     digitalWrite(dirPinWheel3, returnVariablesWheel3.planMotorDirection);


// output data back to other processor through serial transfer software (still to be written)
/*
  serialtransfer.sendBackInfo(returnVariablesWheel0.currentWheelRPM, returnVariablesWheel0.currentWheelAngle,
                                  returnVariablesWheel1.currentWheelRPM, returnVariablesWheel1.currentWheelAngle,
                                  returnVariablesWheel2.currentWheelRPM, returnVariablesWheel2.currentWheelAngle,
                                  returnVariablesWheel3.currentWheelRPM, returnVariablesWheel3.currentWheelAngle);  
*/
  }

}

