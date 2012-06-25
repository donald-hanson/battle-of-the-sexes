#include "g_local.h"

#define MAX_CAPPADS 16

typedef struct gcappad_s {
	int teamPoints;
	int promoPoints;
	int techPoints;
	int waitTime;
	qboolean alternates;
	gentity_t *goal;
	gentity_t *soldier;	//enemy solder holding this pad conquered
} gcappad_t;

gcappad_t	cappads[MAX_CAPPADS];
int			numCapPads;

//player captured the enemy flag!
void BOTS_Goal_FlagCaptured(gentity_t *player, gentity_t *pad)
{
	gcappad_t *capPad = pad == NULL ? NULL : &cappads[pad->count];
	int teamPoints = pad == NULL ? 2 : capPad->teamPoints;
	int promoPoints = pad == NULL ? 2 : capPad->promoPoints;
	int techPoints = pad == NULL ? 2 : capPad->techPoints;

	AddTeamScore(player->s.pos.trBase, player->bots_team, teamPoints);
	BOTS_AddPromotionPoints(player->bots_team, promoPoints);
	BOTS_AddTechPoints(player->bots_team, techPoints);
}

qboolean BOTS_Goal_CanCapture(gentity_t *flag, gentity_t *player, gentity_t *pad)
{
	if (pad == NULL && numCapPads > 0)
		return qfalse;
	if (pad)
	{
		gcappad_t *capPad = &cappads[pad->count];
		if (BOTS_Goal_IsConquerValid(pad, capPad))
			return qfalse;
	}
	
	return qtrue;
}

qboolean BOTS_Goal_IsConquerValid(gentity_t *ent, gcappad_t *capPad)
{
	gentity_t *soldier = capPad->soldier;
	float maxDistance = 400.0f;
	int level = 0;
	if (soldier == NULL)
		return qfalse;
	if (!soldier->inuse)
		return qfalse;
	if (!soldier->client)
		return qfalse;
	if (soldier->bots_class != CLASS_SOLDIER)
		return qfalse;
	if (soldier->health <= 0)
		return qfalse;
	if (soldier->client->ps.stats[STAT_HEALTH] <= 0)
		return qfalse;

	level = soldier->client->ps.persistant[PERS_LEVEL];
	if (level < 1)
		return qfalse;

	if (level > 1)
		maxDistance = maxDistance * (1.0f + ((float)level * 0.5f));
	if (Distance(soldier->s.pos.trBase, ent->s.pos.trBase) > maxDistance)
		return qfalse;

	return qtrue;
}

void BOTS_Goal_ReleaseConquer(gentity_t *pad)
{
	gcappad_t *capPad = &cappads[pad->count];
	capPad->soldier = (gentity_t *)NULL;
}

void BOTS_Goal_TryToConquer(gentity_t *player, gentity_t *pad)
{
	int i = 0;
	gcappad_t *capPad = &cappads[pad->count];
	gcappad_t *temp;
	if (player->bots_class == CLASS_SOLDIER)
	{
		if (!BOTS_Goal_IsConquerValid(pad, capPad))
		{
			for(i=0;i<numCapPads;i++)
			{
				if (i == pad->count)
					continue;
				temp = &cappads[i];
				if (temp->soldier != NULL && temp->soldier == player)
					BOTS_Goal_ReleaseConquer(temp->goal);
			}

			capPad->soldier = player;
			trap_Printf(va("Pad #%d conquered by %s\n", pad->count, player->client->pers.netname));
		}
	}
}

void BOTS_Goal_Think(gentity_t *self)
{
	gcappad_t *capPad = &cappads[self->count];

	self->think = BOTS_Goal_Think;
	self->nextthink = level.time + FRAMETIME;

	if (!BOTS_Goal_IsConquerValid(self, capPad))
		BOTS_Goal_ReleaseConquer(self);
	
	if (capPad->alternates && level.time > self->wait)
	{
		if (self->bots_team == TEAM_RED)
		{
			self->bots_team = TEAM_BLUE;
			self->s.modelindex = G_ModelIndex("models/bots/capture_pad_blue.md3");
		}
		else 
		{
			self->bots_team = TEAM_RED;
			self->s.modelindex = G_ModelIndex("models/bots/capture_pad_red.md3");
		}

		self->wait = level.time + (capPad->waitTime * 1000);
	}
}

void BOTS_Goal_Touch (gentity_t *ent, gentity_t *other, trace_t *trace)
{
	int			team;

	if (!ent) return;
	if (!other) return;
	if (!other->client) return;
	if (other->health < 0) return;

	team = ent->bots_team == TEAM_RED || ent->bots_team == TEAM_BLUE ? ent->bots_team : TEAM_RED;

	if (team == other->bots_team)
		Team_TouchOurFlag(ent, other, ent, team);
	else
		BOTS_Goal_TryToConquer(other, ent);
}

void BOTS_Spawn_Goal(gentity_t *goal)
{
	trace_t tr;
	vec3_t dest;
	gcappad_t *capPad;

	if (numCapPads >= MAX_CAPPADS)
	{
		G_Printf("Too many capture pads defined in map.");
		return;
	}

	capPad = &cappads[numCapPads];
	goal->count = numCapPads;
	capPad->goal = goal;
	numCapPads++;

	G_SpawnInt("bots_points", "2", &capPad->teamPoints);
	G_SpawnInt("bots_promos", "2", &capPad->promoPoints);
	G_SpawnInt("bots_techs", "2", &capPad->techPoints);	
	G_SpawnInt("bots_wait", "10", &capPad->waitTime);

	G_Printf("Capture Pad - Team: %d Points: %d Promos: %d Techs: %d Wait: %d\n", goal->bots_team, capPad->teamPoints, capPad->promoPoints, capPad->techPoints, capPad->waitTime);

	goal->s.eType = ET_GENERAL;
	goal->clipmask = CONTENTS_SOLID;
	goal->s.number = goal - g_entities;
	goal->r.contents = CONTENTS_SOLID;

	// Try to make the cap pad touch the floor	
	VectorSet( dest, goal->s.origin[0], goal->s.origin[1], goal->s.origin[2] - 4096 );
	trap_Trace( &tr, goal->s.origin, goal->r.mins, goal->r.maxs, dest, goal->s.number, MASK_SOLID );
	goal->s.groundEntityNum = tr.entityNum;
	G_SetOrigin( goal, tr.endpos );
	
	goal->touch = BOTS_Goal_Touch;
	goal->think = BOTS_Goal_Think;
	goal->nextthink = level.time + FRAMETIME;

	if (goal->bots_team == TEAM_BLUE)
		goal->s.modelindex = G_ModelIndex("models/bots/capture_pad_blue.md3");
	else if (goal->bots_team == TEAM_RED)
		goal->s.modelindex = G_ModelIndex("models/bots/capture_pad_red.md3");
	else
	{
		capPad->alternates = qtrue;
		goal->s.modelindex = G_ModelIndex("models/bots/capture_pad_red.md3");
		goal->wait = level.time + (capPad->waitTime * 1000);
		goal->bots_team = TEAM_RED;
	}

	VectorSet(goal->r.mins, -30, -30, 0);
	VectorSet(goal->r.maxs, 30, 30, 6);

	trap_LinkEntity (goal);
}
