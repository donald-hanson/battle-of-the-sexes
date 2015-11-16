#include "cg_local.h"

typedef struct soldierState_s {
	rocketMode_t rocketMode;
	float distance;
	float maxDistance;
	qboolean conquerActive;
} soldierState_t;

soldierState_t soldierState;

soldierState_t *BOTS_Soldier_GetState()
{
	return &soldierState;
}

void BOTS_Soldier_Network(int clientNum)
{
	soldierState_t *state = BOTS_Soldier_GetState();
	state->rocketMode = (rocketMode_t)trap_Net_ReadBits(4);
	state->conquerActive = (qboolean)trap_Net_ReadBits(1);
	if(state->conquerActive)
	{
		state->distance = trap_Net_ReadFloat();
		state->maxDistance = trap_Net_ReadFloat();
	}
	else
	{
		state->distance = 0.0f;
		state->maxDistance = 0.0f;
	}
}

void BOTS_Soldier_ClassState(jsWrapper_t *wrapper)
{
	soldierState_t *state = BOTS_Soldier_GetState();

	wrapper->setPropertyInt(wrapper, "rocketMode", (int)state->rocketMode);
	wrapper->setPropertyBit(wrapper, "conquerActive", state->conquerActive);
	wrapper->setPropertyFloat(wrapper, "distance", state->distance);
	wrapper->setPropertyFloat(wrapper, "maxDistance", state->maxDistance);
}