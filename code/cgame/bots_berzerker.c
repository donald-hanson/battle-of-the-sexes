#include "cg_local.h"
#include <jsapi.h>

typedef struct berzerkerState_s {
	qboolean chargeActive;
	int chargeTime;
	qboolean rageActive;
	int rageTime;
	qboolean autoQuadActive;
	int autoQuadTime;
} berzerkerState_t;

berzerkerState_t berzerkerState;

berzerkerState_t *BOTS_Berzerker_GetState()
{
	return &berzerkerState;
}

void BOTS_Berzerker_Network(int clientNum)
{
	int i;
	berzerkerState_t *state = BOTS_Berzerker_GetState();

	state->chargeActive = (qboolean)trap_Net_ReadBits(1);
	state->chargeTime = trap_Net_ReadBits(8);
	state->rageActive = (qboolean)trap_Net_ReadBits(1);
	state->rageTime = trap_Net_ReadBits(8);
	state->autoQuadActive = (qboolean)trap_Net_ReadBits(1);
	state->autoQuadTime = trap_Net_ReadBits(8);
}

void BOTS_Berzerker_ClassState(jsWrapper_t *wrapper)
{
	int i;
	berzerkerState_t *state = BOTS_Berzerker_GetState();

	wrapper->setPropertyBit(wrapper, "chargeActive", state->chargeActive);
	wrapper->setPropertyInt(wrapper, "chargeTime", state->chargeTime);
	wrapper->setPropertyBit(wrapper, "rageActive", state->rageActive);
	wrapper->setPropertyInt(wrapper, "rageTime", state->rageTime);
	wrapper->setPropertyBit(wrapper, "autoQuadActive", state->autoQuadActive);
	wrapper->setPropertyInt(wrapper, "autoQuadTime", state->autoQuadTime);
}
