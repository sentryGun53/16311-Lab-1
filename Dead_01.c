/*****************************************
 * Lab 3 : Starter code
 * Written by Kaushik Viswanathan
 *****************************************/

#define L 16.43 // wheel base in cm
#define R 2.14 // wheel radius in cm

#define PI 3.14159265359

//Global variables - you will need to change some of these
float robot_X = 0.0, robot_Y = 0.0, robot_TH = 0.0;
int velocityUpdateInterval = 5; // ms
int PIDUpdateInterval = 2;
int inputB[3] = {0,0,0};
int inputC[3] = {0,0,0};

float degrees_to_velocity = ((PI/180.0) / (velocityUpdateInterval/1000.0) * R); // units of cm/s
int thetaBPrev = 0, thetaCPrev = 0;

/*****************************************
 * Complete this function so that it
 * continuously updates the robot's position
 *****************************************/
task dead_reckoning()
{
	while(1)
	{
		//
		//Fill in code for numerical integration / position estimation here
		//

		int thetaBNow = nMotorEncoder[motorB];
		int thetaCNow = nMotorEncoder[motorC];

		int deltaThetaB = thetaBNow - thetaBPrev;
		int deltaThetaC = thetaCNow - thetaCPrev;

		float vRight = deltaThetaB * degrees_to_velocity; // Be careful here! Here we are assuming B is the right motor
		float vLeft = deltaThetaC * degrees_to_velocity; // Swap deltaThetaB and deltaThetaC if motors are the other way round
		float v = (vLeft + vRight)/2; // cm/s
		float omega = (vRight - vLeft)/L;

		robot_X = robot_X + v*cos(robot_TH) * (velocityUpdateInterval/1000.0);
		robot_Y = robot_Y + v*sin(robot_TH) * (velocityUpdateInterval/1000.0);
		robot_TH = robot_TH + omega * (velocityUpdateInterval/1000.0);

		thetaBPrev = thetaBNow;
		thetaCPrev = thetaCNow;

		//Code that plots the robot's current position and also prints it out as text
		nxtSetPixel(50 + (int)(100.0 * robot_X), 32 + (int)(100.0 * robot_Y));
		nxtDisplayTextLine(0, "X: %f", robot_X);
		nxtDisplayTextLine(1, "Y: %f", robot_Y);
		nxtDisplayTextLine(2, "t: %f", 57.2958 * robot_TH);

		wait1Msec(velocityUpdateInterval);
	}
}

/*****************************************
 * Function that draws a grid on the LCD
 * for easier readout of whatever is plot
 *****************************************/
void draw_grid()
{
	for(int i = 0; i < 65; i++)
	{
		nxtSetPixel(50, i);
		int grid5 = (i - 32) % 5;
		int grid10 = (i - 32) % 10;
		if(!grid5 && grid10)
		{
			for(int j = -2; j < 3; j++)
			{
				nxtSetPixel(50 + j, i);
			}
		}
		else if(!grid10)
		{
			for(int j = -4; j < 5; j++)
			{
				nxtSetPixel(50 + j, i);
			}
		}
	}
	for(int i = 0; i < 101; i++)
	{
		nxtSetPixel(i, 32);
		int grid5 = (i - 100) % 5;
		int grid10 = (i - 100) % 10;
		if(!grid5 && grid10)
		{
			for(int j = -2; j < 3; j++)
			{
				nxtSetPixel(i, 32 + j);
			}
		}
		else if(!grid10)
		{
			for(int j = -4; j < 5; j++)
			{
				nxtSetPixel(i, 32 + j);
			}
		}
	}
}

/*****************************************
 * Function that fills in the inputB and
 * inputC arrays with values inputted
 * through motorA
 *****************************************/
void getInput()
{
	int i = 0;
	nMotorEncoder[motorA] = 0;
	nNxtButtonTask = 0;
	while(i < 3)
	{
		while(nNxtButtonPressed != kEnterButton)
		{
			inputB[i] = (int)(nMotorEncoder[motorA] / 5.0);
			if(inputB[i] > 100) {inputB[i] = 100; nMotorEncoder[motorA] = 100;}
			else if(inputB[i] < -100) {inputB[i] = -100; nMotorEncoder[motorA] = -100;}
			nxtDisplayTextLine(2 * i, "B : %d", inputB[i]);
			wait1Msec(10);
		}
		wait1Msec(300);
		while(nNxtButtonPressed != kEnterButton)
		{
			inputC[i] = (int)(nMotorEncoder[motorA] / 5.0);
			if(inputC[i] > 100) {inputC[i] = 100; nMotorEncoder[motorA] = 100;}
			else if(inputC[i] < -100) {inputC[i] = -100; nMotorEncoder[motorA] = -100;}
			nxtDisplayTextLine(2 * i + 1, "C : %d", inputC[i]);
			wait1Msec(10);
		}
		i++;
		nMotorEncoder[motorA] = 0;
		wait1Msec(300);
	}
	for(int j = 0; j < 8; j++)
		nxtDisplayClearTextLine(j);
}

/*****************************************
 * Main function - it is not necessary to
 * modify this
 *****************************************/
task main()
{
  /* Reset encoders and turn on PID control */
	nMotorEncoder[motorB] = 0;
	nMotorEncoder[motorC] = 0;
	nMotorPIDSpeedCtrl[motorB] = mtrSpeedReg;
	nMotorPIDSpeedCtrl[motorC] = mtrSpeedReg;
	nPidUpdateInterval = PIDUpdateInterval;

	//getInput();

	draw_grid();
	time1[T1] = 0;
	startTask(dead_reckoning);

	for(int i = 0; i < 3; i++)
	{
		motor[motorB] = inputB[i];
		motor[motorC] = inputC[i];

		wait1Msec(1000 * 5);
  }

  motor[motorB] = 0;
	motor[motorC] = 0;
	nNxtButtonTask  = 0;

	while(nNxtButtonPressed != kExitButton) {}
}
