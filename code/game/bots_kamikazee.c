#include "g_local.h"

typedef struct kamikazeeState_s {
	grenadeLauncherMode_t grenadeLauncherMode;
	detpipeMode_t detpipeMode;
	int detpipeCount[2];
} kamikazeeState_t;

kamikazeeState_t kamikazeeStates[MAX_CLIENTS];

kamikazeeState_t *BOTS_Kamikazee_GetState(int clientNum)
{
	return &kamikazeeStates[clientNum];
}

void BOTS_Kamikazee_Network(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	float distance = 0.0f;
	float maxDistance = 0.0f;
	kamikazeeState_t *state = BOTS_Kamikazee_GetState(clientNum);
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];

	trap_Net_WriteBits((int)state->grenadeLauncherMode, 4);
	trap_Net_WriteBits((int)state->detpipeMode, 4);
	trap_Net_WriteBits((int)state->detpipeCount[0], 4); // number of pipes used in trap #1
	trap_Net_WriteBits((int)state->detpipeCount[1], 4);	// number of pipes used in trap #2
	trap_Net_WriteBits(4+ pLevel, 4);					// number of det pipes allowed
}

/*
	commands
		autorewire
		suicide
		settrap (1|2)
		dettrap (1|2)
		detnuke
*/

void BOTS_Kamikazee_SetGrenadeLauncherMode(int clientNum, grenadeLauncherMode_t grenadeLauncherMode)
{
	kamikazeeState_t *state = BOTS_Kamikazee_GetState(clientNum);
	if (state->grenadeLauncherMode == grenadeLauncherMode)
		state->grenadeLauncherMode = GRENADE_LAUNCHER_NORMAL;
	else
		state->grenadeLauncherMode = grenadeLauncherMode;
}

void BOTS_Kamikazee_SetTrap(int clientNum, detpipeMode_t detpipeMode)
{
	kamikazeeState_t *state = BOTS_Kamikazee_GetState(clientNum);
	state->detpipeMode = detpipeMode;
}

void BOTS_KamikazeeCommand_LongRange(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];
	if (pLevel < 2)
	{
		BOTS_Print(clientNum, "You must be level 2 to launch long range grenades.");
		return;
	}

	BOTS_Kamikazee_SetGrenadeLauncherMode(clientNum, GRENADE_LAUNCHER_LONGRANGE);
}

void BOTS_KamikazeeCommand_Cluster(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];
	if (pLevel < 2)
	{
		BOTS_Print(clientNum, "You must be level 2 to launch cluster grenades.");
		return;
	}

	BOTS_Kamikazee_SetGrenadeLauncherMode(clientNum, GRENADE_LAUNCHER_CLUSTER);
}

void BOTS_KamikazeeCommand_Sticky(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];
	if (pLevel < 3)
	{
		BOTS_Print(clientNum, "You must be level 3 to launch sticky grenades.");
		return;
	}

	BOTS_Kamikazee_SetGrenadeLauncherMode(clientNum, GRENADE_LAUNCHER_STICKY);
}

void BOTS_KamikazeeCommand_Nuke(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];
	if (pLevel < 3)
	{
		BOTS_Print(clientNum, "You must be level 3 to launch a nuke.");
		return;
	}

	BOTS_Kamikazee_SetGrenadeLauncherMode(clientNum, GRENADE_LAUNCHER_NUKE);
}

void BOTS_KamikazeeCommand_DetPipes(int clientNum)
{
	BOTS_Kamikazee_SetGrenadeLauncherMode(clientNum, GRENADE_LAUNCHER_DETPIPES);
}

void BOTS_Kamikazee_DetonateTrap(int clientNum, int trapNumber)
{
	char *trapName;
	int countIndex;
	gentity_t*	grenade = NULL;
	kamikazeeState_t *state = BOTS_Kamikazee_GetState(clientNum);

	if (trapNumber == 1) 
	{
		trapName = "dettrap1";
		countIndex = 0;
	}
	else if (trapNumber == 2)
	{
		trapName = "dettrap2";
		countIndex = 1;
	}

	while ((grenade = G_Find(grenade, FOFS(classname), trapName)) != NULL)
	{
		if (grenade->r.ownerNum == clientNum)
		{
			grenade->nextthink = level.time + 5;
		}
	}

	state->detpipeCount[countIndex] = 0;
}

void BOTS_KamikazeeCommand_DetTrap(int clientNum)
{
	if (trap_Argc() == 2)
	{
		char cmd[MAX_TOKEN_CHARS];
		trap_Argv(1, cmd, sizeof(cmd));
		int trapNumber = atoi(cmd);
		if (trapNumber < 1 || trapNumber > 2) 
		{
			BOTS_Print(clientNum, "Invalid trap number");
		}
		else 
		{
			BOTS_Kamikazee_DetonateTrap(clientNum, trapNumber);
		}
	}
	else
	{
		char cmd[MAX_TOKEN_CHARS];
		trap_Argv(0, cmd, sizeof(cmd));
		BOTS_Print(clientNum, va("usage: %s trap", cmd));
	}
}

void BOTS_KamikazeeCommand_SetTrap(int clientNum)
{
	if (trap_Argc() == 2)
	{
		char cmd[MAX_TOKEN_CHARS];
		trap_Argv(1, cmd, sizeof(cmd));
		int trapNumber = atoi(cmd);
		if (trapNumber == 1)
		{
			BOTS_Kamikazee_SetTrap(clientNum, DETPIPE_TRAP1);
		}
		else if (trapNumber == 2)
		{
			BOTS_Kamikazee_SetTrap(clientNum, DETPIPE_TRAP2);
		}
	}
	else
	{
		BOTS_Kamikazee_SetTrap(clientNum, DETPIPE_AUTO);
	}
}

void BOTS_GrenadeLauncher_FireDetPipes(gentity_t *ent) 
{
	gentity_t *m;
	vec3_t	forward, right, up;
	vec3_t	muzzle;
	kamikazeeState_t *state = BOTS_Kamikazee_GetState(ent->s.clientNum);
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];

	// set aiming directions
	AngleVectors(ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin(ent, ent->client->oldOrigin, forward, right, up, muzzle);

	m = fire_grenade(ent, muzzle, forward, 700);

	if (state->detpipeCount[0] + state->detpipeCount[1] >= 4 + pLevel)
		return;

	m->nextthink = 0;
	m->s.modelindex2 = GRENADE_MODEL_DETPIPE;
	m->damage *= 0.75f;

	if ((state->detpipeMode == DETPIPE_AUTO && state->detpipeCount[0] < 4) || state->detpipeMode == DETPIPE_TRAP1)
	{
		m->classname = "dettrap1";
		state->detpipeCount[0]++;
	}
	else if ((state->detpipeMode == DETPIPE_AUTO && state->detpipeCount[1] < pLevel) || state->detpipeMode == DETPIPE_TRAP2)
	{
		m->classname = "dettrap2";
		state->detpipeCount[1]++;
	}
}

void BOTS_GrenadeLauncher_FireCluster(gentity_t *ent)
{

}

void BOTS_GrenadeLauncher_FireLongRange(gentity_t *ent)
{

}

void BOTS_GrenadeLauncher_FireSticky(gentity_t *ent)
{

}

void BOTS_GrenadeLauncher_FireNuke(gentity_t *ent)
{
	gentity_t *m;
	vec3_t	forward, right, up;
	vec3_t	muzzle;
	int ammo;

	// set aiming directions
	AngleVectors(ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin(ent, ent->client->oldOrigin, forward, right, up, muzzle);

	m = fire_grenade(ent, muzzle, forward, 700);

	ammo = ent->client->ps.ammo[WP_GRENADE_LAUNCHER];

	m->classname = "nuke";
	m->methodOfDeath = MOD_NUKE;
	m->splashMethodOfDeath = MOD_NUKE;
	m->damage = ammo * 25;
	m->splashDamage = ammo * 25;
	m->splashRadius = ammo * 100;

	m->s.loopSound = G_SoundIndex("sound/siren.wav");
	m->s.modelindex2 = GRENADE_MODEL_NUKE;
	m->count = level.time + 10000;
	m->nextthink = level.time + 10000;

	ent->client->ps.ammo[WP_GRENADE_LAUNCHER] = 0;

	BOTS_Kamikazee_SetGrenadeLauncherMode(ent->s.clientNum, GRENADE_LAUNCHER_NORMAL);
}

typedef struct grenadeLauncherModeInfo_s {
	grenadeLauncherMode_t grenadeLauncherMode;
	void(*handler)(gentity_t *ent);
} grenadeLauncherModeInfo_t;

grenadeLauncherModeInfo_t grenadeLauncherModeInfos[] = {
	{ GRENADE_LAUNCHER_NORMAL,					NULL },
	{ GRENADE_LAUNCHER_DETPIPES,				BOTS_GrenadeLauncher_FireDetPipes },
	{ GRENADE_LAUNCHER_CLUSTER,					BOTS_GrenadeLauncher_FireCluster },
	{ GRENADE_LAUNCHER_LONGRANGE,				BOTS_GrenadeLauncher_FireLongRange },
	{ GRENADE_LAUNCHER_STICKY,					BOTS_GrenadeLauncher_FireSticky },
	{ GRENADE_LAUNCHER_NUKE,					BOTS_GrenadeLauncher_FireNuke },
	{ GRENADE_LAUNCHER_NUM_GRENADELAUNCHERMODES,NULL },
};

qboolean BOTS_Kamikazee_FireWeapon(gentity_t *ent)
{
	kamikazeeState_t *state = BOTS_Kamikazee_GetState(ent->s.clientNum);
	grenadeLauncherModeInfo_t *info = &grenadeLauncherModeInfos[state->grenadeLauncherMode];
	if (ent->s.weapon == WP_GRENADE_LAUNCHER && info->handler)
	{
		info->handler(ent);
		return qtrue;
	}
	return qfalse;
}