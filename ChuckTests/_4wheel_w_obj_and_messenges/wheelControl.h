#include "returnedVariables.h"
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

