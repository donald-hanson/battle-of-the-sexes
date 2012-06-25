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

void BOTS_Rocket_GuidedThink2(gentity_t *missile)
{
	vec3_t forward, right, up; 
	vec3_t muzzle;
	float dist;
	gentity_t *player = missile->parent;

	// If our owner can't be found, just return
	if (!player)
		return;

	// Get our forward, right, up vector from the view angle of the player
	AngleVectors (player->client->ps.viewangles, forward, right, up);

	// Calculate the player's eyes origin, and store this origin in muzzle
	CalcMuzzlePoint ( player, forward, right, up, muzzle );
 
	// Tells the engine that our movement starts at the current missile's origin
	VectorCopy (missile->r.currentOrigin, missile->s.pos.trBase );

	// Trajectory type setup (linear move - fly)
	missile->s.pos.trType = TR_LINEAR;
	missile->s.pos.trTime = level.time - 50;

	// Get the dir vector between the player's point of view and the rocket
	// and store it into muzzle again
	VectorSubtract (muzzle, missile->r.currentOrigin, muzzle);
	
	// Add some range to our "line" so we can go behind blocks
	// We could have used a trace function here, but the rocket would
	// have come back if player was aiming on a block while the rocket is behind it
	// as the trace stops on solid blocks
	dist = VectorLength (muzzle) + 400;	 //give the range of our muzzle vector + 400 units
	VectorScale (forward, dist, forward);

	// line straight forward
	VectorAdd (forward, muzzle, muzzle);

	// Normalize the vector so it's 1 unit long, but keep its direction
	VectorNormalize (muzzle);

	// Slow the rocket down a bit, so we can handle it
	VectorScale (muzzle, 300, forward);

	// Set the rockets's velocity so it'll move toward our new direction
	VectorCopy (forward, missile->s.pos.trDelta);

	// Change the rocket's angle so it'll point toward the new direction
	vectoangles (muzzle, missile->s.angles);

	// This should "save net bandwidth" =D
	SnapVector( missile->s.pos.trDelta );

	// Call this function in 0,1 s
	missile->nextthink = level.time + FRAMETIME/10; 
}

void BOTS_Rocket_FireGuided2(gentity_t *ent)
{
	gentity_t *m;
	vec3_t	forward, right, up;
	vec3_t	muzzle;
	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );
	m = fire_rocket(ent, muzzle, forward, 800);
	m->think = BOTS_Rocket_GuidedThink2;
	m->nextthink = level.time + FRAMETIME/10;
}

void BOTS_Rocket_GuidedThink1(gentity_t *self)
{
	gentity_t*	player;
	vec3_t		forward;
	int			speed;

	if (!self->parent)
		return;

	player = self->parent;
	if (!player->client)
		return;

	// get the clients view direction
	AngleVectors (player->client->ps.viewangles, forward, NULL, NULL);
	VectorNormalize(forward);

	// change the direction of the rocket
	VectorAdd(forward, self->movedir, forward);
	VectorNormalize(forward);
	VectorCopy(forward, self->movedir);
	vectoangles(forward, self->s.angles);

	// set the velocity
	speed = VectorLength(self->s.pos.trDelta);
	VectorScale(forward, speed, self->s.pos.trDelta);

	self->nextthink = level.time + FRAMETIME/10;
	self->think = BOTS_Rocket_GuidedThink1;
}

void BOTS_Rocket_FireGuided1(gentity_t *ent)
{
	gentity_t *m;
	vec3_t	forward, right, up;
	vec3_t	muzzle;
	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );
	m = fire_rocket(ent, muzzle, forward, 300);
	m->think = BOTS_Rocket_GuidedThink1;
	m->nextthink = level.time + FRAMETIME/10;
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
	{ROCKET_GUIDED1,			BOTS_Rocket_FireGuided1},
	{ROCKET_GUIDED2,			BOTS_Rocket_FireGuided2},
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

void BOTS_SoldierCommand_Guide1(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_GUIDED1);
}

void BOTS_SoldierCommand_Guide2(int clientNum)
{
	BOTS_Solder_SetRocketMode(clientNum, ROCKET_GUIDED2);
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