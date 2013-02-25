#include "cg_local.h"
#include <jsapi.h>

#define MAX_LASERS 3

typedef struct laserState_s {
	qboolean active;	// has the laser base been placed in the game world?
	qboolean on;		// is the laser currently on?
	float distance;		// distance to the laser
} laserState_t;

typedef struct bodyguardState_s {
	laserState_t lasers[MAX_LASERS];
	qboolean protect;
	qboolean decoyActive;
	int decoyTime;
} bodyguardState_t;

bodyguardState_t bodyguardState;

bodyguardState_t *BOTS_Bodyguard_GetState()
{
	return &bodyguardState;
}

void BOTS_Bodyguard_Network(int clientNum)
{
	int i;
	bodyguardState_t *state = BOTS_Bodyguard_GetState();
	qboolean decoyActive = qfalse;
	int decoyTime = 0;

	state->decoyActive = (qboolean)trap_Net_ReadBits(1);
	state->decoyTime = trap_Net_ReadBits(8);
	state->protect = (qboolean)trap_Net_ReadBits(1);
	for (i=0;i<MAX_LASERS;i++)
	{
		laserState_t *laser = &state->lasers[i];
		if (trap_Net_ReadBits(1))
		{
			laser->active = qtrue;
			laser->on = (qboolean)trap_Net_ReadBits(1);
			laser->distance = trap_Net_ReadFloat();
		}
		else
		{
			laser->active = qfalse;
			laser->on = qfalse;
			laser->distance = 0;
		}
	}
}

void BOTS_Bodyguard_ClassState(jsWrapper_t *wrapper)
{
	int i;
	jsWrapper_t *laserWrappers[MAX_LASERS];
	jsWrapper_t *laserWrapper;
	laserState_t *laserState;
	bodyguardState_t *state = BOTS_Bodyguard_GetState();

	for (i=0;i<MAX_LASERS;i++)
	{
		laserState  = &state->lasers[i];
		laserWrappers[i] = laserWrapper = wrapper->newObject(wrapper);
		laserWrapper->setPropertyBit(laserWrapper, "active", laserState->active);
		laserWrapper->setPropertyBit(laserWrapper, "on", laserState->on);
		laserWrapper->setPropertyFloat(laserWrapper, "distance", laserState->distance);
	}
	wrapper->addObjects(wrapper, "lasers", laserWrappers, MAX_LASERS);

	wrapper->setPropertyBit(wrapper, "protect", state->protect);
	wrapper->setPropertyBit(wrapper, "decoyActive", state->decoyActive);
	wrapper->setPropertyInt(wrapper, "decoyTime", state->decoyTime);
}