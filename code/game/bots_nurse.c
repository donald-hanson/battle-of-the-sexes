#include "g_local.h"

typedef struct nurseState_s {
	qboolean poison;
} nurseState_t;

nurseState_t nurseStates[MAX_CLIENTS];

nurseState_t *BOTS_Nurse_GetState(int clientNum)
{
	return &nurseStates[clientNum];
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
				BOTS_Print(player->s.clientNum, "Cleared poison\n");
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
				BOTS_Print(player->s.clientNum, "Applied poison\n");
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
		BOTS_Print(clientNum, "Poison Enabled\n");
	else
		BOTS_Print(clientNum, "Poison Disabled\n");
}
