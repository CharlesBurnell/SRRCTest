
public class RobotWheelControl {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
    
    
     
     RobotWheel wheelOne = new RobotWheel(-45.0, 45.0, 0, 4);
     RobotWheel wheelTwo = new RobotWheel(45.0, 135.0, 1, 5);
     RobotWheel wheelThree = new RobotWheel(-135.0, -45.0, 2, 6);
     RobotWheel wheelFour = new RobotWheel(135.0, -135.0, 3, 7);
     
     double speed = 0.5;
     double thetaVel = 25;
     double thetaDot = 0;
     double wheelOneSpeed = 0;
     double wheelOneTheta = 0;
     double wheelTwoSpeed = 0;
     double wheelTwoTheta = 0;
     double wheelThreeSpeed = 0;
     double wheelThreeTheta = 0;
     double wheelFourSpeed = 0;
     double wheelFourTheta = 0;
    
     
     /* need to get current theta of each wheel   */
     
     wheelOne.wheelControl(speed, thetaVel, thetaDot, wheelOneTheta, wheelOneSpeed);
     wheelTwo.wheelControl(speed, thetaVel, thetaDot, wheelTwoTheta, wheelTwoSpeed);
     wheelThree.wheelControl(speed, thetaVel, thetaDot, wheelThreeTheta, wheelThreeSpeed);
     wheelFour.wheelControl(speed, thetaVel, thetaDot, wheelFourTheta, wheelFourSpeed);
     
     System.out.println ("speed = "+ speed + " thetaVel = " + thetaVel + " thetaDot = " + thetaDot);
     System.out.println ("Wheel one speed = " + wheelOneSpeed + " wheel one theta = " + wheelOneTheta);
     System.out.println ("Wheel two speed = " + wheelTwoSpeed + " wheel two theta = " + wheelTwoTheta);
     System.out.println ("Wheel three speed = " + wheelThreeSpeed + " wheel three theta = " + wheelThreeTheta);
     System.out.println ("Wheel four speed = " + wheelFourSpeed + " wheel four theta = " + wheelFourTheta);
     
     wheelOne.setWheelSpeed(wheelOneSpeed);
     wheelOne.setWheelTheta(wheelOneTheta);
     wheelTwo.setWheelSpeed(wheelTwoSpeed);
     wheelTwo.setWheelTheta(wheelTwoTheta);
     wheelThree.setWheelSpeed(wheelThreeSpeed);
     wheelThree.setWheelTheta(wheelThreeTheta);
     wheelFour.setWheelSpeed(wheelFourSpeed);
     wheelFour.setWheelTheta(wheelFourTheta);
     
	}

}
