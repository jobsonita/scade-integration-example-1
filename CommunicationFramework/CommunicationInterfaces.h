#ifndef COMMUNICATIONINTERFACES_H
#define COMMUNICATIONINTERFACES_H

#include "kcg_types.h"
#include "kcg_consts.h"

#define TEAM1 1
#define TEAM2 2

typedef struct {
    kcg_bool SignalFromTeam2;
} TEAM1_INPUT_INTERFACE;

typedef struct {
    kcg_bool SignalFromTeam1;
} TEAM2_INPUT_INTERFACE;

typedef union {
	TEAM1_INPUT_INTERFACE team1_input_interface;
	TEAM2_INPUT_INTERFACE team2_input_interface;
} INPUT_INTERFACE;

typedef struct {
	int to, from;
    INPUT_INTERFACE input_interface;
} FRAMEWORK_MESSAGE;

#define MSG_SIZE 2048

#endif