#include "cg_local.h"

typedef struct soldierState_s {
	rocketMode_t rocketMode;
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
}