#include "g_local.h"

typedef struct berzerkerState_s {
	int x;
} berzerkerState_t;

berzerkerState_t berzerkerStates[MAX_CLIENTS];

berzerkerState_t *BOTS_Berzerker_GetState(int clientNum)
{
	return &berzerkerStates[clientNum];
}

void BOTS_BerzerkerSpawn(gentity_t *player)
{
	gclient_t	*client = player->client;
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