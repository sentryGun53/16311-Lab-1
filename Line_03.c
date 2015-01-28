#pragma config(Sensor, S3,     lightSensor,    sensorLightActive)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "motion_functions.h"
#include "math_functions.h"

#define LIGHT_SENSOR_BLACK 36 // originally 37
#define LIGHT_SENSOR_WHITE 58 // originally 53

#define PIVOT_SPEED 20
#define DEAD_ZONE_OFFSET 0.04
#define MAX_SPEED 35
#define MIN_SPEED 25

// uncomment this line to make cool noises
//#define MELODIES

task main()
{
	while(true) {
		int sensor_reading = SensorValue[lightSensor];
 		if (sensor_reading < LIGHT_SENSOR_BLACK) {
			pivot_left(PIVOT_SPEED);
		}
		else if (sensor_reading > LIGHT_SENSOR_WHITE) {
			pivot_right(PIVOT_SPEED);
		}
		else { // proportional control
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
		float normalized_speed = (motor[motorB] + 100.0) / 200.0; // 0.0 to 1.0
		int freq = normalized_speed * 500 + 700;
#ifdef MELODIES
		playTone(freq,1);
#endif
	}
}
