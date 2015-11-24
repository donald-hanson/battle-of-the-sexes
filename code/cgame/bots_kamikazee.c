#include "cg_local.h"

typedef struct kamikazeeState_s {
	grenadeLauncherMode_t grenadeLauncherMode;
	detpipeMode_t detpipeMode;
	int detpipeCount[2];
	int detpipeLimit;
} kamikazeeState_t;

kamikazeeState_t kamikazeeState;

kamikazeeState_t *BOTS_Kamikazee_GetState()
{
	return &kamikazeeState;
}

void BOTS_Kamikazee_Network(int clientNum)
{
	kamikazeeState_t *state = BOTS_Kamikazee_GetState();
	state->grenadeLauncherMode = (grenadeLauncherMode_t)trap_Net_ReadBits(4);
	state->detpipeMode = (detpipeMode_t)trap_Net_ReadBits(4);
	state->detpipeCount[0] = trap_Net_ReadBits(4);
	state->detpipeCount[1] = trap_Net_ReadBits(4);
	state->detpipeLimit = trap_Net_ReadBits(4);
}

void BOTS_Kamikazee_ClassState(jsWrapper_t *wrapper)
{
	kamikazeeState_t *state = BOTS_Kamikazee_GetState();

	wrapper->setPropertyInt(wrapper, "grenadeLauncherMode", (int)state->grenadeLauncherMode);
	wrapper->setPropertyInt(wrapper, "detpipeMode", (int)state->detpipeMode);
	wrapper->setPropertyArray(wrapper, "detpipeCount", state->detpipeCount, 2);
	wrapper->setPropertyInt(wrapper, "detpipeLimit", (int)state->detpipeLimit);
}