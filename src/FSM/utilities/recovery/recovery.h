#pragma once
#ifndef RECOVERY_H
#define RECOVERY_H

#include <Servo.h>
#include "apogee_detect.h"

#define DrogueServoPin 2
#define MainServoPin 3
#define MainChuteALT 457

struct ParachuteStateStruct {
    bool drogueDeployed = false; //type uint8_t = 1 byte
    bool mainDeployed = false;
    bool mainStopped = false;
   // (Constructor Option) ParachuteStateStruct() : drogueDeployed(false), mainDeployed(false), mainStopped(false) {}
};

typedef struct ParachuteStateStruct Parachute;

/* Alternative to struct -> array
 enum deployed_parachutes {DROGUE, MAIN_DEPLOY, MAIN_STOP, NUM_TYPES};
 bool parachute_state[NUM_TYPES]; //to access bool variables, then instead of struct pointer we can use array pointer.
 */

ApogeeArray* getApogee();
ParachuteStateStruct* getParachute();

#endif /* recovery_h */
