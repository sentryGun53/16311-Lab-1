#pragma config(Sensor, S3,     lightSensor,    sensorLightActive)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

task main()
{
	while(true) {
 		if (SensorValue[lightSensor] < 37) { // Might wanna play with threshold values
			motor[motorB] = 15;
			motor[motorC] = -15;
		} else if (SensorValue[lightSensor] > 53) { // Might wanna play with threshold values
			motor[motorB] = -15;
			motor[motorC] = 15;
		} else { // Go straight
			motor[motorB] = 100;
			motor[motorC] = 100;
		}
	}
}