#include "g_local.h"

void BOTS_Scientist_DropTech(int clientNum, qboolean launch)
{
	BOTS_Common_DropKey(clientNum, launch, qtrue);
}

void BOTS_ScientistSpawn(gentity_t *ent)
{
	if (BOTS_GetTeamTechKey(ent->bots_team) == NULL)
		ent->client->ps.stats[STAT_KEY] = ent->bots_team == TEAM_RED ? KEY_RED_TECH : KEY_BLUE_TECH;
}

//scientist is the killed entity
void BOTS_ScientistDeath(gentity_t *killed, gentity_t *killedBy, gentity_t *killer, int damage, int meansOfDeath)
{
	//award 1 point to the scientist's enemy team even if the death was self inflicted
	BOTS_AddTechPoints(killed->bots_team == TEAM_RED ? TEAM_BLUE : TEAM_RED, 1);
	if (BOTS_GetTeamTechKey(killed->bots_team) == NULL)
		BOTS_Scientist_DropTech(killed->s.clientNum, qfalse);
}

//scientist is the killer entity
void BOTS_ScientistKiller(gentity_t *killer, gentity_t *killedBy, gentity_t *killed, int damage, int meansOfDeath)
{
	//award one point to the scientist's team for killing an enemy
	if (killer->bots_team != killed->bots_team)
		BOTS_AddTechPoints(killer->bots_team, 1);
}

void BOTS_ScientistCommand_DropTech(int clientNum)
{
	gentity_t*  ent = g_entities + clientNum;

	if (ent->client->ps.stats[STAT_KEY] == KEY_NONE)
		BOTS_Print(clientNum, "You're not carrying the tech key!");
	else
		BOTS_Scientist_DropTech(clientNum, qtrue);
}