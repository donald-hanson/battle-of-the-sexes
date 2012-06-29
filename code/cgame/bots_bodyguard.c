#include "cg_local.h"

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
		}
	}
}