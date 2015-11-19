#include "g_local.h"

#define CHARGE_TIME				2500
#define CHARGE_DELAY			30000
#define CHARGE_DELAY_REDUCTION	2500
#define AUTOQUAD_COOLDOWN_RED	90000
#define AUTOQUAD_COOLDOWN_BLUE	60000
#define AUTOQUAD_TIME			10000

typedef struct berzerkerState_s {
	int charge;
	int chargeCooldown;
	qboolean rageActive;
	int rageStopTime;
	qboolean autoQuadActive;
	int autoQuadStartTime;
} berzerkerState_t;

berzerkerState_t berzerkerStates[MAX_CLIENTS];

berzerkerState_t *BOTS_Berzerker_GetState(int clientNum)
{
	return &berzerkerStates[clientNum];
}

void BOTS_Berzerker_Network(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	berzerkerState_t *state = BOTS_Berzerker_GetState(clientNum);
	int i = 0;

	if (state->charge > level.time)
	{
		trap_Net_WriteBits(1, 1);
		trap_Net_WriteBits((state->charge - level.time) / 1000, 8);
	}
	else if (state->charge + state->chargeCooldown > level.time)
	{
		trap_Net_WriteBits(0, 1);
		trap_Net_WriteBits((state->charge + state->chargeCooldown - level.time) / 1000, 8);
	}
	else
	{
		trap_Net_WriteBits(0, 1);
		trap_Net_WriteBits(0, 8);
	}

	if (state->rageActive) 
	{
		trap_Net_WriteBits(1, 1);
		trap_Net_WriteBits((state->rageStopTime - level.time) / 1000, 8);
	}
	else 
	{
		trap_Net_WriteBits(0, 1);
		trap_Net_WriteBits(0, 8);
	}

	if (state->autoQuadActive) 
	{
		trap_Net_WriteBits(1, 1);
		trap_Net_WriteBits((state->autoQuadStartTime - level.time) / 1000, 8);
	}
	else 
	{
		trap_Net_WriteBits(0, 1);
		trap_Net_WriteBits(0, 8);
	}
}

void BOTS_Berzerker_ResetAutoQuadStartTime(gentity_t *player) 
{
	berzerkerState_t *state = BOTS_Berzerker_GetState(player->s.clientNum);
	if (player->bots_team == TEAM_RED)
		state->autoQuadStartTime = level.time + AUTOQUAD_COOLDOWN_RED;
	else
		state->autoQuadStartTime = level.time + AUTOQUAD_COOLDOWN_BLUE;
}

void BOTS_BerzerkerSpawn(gentity_t *player)
{
	gclient_t	*client = player->client;
	berzerkerState_t *state = BOTS_Berzerker_GetState(player->s.clientNum);
	int pLevel = client->ps.persistant[PERS_LEVEL];
	float multiplier = 1.0f;
	if (pLevel > 0) 
	{
		multiplier += 0.05f * pLevel;

		client->ps.stats[STAT_MAX_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * multiplier;
		client->ps.stats[STAT_MAX_ARMOR] = client->ps.stats[STAT_MAX_ARMOR] * multiplier;

		player->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];
		client->ps.stats[STAT_ARMOR] = client->ps.stats[STAT_MAX_ARMOR];
	}

	if (pLevel < 2) 
	{
		state->autoQuadActive = qtrue;
	}

	BOTS_Berzerker_ResetAutoQuadStartTime(player);
}

void BOTS_Berzerker_ModifyDamage(gentity_t *target, gentity_t *inflictor, gentity_t *attacker, int *damage, int mod)
{
	int finalDamage = *damage;
	int currentLevel = attacker->client->ps.persistant[PERS_LEVEL];
	if (mod == MOD_GAUNTLET) 
	{
		finalDamage *= 2;
		finalDamage *= 1.0f + (0.25f * currentLevel);
	}
	*damage = finalDamage;
}

void BOTS_BerzerkerCommand_Charge(int clientNum)
{
	vec3_t distance;
	int activeTime = 0;
	int cooldownTime = 0;
	gentity_t *ent = g_entities + clientNum;
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];
	berzerkerState_t *state = BOTS_Berzerker_GetState(clientNum);

	if (state->charge > level.time)
	{
		trap_SendServerCommand(clientNum, "print \"Charge currently active.\n\"");
	}
	else if (state->charge + state->chargeCooldown > level.time)
	{
		trap_SendServerCommand(clientNum, "print \"Charge currently on cooldown.\n\"");
	}
	else if (ent->client->ps.powerups[PW_REDFLAG] || ent->client->ps.powerups[PW_BLUEFLAG]) 
	{
		trap_SendServerCommand(clientNum, "print \"You cannot charge with the flag.\n\"");
	}
	else
	{
		cooldownTime = CHARGE_DELAY - (CHARGE_DELAY_REDUCTION * pLevel);

		if (pLevel == 0)
			pLevel = 1;

		activeTime = (CHARGE_TIME * pLevel);
		
		ent->client->ps.powerups[PW_HASTE] = state->charge = level.time + activeTime;
		state->chargeCooldown = cooldownTime;

		trap_SendServerCommand(clientNum, "print \"Charge enabled.\n\"");
	}
}

void BOTS_BerzerkerCommand_Rage(int clientNum)
{
	vec3_t distance;
	int activeTime = 0;
	int cooldownTime = 0;
	gentity_t *ent = g_entities + clientNum;
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];
	berzerkerState_t *state = BOTS_Berzerker_GetState(clientNum);

	if (state->rageStopTime > level.time)
	{
		trap_SendServerCommand(clientNum, "print \"Rage currently active.\n\"");
	}
	else if (ent->client->ps.powerups[PW_REDFLAG] || ent->client->ps.powerups[PW_BLUEFLAG])
	{
		trap_SendServerCommand(clientNum, "print \"You cannot use rage with the flag.\n\"");
	}
	else
	{
		if (ent->bots_team == TEAM_RED && ent->health >= 75) 
		{
			trap_SendServerCommand(clientNum, "print \"You have too much health to use rage.\n\"");
		} 
		else if (ent->health >= 100) 
		{
			trap_SendServerCommand(clientNum, "print \"You have too much health to use rage.\n\"");
		}
		else 
		{
			trap_SendServerCommand(clientNum, "print \"Rage enabled.\n\"");
			state->rageActive = qtrue;
			state->rageStopTime = level.time + (200 * ent->health);
		}
	}
}

void BOTS_BerzerkerCommand_TogQuad(int clientNum) 
{
	gentity_t *ent = g_entities + clientNum;
	berzerkerState_t *state = BOTS_Berzerker_GetState(clientNum);
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];

	if (pLevel < 2) 
	{
		trap_SendServerCommand(clientNum, "print \"You must be level 2 to toggle quad.\n\"");
	}
	else 
	{
		state->autoQuadActive = !state->autoQuadActive;

		if (state->autoQuadActive) 
		{
			BOTS_Berzerker_ResetAutoQuadStartTime(ent);
		}
		else 
		{
			state->autoQuadStartTime = 0;
		}
	}
}

void BOTS_Berzerker_Think(gentity_t *player, usercmd_t *ucmd)
{
	berzerkerState_t *state = BOTS_Berzerker_GetState(player->s.clientNum);
	if (state->rageActive) 
	{
		if (state->rageStopTime > level.time)
		{
			player->takedamage = qfalse;
		}
		else
		{
			state->rageActive = qfalse;
			player->takedamage = qtrue;
			G_Damage(player, player, player, NULL, NULL, 100000, 0, MOD_SUICIDE);
		}
	}

	if (state->autoQuadActive)
	{
		if (state->autoQuadStartTime <= level.time)
		{
			if (player->client->ps.powerups[PW_QUAD])
				player->client->ps.powerups[PW_QUAD] += AUTOQUAD_TIME;
			else
				player->client->ps.powerups[PW_QUAD] = level.time + AUTOQUAD_TIME;

			BOTS_Berzerker_ResetAutoQuadStartTime(player);
		}
	}
}