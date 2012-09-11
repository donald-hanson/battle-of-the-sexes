#include "g_local.h"

#define BOTS_BLIND_TIME 3000

typedef struct grenadeState_s {
	int startTime;		//time the user started pressing +gren
	int lastTime;		//time the user last released -gren
	int activeGrenades;	// total number of active special grenades, if this number exceeds the limit (below) grenades thrown will be normal grenades
} grenadeState_t;

grenadeState_t grenadeStates[MAX_CLIENTS];

typedef struct grenadeInfo_s {
	grenadeType_t type;
	void (*handler)(gentity_t *player, gentity_t *grenade);
} grenadeInfo_t;

void BOTS_Grenade_Setup_Proximity(gentity_t *ent, gentity_t *grenade);
void BOTS_Grenade_Setup_Decoy(gentity_t *ent, gentity_t *grenade);
void BOTS_Grenade_Setup_Flash(gentity_t *ent, gentity_t *grenade);

grenadeInfo_t grenadeInfos[] = {
	//type,						handler
	{GRENADE_NORMAL,			NULL },
	{GRENADE_PROXIMITY,			BOTS_Grenade_Setup_Proximity },
	{GRENADE_FLASH,				BOTS_Grenade_Setup_Flash },
	{GRENADE_TELEPORT,			NULL },
	{GRENADE_FREEZE,			NULL },
	{GRENADE_DECOY,				BOTS_Grenade_Setup_Decoy },
	{GRENADE_NUM_GRENADES,		NULL },
};

void BOTS_Grenade_Flash_Think(gentity_t *ent)
{
	int i;
	gentity_t *temp;
	gentity_t *player = ent->parent;
	float blindRadius = 300.0;
	float distance = 0;
	int playerLevel = player->client->ps.persistant[PERS_LEVEL];
	if (playerLevel < 1)
		playerLevel = 1;
	blindRadius *= playerLevel;
	
	for(i=0;i<level.maxclients;i++)
	{
		temp = g_entities + i;
		if (temp && 
			temp->inuse && 
			temp->client &&
			temp->health > 0 &&
			!OnSameTeam(temp, player) &&
			BOTS_Common_Visible(temp, ent)
		)
		{
			distance = Distance(temp->r.currentOrigin, ent->r.currentOrigin);
			if (distance <= blindRadius)	// adjust based on distance from grenade?
				temp->client->ps.powerups[PW_BLIND] = level.time + BOTS_BLIND_TIME;
		}
	}

	ent->nextthink = level.time + FRAMETIME;
	ent->think = G_ExplodeMissile;
}

void BOTS_Grenade_Setup_Flash(gentity_t *ent, gentity_t *grenade)
{
	grenade->classname = "flash_grenade";
	grenade->think = BOTS_Grenade_Flash_Think;
	grenade->s.modelindex2 = GRENADE_MODEL_FLASH;
	grenade->damage = 0;
	grenade->splashDamage = 0;
}

void BOTS_Grenade_Proximity_Think(gentity_t *ent)
{
	gentity_t *player = ent->parent;
	grenadeState_t *grenadeState = &grenadeStates[player->s.clientNum];
	int i=0;
	gentity_t *temp;

	if (level.time > ent->count)
	{
		G_ExplodeMissile(ent);
		grenadeState->activeGrenades--;
		return;
	}

	ent->think = BOTS_Grenade_Proximity_Think;
	ent->nextthink = level.time + FRAMETIME;
	
	for(i=0;i<level.maxclients;i++)
	{
		temp = g_entities + i;
		if (temp && 
			temp->inuse && 
			temp->client &&
			temp->health > 0 &&
			!OnSameTeam(temp, player) &&
			BOTS_Common_Visible(temp, ent) &&
			Distance(temp->r.currentOrigin, ent->r.currentOrigin) <= 100.0
			)
		{
			ent->count = 0;
			ent->nextthink = level.time + 10;
			break;
		}
	}
}

void BOTS_Grenade_Setup_Proximity(gentity_t *ent, gentity_t *grenade)
{
	grenadeState_t *grenadeState = &grenadeStates[ent->s.clientNum];
	int modifier = ent->client->ps.persistant[PERS_LEVEL];

	if (grenadeState->activeGrenades < 8)
	{
		grenadeState->activeGrenades++;

		if (modifier < 1)
			modifier = 1;

		grenade->classname = "proxy_grenade";
		grenade->think = BOTS_Grenade_Proximity_Think;
		grenade->nextthink = level.time + FRAMETIME;
		grenade->s.loopSound = G_SoundIndex("sound/Hgrenc1b.wav");
		grenade->s.modelindex2 = GRENADE_MODEL_PROXIMITY;
		grenade->count = level.time + (30000 * modifier);
	}
}

void BOTS_Grenade_Setup_Decoy(gentity_t *ent, gentity_t *grenade)
{
	static int seed = 0x92;
	grenadeState_t *grenadeState = &grenadeStates[ent->s.clientNum];
	int playerLevel = ent->client->ps.persistant[PERS_LEVEL];

	if (grenadeState->activeGrenades < 4 + playerLevel)
	{
		grenadeState->activeGrenades++;

		grenade->classname = "decoy_grenade";
		grenade->think = BOTS_Grenade_Proximity_Think;
		grenade->nextthink = level.time + FRAMETIME;
		grenade->s.modelindex2 = Q_randomBetween(&seed, GRENADE_MODEL_SHOTGUN, GRENADE_MODEL_BFG);
		grenade->count = level.time + 25000;
	}
}

void BOTS_Grenade_Throw(gentity_t *ent, grenadeType_t grenadeType, int heldTime)
{
	gentity_t *grenade;
	vec3_t	forward, right, up;
	vec3_t	muzzle;
	grenadeState_t *grenadeState = &grenadeStates[ent->s.clientNum];
	grenadeInfo_t *grenadeInfo = &grenadeInfos[grenadeType];
	int velocity = 700;
	AngleVectors (ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );
	forward[2] += 0.2f;
	VectorNormalize( forward );

	if (heldTime < 250)
		velocity = 200;
	else
	{
		if (ent->bots_team == TEAM_RED)
		{
			if (heldTime < 1000)
				velocity = 600;
			else if (heldTime < 2000)
				velocity = 900;
			else
				velocity = 1200;
		}
		else
		{
			if (heldTime < 1000)
				velocity = 300;
			else if (heldTime < 2000)
				velocity = 600;
			else
				velocity = 800;
		}
	}

	grenade = fire_grenade (ent, muzzle, forward, velocity);
	grenade->s.modelindex2 = GRENADE_MODEL_NORMAL;

	if (grenadeInfo->handler != NULL)
		grenadeInfo->handler(ent, grenade);

	ent->client->ps.ammo[WP_GRENADE_LAUNCHER]--;
	ent->client->ps.weaponTime = 1000;
	ent->client->ps.weaponstate = WEAPON_DROPPING;
	ent->client->ps.torsoAnim = ((ent->client->ps.torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT)| TORSO_ATTACK2;
}

void BOTS_Grenade_HandleKeyPress(gentity_t *ent)
{
	grenadeState_t *grenadeState = &grenadeStates[ent->s.clientNum];
	qboolean wasDown =  (ent->client->oldbuttons & BUTTON_GRENADE) == BUTTON_GRENADE ? qtrue : qfalse;
	qboolean isDown = (ent->client->pers.cmd.buttons & BUTTON_GRENADE) == BUTTON_GRENADE ? qtrue : qfalse;

	if (isDown && !wasDown)
	{
		grenadeState->startTime = level.time;
	}
	else if (!isDown && wasDown)
	{
		if (grenadeState->lastTime + 1000 <= level.time && 
			ent->health > 0 && 
			ent->client->ps.stats[STAT_HEALTH] > 0 && 
			ent->client->ps.ammo[WP_GRENADE_LAUNCHER] > 0)
		{
			if (grenadeState->startTime > 0)
			{
				grenadeType_t grenadeType = BOTS_GetGrenadeType(ent->bots_class);
				if (ent->bots_class == CLASS_SCIENTIST && ent->client->ps.persistant[PERS_LEVEL] < 1)
					grenadeType = GRENADE_NORMAL;

				BOTS_Grenade_Throw(ent, grenadeType, level.time - grenadeState->startTime);
			}
			grenadeState->startTime = 0;
			grenadeState->lastTime = level.time;
		}
	}
}

void BOTS_Grenade_ExplodeNearByGrenades(gentity_t *ent)
{
	int i=0;
	gentity_t *temp;

	if (Q_stricmp (ent->classname, "hook")   == 0 ||
		Q_stricmp (ent->classname, "bfg")    == 0 ||
		Q_stricmp (ent->classname, "plasma") == 0)
		return;


	for(i=0;i<MAX_GENTITIES;i++)
	{
		temp = g_entities + i;
		if (temp && 
			temp->inuse && 
			temp->classname &&
			temp != ent &&
			Distance(ent->s.pos.trBase, temp->s.pos.trBase) <= 80 &&
			(Q_stricmp (temp->classname, "proxy_grenade")  == 0 || Q_stricmp (temp->classname, "decoy_grenade")  == 0))
		{
			temp->count = 0;
			temp->nextthink = level.time + 10;
		}
	}
}

qboolean BOTS_Grenade_TryToStick(gentity_t *ent, gentity_t *other, trace_t *trace)
{
	// the entity hitting other is not a proximity grenade.
	if (Q_stricmp(ent->classname, "proxy_grenade") != 0)
		return qfalse;

	// hit a player.. so no...
	if (other->client)
		return qfalse;

	// hit a door... no stickie for you!
	if (Q_stricmp (other->classname, "func_door") == 0)
		return qfalse;

	G_SetOrigin(ent, trace->endpos);
	VectorCopy(trace->plane.normal,ent->s.pos.trDelta);
	ent->s.pos.trTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;

	return qtrue;
}