#include "g_local.h"

typedef struct nurseState_s {
	qboolean poison;
} nurseState_t;

nurseState_t nurseStates[MAX_CLIENTS];

nurseState_t *BOTS_Nurse_GetState(int clientNum)
{
	return &nurseStates[clientNum];
}

void BOTS_NurseSpawn(gentity_t *player)
{
	int pLevel = player->client->ps.persistant[PERS_LEVEL];

	player->client->ps.maxammo[WP_BFG] = 50 * (pLevel + 1);
}

// return qtrue and the player will not pickup the health
qboolean BOTS_Nurse_PoisonHealth(gentity_t *health, gentity_t *player)
{
	gentity_t *otherNurse;
	nurseState_t *state;

	if (player->bots_class == CLASS_NURSE)
	{
		if (health->s.powerups & (1 <<PW_POISON))
		{
			otherNurse = g_entities + health->s.otherEntityNum;
			if (OnSameTeam(player, otherNurse))
				return qtrue; 	// health is already poisoned by our teammate, return qtrue so we don't consume it
			else
			{
				// health is poisoned by an enemy
				// clear the poison and return false so we can pick it up
				health->s.powerups &= ~(1 << PW_POISON);
				health->s.otherEntityNum = -1;
				BOTS_Print(player->s.clientNum, "Cleared poison");
				return qfalse;
			}
		}
		else
		{
			// health is not poisoned. do we want to poison it?
			state = BOTS_Nurse_GetState(player->s.clientNum);			
			if (state->poison)
			{
				health->s.powerups |= (1 << PW_POISON);
				health->s.otherEntityNum = player->s.clientNum;
				BOTS_Print(player->s.clientNum, "Applied poison");
				return qtrue; // health is now poisoned, return qtrue so we don't consume it
			}
		}
	}

	return qfalse; // let them try to pick it up
}

void BOTS_NurseCommand_Poison(int clientNum)
{
	nurseState_t *state = BOTS_Nurse_GetState(clientNum);
	state->poison = state->poison ? qfalse : qtrue;

	if (state->poison)
		BOTS_Print(clientNum, "Poison Enabled");
	else
		BOTS_Print(clientNum, "Poison Disabled");
}


void BOTS_NurseCommand_HealRadius(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	gentity_t *teamMember;
	int i = 0;
	int pLevel = ent->client->ps.persistant[PERS_LEVEL];
	int ammo = 25 - ( 5 * ( pLevel - 1 ) );
	int radius = 100 * pLevel;
	int totalHealed = 0;
	int playersHealed = 0;

	if (pLevel < 1)
	{
		BOTS_Print(clientNum,"You must be level 1 to use healradius.");
	}
	else if (ent->client->ps.ammo[WP_BFG] < ammo)
	{
		BOTS_Print(clientNum,va("You must have at least %d cells to use healradius.", ammo));
	}
	else
	{
		for (i=0;i<level.maxclients;i++)
		{
			teamMember = g_entities + i;
			if (teamMember && 
				teamMember->inuse &&
				teamMember->client && 
				teamMember->health > 0 &&
				teamMember->client->ps.persistant[PERS_TEAM] == ent->bots_team &&
				Distance(ent->client->ps.origin, teamMember->client->ps.origin) <= radius)
			{
				int maxHealth = teamMember->client->ps.stats[STAT_MAX_HEALTH];
				int health = teamMember->client->ps.stats[STAT_HEALTH];
				int toHeal = maxHealth - health;
				if (toHeal > 100)
					toHeal = 100;
				if (toHeal > 0)
				{
					teamMember->health += toHeal;
					teamMember->client->ps.stats[STAT_HEALTH] += toHeal;
					playersHealed++;
					totalHealed += toHeal;
					G_AddEvent(teamMember, EV_HEALRADIUS, clientNum);
				}
			}
		}

		if (playersHealed > 0 && totalHealed > 0)
		{
			ent->client->ps.ammo[WP_BFG] -= ammo;

			if (playersHealed > 1)
				BOTS_Print(clientNum, va("Healed %d team members for a total of %d health.", playersHealed, totalHealed));
			else
				BOTS_Print(clientNum, va("Healed %d team member for a total of %d health.", playersHealed, totalHealed));
		}
	}
}

void BOTS_Nurse_CreateItem(int clientNum, char *itemName, int baseCost, int minimumLevel, qboolean modifyCostByLevel, char *abilityDescription)
{
	gentity_t*	ent = g_entities + clientNum;
	gentity_t*	drop;
	gitem_t*	item;
	vec3_t		angles, velocity, origin, forward;
	int			li_modifier;
	int			pLevel = ent->client->ps.persistant[PERS_LEVEL];
	int			cost = baseCost;

	if (pLevel < minimumLevel)
	{
		BOTS_Print(clientNum,va("You must be level %d to %s.", minimumLevel, abilityDescription));
		return;
	}

	if (modifyCostByLevel && pLevel > 0)
		cost = baseCost / pLevel;
		
	if (ent->client->ps.ammo[WP_BFG] < cost)
	{
		BOTS_Print(clientNum, va("You need %d cells to %s.", cost, abilityDescription));
		return;
	}

	ent->client->ps.ammo[WP_BFG] -= cost;

	item = BG_FindItem(itemName);

	// set aiming directions
	VectorCopy( ent->s.apos.trBase, angles );

	// set the origin of the launch away from the client
	AngleVectors (ent->client->ps.viewangles, forward, NULL, NULL);
	forward[2] = 0;
	VectorMA(ent->s.pos.trBase, 100, forward, origin);

	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

	drop = LaunchItem( item, origin, velocity );

	if (Q_stricmp (itemName, "Regeneration") == 0 )
		drop->count = 15;
}

void BOTS_NurseCommand_CreateSmallHealth(int clientNum)
{
	BOTS_Nurse_CreateItem(clientNum, "25 Health", 25, 1, qtrue, "create a small health");
}

void BOTS_NurseCommand_CreateLargeHealth(int clientNum)
{
	BOTS_Nurse_CreateItem(clientNum, "50 Health", 50, 1, qtrue, "create a large health");
}

void BOTS_NurseCommand_CreateMegaHealth(int clientNum)
{
	BOTS_Nurse_CreateItem(clientNum, "Mega Health", 100, 2, qfalse, "create a mega health");
}

void BOTS_NurseCommand_CreateMedikitHealth(int clientNum)
{
	BOTS_Nurse_CreateItem(clientNum, "Medkit", 100, 3, qfalse, "create a medikit");
}

void BOTS_NurseCommand_CreateRegenHealth(int clientNum)
{
	BOTS_Nurse_CreateItem(clientNum, "Regeneration", 100, 4, qfalse, "create a regeneration powerup");
}