#include "cg_local.h"

typedef struct captainState_s {
	bfgMode_t bfgMode;
} captainState_t;

captainState_t captainState;

captainState_t *BOTS_Captain_GetState()
{
	return &captainState;
}

void BOTS_Captain_Network(int clientNum)
{
	captainState_t *state = BOTS_Captain_GetState();
	state->bfgMode = (bfgMode_t)trap_Net_ReadBits(4);
}

void BOTS_Captain_ClassState(jsWrapper_t *wrapper)
{
	captainState_t *state = BOTS_Captain_GetState();

	wrapper->setPropertyInt(wrapper, "bfgMode", (int)state->bfgMode);
}