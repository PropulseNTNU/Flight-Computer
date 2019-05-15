#include "../states.h"
#include "../utilities/recovery/recovery.h"
#include "../utilities/airbrakes/controll.h"
#include "../utilities/airbrakes/interpolation.h"
#include "../utilities/airbrakes/kalman.h"
#include "../../servo_interface/servo_interface.h"
#include "../../SD_interface/SD_interface.h"
#include <Arduino.h>

//initilises variables
float error = 0; //error used in controller
float riemann_sum = 0; //used in integrator, witch is used in controller
float u = 0;
float dt = 0; //time step used in integrator and kalman filter
ControlParameters parameters = { 1 , 0.01 , 1 }; //Control parameters (Kp, Ki, Kd)
unsigned long time_old = 0; // time variable for delta time

float sensor_data[2]={0,0}; //Barometer at index 0 and accelrometer (z-direction)at index 1. Utvides kanskje senere m/pitch
float estimates[2] = {0,0}; //Estimates from Kalman filter. [height, velocity]
float reference_v= 0; //reference_velovity



int airbrakes_state(double data[]) {
	return_code ret_code;
    
	//Updats dt
	float t = millis();
	dt = (float)(t- time_old);
	dt /= (float)1000; // converted to seconds
	time_old = t;
	if(dt > 0 && t > 0){
		dt = 0.03/(t/dt);
		}
	else{
		dt = 0.01;
		}

	kalman(estimates, data[1], data[2], dt, reference_v);
	
	reference_v = getReferenceVelocity(estimates[0]);
	error = estimates[1] - reference_v ;
	u = controller(&error, &parameters, &riemann_sum, dt); //updates controll signal

	// write error and controll signal too file before if statement
	if(u >= 0 && u <= 75) {
		get_servo(AIRBRAKES_SERVO)->write(u); //updates servo position
		Serial.print("c_s");
		Serial.println(u);
	}

    // This updates the ApogeeArray with current altitude
    getAltitudeStruct()->updateDataArray(getAltitudeStruct(), (double)estimates[0]); //kalman_altitude == estimates[0]
	
	// write values from both airbrakes and recovery to SD card
	if ((millis() - *getLastLog(COMMON_LASTLOG)) >= *getLogInterval(AIRBRAKES_INTERVAL)) {
		setLastLog(millis(), COMMON_LASTLOG);
		// these values may be nan during testing since the lookup table or sensors may be missing
		double abValues[4] = {data[0], estimates[0], estimates[1], u};
		write_SD(AIRBRAKES_FILE, abValues, 4);
        // writing recovery values
		write_SD(RECOVERY_FILE, getApogee()->recoveryData, RECOVERY_DATA_LEN);
	}

    // Directly checks if average altitude falls below max altitude by a margin
	if (apogeeDetected(getApogee(), data)) {
		get_servo(AIRBRAKES_SERVO)->write(0); 
		ret_code = NEXT;
	}
	else {
		ret_code = REPEAT;
	}

	return ret_code;
}
