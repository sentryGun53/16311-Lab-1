#pragma config(Sensor, S1,     ultrasonicSensor, sensorSONAR)
#pragma config(Sensor, S4,     lightSensor,    sensorLightActive)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "motion_functions.h"

#define LIGHT_SENSOR_BLACK 37 // 37 for black
#define LIGHT_SENSOR_WHITE 59 // 60 for white

#define PIVOT_SPEED 20
#define DEAD_ZONE_OFFSET 0.01
#define MAX_SPEED 25
#define MIN_SPEED 20

#define L 4.154 // inches (4.1 Adriel 2-13-15) (4.201 Adriel 2-10-15)
#define R 1.077 // inches (1.076285 Adriel 2-13-15) (1.0776 Adriel 2-10-15)
#define ARM_LENGTH 7
#define ARM_OFFSET 3


float robotX = 0.0, robotY = 0.0, robotTH = 0.0;
float old_TH = 0;
int velocityUpdateInterval = 5; // ms
float degrees_to_velocity = ((PI/180.0) / (velocityUpdateInterval/1000.0) * R); // units of inch/s
int thetaBPrev = 0, thetaCPrev = 0;
int turnsMade = 0;
float lastCornerX = 0, lastCornerY = 0;

float readings[5][15] = {
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0}
};

float calculateDistance(float x, float y) {
    return sqrt(pow(x,2) + pow(y,2));
}

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

	 	// Be careful here! Here we are assuming B is the left motor
		// Swap deltaThetaB and deltaThetaC if motors are the other way round
		float vLeft = deltaThetaB * degrees_to_velocity;
		float vRight = deltaThetaC * degrees_to_velocity;
		float v = (vLeft + vRight)/2; // cm/s
		float omega = (vRight - vLeft)/L; // radians/s

		// Runge-Kutta approximation
		float t = velocityUpdateInterval/1000.0;
		float k00 = v * cos(robotTH);
		float k01 = v * sin(robotTH);

		float k10 = v * cos(robotTH + t/2 * omega);
		float k11 = v * sin(robotTH + t/2 * omega);

		float k30 = v * cos(robotTH + t * omega);
		float k31 = v * sin(robotTH + t * omega);

		robotX += t/6 * (k00 + 4*k10 + k30);
		robotY += t/6 * (k01 + 4*k11 + k31);
		robotTH += t * omega;

		thetaBPrev = thetaBNow;
		thetaCPrev = thetaCNow;

		// Get ultrasonic reading
		int d = SensorRaw[ultrasonicSensor];
		//nxtDisplayTextLine(0, "d: %d", d);
		float dInches = d/2.54;
		float dFromCentre = dInches + ARM_LENGTH;

		float dX = robotX + ARM_OFFSET * cos(robotTH + PI) + dFromCentre * cos(robotTH + PI/2);
		float dY = robotY + ARM_OFFSET * sin(robotTH + PI) + dFromCentre * sin(robotTH + PI/2);

		// Calculate distance from last corner
		float distanceAlongCurrentStraightEdge = calculateDistance(robotX-lastCornerX, robotY-lastCornerY);
		// If rounded (down) distance is a multiple of 5, add it to readings
		int step = 2;
		int units = (int) distanceAlongCurrentStraightEdge;
		nxtDisplayTextLine(1, "distance: %d", units);
		if (units % step == 0) {
			readings[turnsMade][units/step] = d;
		}

		// Draw position on screen
		// Screen is 99 x 63
		// Rectangle of travel is 18" x 12"
		// 63 pixels for 18"
		// 3 pixels per inch
		nxtSetPixel(50 + (int)(1 * robotX), 32 + (int)(1 * robotY));
		nxtSetPixel(50 + (int)(1 * dX), 32 + (int)(1 * dY));

		/*
		//Code that plots the robot's current position and also prints it out as text
		nxtSetPixel(50 + (int)(100.0 * robotX), 32 + (int)(100.0 * robotY));
		nxtDisplayTextLine(0, "X: %f", robotX);
		nxtDisplayTextLine(1, "Y: %f", robotY);
		nxtDisplayTextLine(2, "t: %f", 57.2958 * robotTH);
		*/

		wait1Msec(velocityUpdateInterval);
	}
}



task turnCounter() {
	while (true) {
		float delta_TH = abs(robotTH - old_TH)*57.29;
		if (45 < delta_TH) {
			turnsMade++;
			lastCornerX = robotX;
			lastCornerY = robotY;
		}
		nxtDisplayTextLine(0, "turns: %d", turnsMade);
		old_TH = robotTH;
		wait10Msec(100); // Wait 1 sec
	}
}


task main()
{
	nMotorEncoder[motorB] = 0;
	nMotorEncoder[motorC] = 0;

	time1[T1] = 0;
	startTask(dead_reckoning);
	startTask(turnCounter);

	nNxtButtonTask  = 0;
	nNxtExitClicks = 2;

	while (turnsMade < 5) { //(nNxtButtonPressed != kExitButton && (time100[T1] < 50 || (abs(robotX) > 2 || abs(robotY) > 2) ) ) {
		int sensor_reading = SensorValue[lightSensor];

 		if (sensor_reading < LIGHT_SENSOR_BLACK) {
			pivot_left(PIVOT_SPEED);
		}
		else if (sensor_reading > LIGHT_SENSOR_WHITE) {
			pivot_right(PIVOT_SPEED);
		} else { // Proportional control
			// normalize sensor reading to a value between 0.0 and 1.0, where 0.0 is black and 1.0 is white
			float normalized_reading = (float)(sensor_reading - LIGHT_SENSOR_BLACK) / (float)(LIGHT_SENSOR_WHITE - LIGHT_SENSOR_BLACK);
			if (normalized_reading < 0.5 - DEAD_ZONE_OFFSET) {
				// turn right proportionally
				int off_center_amount = 0.5 - normalized_reading; // 0.0 to 0.5
				int adjust_speed = off_center_amount * 2 * (MAX_SPEED - MIN_SPEED);
				motor[motorB] = MAX_SPEED;
				motor[motorC] = (MAX_SPEED - adjust_speed);
			} else if (normalized_reading > 0.5 + DEAD_ZONE_OFFSET) {
				// turn left proportionally
				int off_center_amount = normalized_reading - 0.5; // 0.0 to 0.5
				int adjust_speed = off_center_amount * 2 * (MAX_SPEED - MIN_SPEED);
				motor[motorB] = (MAX_SPEED - adjust_speed);
				motor[motorC] = MAX_SPEED;
			}
			else { // dead zone, go straight ahead
				motor[motorB] = MAX_SPEED;
				motor[motorC] = MAX_SPEED;
			}
		}
	}

	stopTask(dead_reckoning);
		motor[motorB] = 0;
		motor[motorC] = 0;

		/*
		for (int i=0; i < 5; i++) {
			nxtDisplayTextLine(i+1, "%d: %d,%d,%d,%d,%d", i, readings[i][0], readings[i][1], readings[i][2], readings[i][3], readings[i][4]);
		}
		*/

		// Save readings to file
		TFileHandle output;
		TFileIOResult result;
		int sizeOfFloat = 8;
		short fileSize = 5*15*sizeOfFloat;
		string fileName = "readings.txt";
		Delete(fileName, result);
		OpenWrite(output, result, fileName, fileSize);
		for (int i=0; i < 5; i++) {
			for (int j=0; j < 15; j++) {
				string str = "";
				StringFormat(str, "%.1f", readings[i][j]);
				WriteString(output, result, str);
			}
			WriteString(output, result, "");
		}
		Close(output, result);

		while (nNxtButtonPressed != kExitButton) {}

}
