#include "g_local.h"

typedef struct soldierState_s {
	rocketMode_t rocketMode;
} soldierState_t;

soldierState_t soldierStates[MAX_CLIENTS];

soldierState_t *BOTS_Soldier_GetState(int clientNum)
{
	return &soldierStates[clientNum];
}

void BOTS_Rocket_FireRapid(gentity_t *ent)
{
	gentity_t *m;
	vec3_t	forward, right, up;
	vec3_t	muzzle;
	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );
	m = fire_rocket(ent, muzzle, forward, 2000);
	m->damage *= 0.60;
}

void BOTS_Rocket_FireSplit(gentity_t *ent, int horizontal, int vertical)
{
	gentity_t *m1, *m2, *m3;
	vec3_t	forward, right, up, forward2, forward3;
	vec3_t	muzzle;
	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );
	m1 = fire_rocket(ent, muzzle, forward, 900);

	VectorScale(forward, 1000, forward2);
	VectorMA(forward2, horizontal,	right,	forward2);
	VectorMA(forward2, vertical,	up,		forward2);
	m2 = fire_rocket (ent, muzzle, forward2, 900);

	VectorScale(forward, 1000, forward3);
	VectorMA(forward3,  -1*horizontal,	right,	forward3);
	VectorMA(forward3, vertical,		up,		forward3);
	m3 = fire_rocket (ent, muzzle, forward3, 900);
}

void BOTS_Rocket_FireSplit1(gentity_t *ent)
{
	BOTS_Rocket_FireSplit(ent, 60, -60);
}

void BOTS_Rocket_FireSplit2(gentity_t *ent)
{
	BOTS_Rocket_FireSplit(ent, 70, -20);
}

void BOTS_Rocket_FireSplit3(gentity_t *ent)
{
	BOTS_Rocket_FireSplit(ent, 95, -20);
}

typedef struct rocketModeInfo_s {
	rocketMode_t rocketMode;
	void (*handler)(gentity_t *ent);
} rocketModeInfo_t;

rocketModeInfo_t rocketModeInfos[] ={
	{ROCKET_NORMAL,				NULL},
	{ROCKET_RAPID,				BOTS_Rocket_FireRapid},
	{ROCKET_TAG,				NULL},
	{ROCKET_SPLIT1,				BOTS_Rocket_FireSplit1},
	{ROCKET_SPLIT2,				BOTS_Rocket_FireSplit2},
	{ROCKET_SPLIT3,				BOTS_Rocket_FireSplit3},
	{ROCKET_GUIDED,				NULL},
	{ROCKET_NUM_ROCKETMODES,	NULL},
};

qboolean BOTS_Soldier_FireWeapon(gentity_t *ent)
{
	soldierState_t *state = BOTS_Soldier_GetState(ent->s.clientNum);
	rocketModeInfo_t *info = &rocketModeInfos[state->rocketMode];
	if (ent->s.weapon == WP_ROCKET_LAUNCHER && info->handler)
	{
		info->handler(ent);
		return qtrue;
	}
	return qfalse;
}

void BOTS_Solder_SetRocketMode(int clientNum, rocketMode_t rocketMode)
{
	soldierState_t *state = BOTS_Soldier_GetState(clientNum);
	if (state->rocketMode == rocketMode)
		state->rocketMode = ROCKET_NORMAL;
	else
		state->rocketMode = rocketMode;
}

void BOTS_SoldierCommand_Rapid(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_RAPID);
}

void BOTS_SoldierCommand_Guide(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_GUIDED);
}

void BOTS_SoldierCommand_Tag(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_TAG);
}

void BOTS_SoldierCommand_Split1(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_SPLIT1);
}

void BOTS_SoldierCommand_Split2(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_SPLIT2);
}

void BOTS_SoldierCommand_Split3(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_SPLIT3);
}