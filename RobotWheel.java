
public class RobotWheel {
	
	public double wheelPsi;
	public double thetaNoSpeed;
	public int PWMForHubMotor;
	public int PWMForTurnMotor;
	private double curWheelTheta = 0;
	
	
	public RobotWheel(double psiIn, double thetaNoSpeedIn, int PWMForHubMotorIn, int PWMForTurnMotorIn) {
		wheelPsi = psiIn;
		thetaNoSpeed = thetaNoSpeedIn;
		PWMForHubMotor = PWMForHubMotorIn;
		PWMForTurnMotor = PWMForTurnMotorIn;
		
	}
	
	/** need to declare and instantiate motor controllers with PWMs at this point, plus encoders or position feedbacks so we can get current
	  * wheel theta, current wheel speed, etc.  Then we will also have methods for set wheel speed and set wheel theta.
	  * 
	  *   curWheelTheta = encoderTurnMotor.getposition();      or something like that                             
	  *   curWheelSpeed = encoderHubMotor.getspeed();     or something like that                   */
	
	public void wheelControl(double speedIn, double thetaIn, double thetaDotIn, double thetaOut, double speedOut) {
		
		double speed;
		double theta;
		double rotSpeed;
		double angleVvRot;
		double compAngle;
		double compAngleRad;
		double deltaTheta;
	
		
		double robotRadius = 0.5;  /* distance from center of robot to wheel  */
		
		rotSpeed = Math.abs(thetaDotIn) * robotRadius * 0.0174533;  /*0.0174533 = 2 * pi / 360  */
		
		
			
		
		if (Math.abs(speedIn) < 0.05) {    /* no speed  */
			if (Math.abs(rotSpeed) > 0.0001) {      /* there is rotation   */
		
				speed = rotSpeed;
				theta = thetaNoSpeed;
			}
			else {         /* no rotation and no speed   */
				speed = 0.0;
				theta = curWheelTheta;
			}
		}
		else {        /* there is speed   */ 
			if (Math.abs(rotSpeed) < .0001) {     /* no rotation   */
				theta = thetaIn;
				speed = speedIn;
			}
			else {     /* there is speed and rotation   */
				
				/* determine the angle between the velocity vector and rotation vector at wheel  */
				
				if (thetaDotIn < 0) {
					angleVvRot = wheelPsi - thetaIn - 90.0;	
				}
				else {
					angleVvRot = wheelPsi - thetaIn + 90.0;
				}
				compAngle = 180.0 - angleVvRot;  /* determine complementary angle and make sure it is +-180 */
				if (compAngle > 180.0) {
					compAngle = compAngle - 360.0;
				}
				else if (compAngle < 180.0) {
					compAngle = compAngle + 360.0;
				}
				
				compAngleRad = compAngle / 0.0174533;  /*  pi / 180  */
				
				  /* determine wheel speed   */
				speed = Math.sqrt(speedIn * speedIn + rotSpeed * rotSpeed - 2.0 * speedIn * rotSpeed * Math.cos(compAngle));
				
				   /* determine delta theta or angle or resultant vector form speed vector  */
				
				deltaTheta = Math.toDegrees(Math.asin((Math.sin(compAngleRad) * speedIn / speed)));
				theta = thetaIn + deltaTheta;
			}
		}
		
		   /* need to check in theta is still +- 180    */
		if (theta > 180.0) {
			theta = theta - 360.0;
		}
		else if (theta < 180.0) {
			theta = theta + 360.0;
		}
		  /*  minimize wheel movement   */
		if (Math.abs(curWheelTheta - theta)  > 90.0) {
			speed = speed * -1.0;
			if (theta < 0.0) {
				theta = theta + 180;
			}
			else {
				theta = theta - 180.0;
			}
		}
		
		thetaOut = theta;
		speedOut = speed;
	}
	
	public void setWheelSpeed (double speed) {
	 /** Need to set a wheel rotational speed based of the desired translational speed at that wheel and the radius of the wheel. 
	 *  Probably will need a PD controller to help with this. Something like this ---
	 *  
	 *  wheelSpeedError = speed - curWheelSpeed;
	 *  deltaWheelSpeedError = wheelSpeedError - lastDeltaWheelSpeedError;
	 *  newWheelSpeed = curWheelSpeed + (wheelSPeedError * PGain + deltaWheelSpeedError * DGain);
	 *  controllerHubMotor.setSpeed(NewWheelSpeed);
	 *  lastDeltaSpeedError = deltaWheelSpeedError;
	 *  */
	 }
	 
	 public void setWheelTheta (double theta)  {
	 /**   this method will depend on what type a motor we use, but it will be similar to setWheelSpeed() 
	 *  
	 * */ 
	 }
	 

}
