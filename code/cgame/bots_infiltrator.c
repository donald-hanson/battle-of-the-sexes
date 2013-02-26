#include "cg_local.h"
#include <jsapi.h>

typedef struct infiltratorState_s {
	class_t disguiseClass;
	team_t disguiseTeam;
	qboolean stealEnabled;
} infiltratorState_t;

infiltratorState_t infiltratorState;

infiltratorState_t *BOTS_Infiltrator_GetState()
{
	return &infiltratorState;
}

void BOTS_Infiltrator_Network(int clientNum)
{
	infiltratorState_t *state = BOTS_Infiltrator_GetState();
	state->stealEnabled = (qboolean)trap_Net_ReadBits(1);
	state->disguiseTeam = (team_t)trap_Net_ReadBits(3);
	state->disguiseClass = (class_t)trap_Net_ReadBits(4);
}

void BOTS_Infiltrator_ClassState(jsWrapper_t *wrapper)
{
	infiltratorState_t *state = BOTS_Infiltrator_GetState();
	wrapper->setPropertyInt(wrapper, "disguiseTeam", state->disguiseTeam);
	wrapper->setPropertyInt(wrapper, "disguiseClass", state->disguiseClass);
	wrapper->setPropertyBit(wrapper, "stealEnabled", state->stealEnabled);
}