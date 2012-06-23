#include "g_local.h"

void BOTS_RewriteSpawnVar(char *key, int keySize, char *value, int valueSize)
{
	if (Q_stricmp(key, "classname") == 0)
	{
		if (Q_stricmp (value, "item_pack") == 0)
			Q_strncpyz( value, "ammo_bfg", valueSize );
		else if (Q_stricmpn (value, "ammo_", 4) == 0)
			Q_strncpyz( value, "ammo_bfg", valueSize );
		else if (Q_stricmpn (value, "weapon_", 6) == 0)
			Q_strncpyz( value, "ammo_bfg", valueSize );
		else if (Q_stricmpn (value, "holdable_", 8) == 0)
			Q_strncpyz( value, "ammo_bfg", valueSize );
		else if (Q_stricmpn (value, "item_armor_", 10) == 0) {}
		else if (Q_stricmpn (value, "item_health_", 11) == 0) {}
		else if (Q_stricmpn (value, "item_quad", 8) == 0) {}
		else if (Q_stricmpn (value, "item_haste", 10) == 0) {}
		else if (Q_stricmpn (value, "item_enviro", 11) == 0) {}
		else if (Q_stricmpn (value, "item_", 4) == 0)
			Q_strncpyz( value, "ammo_bfg", valueSize );		
		else if (Q_stricmp(value,"bots_goal_all") == 0)
		{
			Q_strncpyz(value,"bots_goal", valueSize);
			G_SpawnAddVar("bots_team","free");
		}
		else if (Q_stricmp(value,"bots_goal_blue") == 0)
		{
			Q_strncpyz(value,"bots_goal", valueSize);
			G_SpawnAddVar("bots_team","blue");
		}
		else if (Q_stricmp(value,"bots_goal_red") == 0)
		{
			Q_strncpyz(value,"bots_goal", valueSize);
			G_SpawnAddVar("bots_team","red");
		}
	}
}
void BOTS_Common_DropKey(int clientNum, qboolean launch, qboolean tech)
{
	gentity_t*  ent = g_entities + clientNum;
	gentity_t*	drop;
	gitem_t*	item;
	vec3_t		angles, velocity, origin, forward, right, up, muzzle, wallp;
	trace_t		tr1;

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, NULL, NULL);

	// setup muzzle point
	CalcMuzzlePoint(ent, forward, NULL, NULL, muzzle);

	if (launch)
	{
		// Setup end point of the vector we want to trace
		VectorCopy(muzzle, wallp);
		VectorMA(wallp, 50, forward, wallp);
		trap_Trace(&tr1, muzzle, NULL, NULL, wallp, ent->s.number, MASK_SOLID);
		SnapVector( tr1.endpos );

		// check to make sure we are not too close to a wall	
		if (tr1.fraction != 1)
			return;
	}

	if (ent->client->sess.sessionTeam == TEAM_RED)
		item = BG_FindItemForKey(tech ? KEY_RED_TECH : KEY_RED_PROMO);
	else
		item = BG_FindItemForKey(tech ? KEY_BLUE_TECH : KEY_BLUE_PROMO);

	// set aiming directions
	VectorCopy( ent->s.apos.trBase, angles );

	forward[2] = 0;
	VectorCopy(ent->s.pos.trBase, origin);

	angles[PITCH] = 0;	// always forward

	if (launch)
	{
		AngleVectors( angles, velocity, NULL, NULL );
		VectorScale( velocity, 300, velocity );
		velocity[2] += 200;
	}
	else
		VectorSet(velocity, 0, 0, 0);

	drop = LaunchItem( item, origin, velocity );
	drop->parent = ent;
	drop->keyDropTime = level.time;

	// tilt the head to look up a bit
	AngleVectors (ent->client->ps.viewangles, NULL, right, up);
	right[2] = 0;
	VectorScale(forward, -1, forward);
	vectorFromAngle(forward, axisDefault[2], right, 60, forward);
	vectoangles(forward, drop->s.apos.trBase);

	ent->client->ps.stats[STAT_KEY] = KEY_NONE;

	if (tech)
		BOTS_SetTeamTechKey(ent->client->sess.sessionTeam, drop);
	else
		BOTS_SetTeamPromotionKey(ent->client->sess.sessionTeam, drop);
}

void BOTS_CommonCommand_Class(int clientNum)
{
	class_t		oldClass;
	char		s[MAX_TOKEN_CHARS];
	gentity_t *ent = g_entities + clientNum;

	if ( trap_Argc() != 2 ) {
		oldClass = ent->client->sess.sessionClass;
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", BOTS_ClassName(oldClass)) );
		return;
	}

	if ( ent->client->switchClassTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, "print \"May not switch classes more than once per 5 seconds.\n\"" );
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )	&& ent->client->sess.sessionTeam == TEAM_FREE )
		ent->client->sess.losses++;

	trap_Argv( 1, s, sizeof( s ) );

	BOTS_SetClass( ent, s );

	ent->client->switchClassTime = level.time + 5000;	
}

void BOTS_CommonCommand_LocatePromo(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	gentity_t *key = BOTS_GetTeamPromotionKey(ent->bots_team);
	
	if (!key)
		BOTS_Print(clientNum, va("Your captain is carrying the promotion key! (%d)", BOTS_GetPromotionPoints(ent->bots_team)));
	else
	{
		vec3_t distanceVector;
		int distance;
		VectorSubtract(ent->s.pos.trBase, key->s.pos.trBase, distanceVector);
		distance = (int)VectorLength(distanceVector);
		BOTS_Print(clientNum, va("Distance to promotion key: %d (%d)", distance, BOTS_GetPromotionPoints(ent->bots_team)));
	}
}

void BOTS_CommonCommand_LocateTech(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	gentity_t *key = BOTS_GetTeamTechKey(ent->bots_team);
	
	if (!key)
		BOTS_Print(clientNum, va("Your scientist is carrying the tech key! (%d)", BOTS_GetTechPoints(ent->bots_team)));
	else
	{
		vec3_t distanceVector;
		int distance;
		VectorSubtract(ent->s.pos.trBase, key->s.pos.trBase, distanceVector);
		distance = (int)VectorLength(distanceVector);
		BOTS_Print(clientNum, va("Distance to tech key: %d (%d)", distance, BOTS_GetTechPoints(ent->bots_team)));
	}
}

qboolean BOTS_CommonCommand_GetCheatNumber(gentity_t *ent, int def, int min, int max, int *out)
{
	if (CheatsOk(ent))
	{
		if (trap_Argc() == 2)
		{
			char cmd[MAX_TOKEN_CHARS];
			int value = 0;
			trap_Argv( 1, cmd, sizeof( cmd ) );
			value = atoi( cmd );
			if (value > max)
				value = max;
			else if (value < min)
				value = min;
			*out = value;
		}
		else
		{
			char cmd[MAX_TOKEN_CHARS];
			trap_Argv( 0, cmd, sizeof( cmd ) );
			BOTS_Print(ent->client->ps.clientNum, va("usage: %s number", cmd));
			return qfalse;
		}
		return qtrue;
	}
	return qfalse;
}

void BOTS_CommonCommand_SetLevel(int clientNum)
{
	int points;
	gentity_t *ent = g_entities + clientNum;
	if (BOTS_CommonCommand_GetCheatNumber(ent, 0, 0, 4, &points))
		ent->client->ps.persistant[PERS_LEVEL] = points;
}

void BOTS_CommonCommand_SetPromos(int clientNum)
{
	int points;
	gentity_t *ent = g_entities + clientNum;
	if (BOTS_CommonCommand_GetCheatNumber(ent, 0, 0, 32768, &points))
	{
		BOTS_SetPromotionPoints(ent->client->ps.persistant[PERS_TEAM], points);
		BOTS_SyncScoresConfigStrings();
	}
}

void BOTS_CommonCommand_SetTechs(int clientNum)
{
	int points;
	gentity_t *ent = g_entities + clientNum;
	if (BOTS_CommonCommand_GetCheatNumber(ent, 0, 0, 32768, &points))
	{
		BOTS_SetTechPoints(ent->client->ps.persistant[PERS_TEAM], points);
		BOTS_SyncScoresConfigStrings();
	}
}

void BOTS_Common_ApplyStriping(gentity_t *killed, gentity_t *killer)
{
	//must be on opposite teams
	//killer may not be a captain
	//killed may not be a captain
	//killer may not be a scientist
	//killed may not be a scientist
	if (	killed->bots_team != killer->bots_team
		&&	killed->bots_class != CLASS_CAPTAIN 
		&&	killer->bots_class != CLASS_CAPTAIN 
		&&	killed->bots_class != CLASS_SCIENTIST
		&&	killer->bots_class != CLASS_SCIENTIST
		)
	{
		int killedLevel = killed->client->ps.persistant[PERS_LEVEL];
		int killerLevel = killer->client->ps.persistant[PERS_LEVEL];

		//player being killed is 2 or more levels above the killer
		if (killedLevel - killerLevel >= 2)
		{
			//demote the killed player one level
			killed->client->ps.persistant[PERS_LEVEL]--;
			//promote the killer player one level
			killer->client->ps.persistant[PERS_LEVEL]++;
			//sync the killed player's captain
			BOTS_UpdateCaptainLevel(killed->bots_team);
			//sync the killer player's captain
			BOTS_UpdateCaptainLevel(killer->bots_team);
			//notify the killed they lost a level!
			BOTS_Print(killed->s.clientNum, va("You have been striped! You are now level %d!", killed->client->ps.persistant[PERS_LEVEL]));
			//notify the killer they gained a level!
			BOTS_Print(killer->s.clientNum, va("You earned a stripe! You are now level %d!", killer->client->ps.persistant[PERS_LEVEL]));
		}
	}
}

qboolean BOTS_MapEntityCheck(gentity_t *mapEntity, gentity_t *player)
{
	int	pclass = -1;		//player's class
	int	pteam = -1;			//player's team
	int	plevel = -1;		//player's level
	int	mclass = -1;		//mapentity's class
	int	mteam = -1;			//mapentity's team

	if (!player->client)
		return qtrue;

	pclass	= player->bots_class;
	pteam	= player->bots_team;
	plevel	= player->client->ps.persistant[PERS_LEVEL];
	mclass	= mapEntity->bots_class;
	mteam	= mapEntity->bots_team;

	if ((pclass == CLASS_INFILTRATOR) && (plevel > 2))
	{
		//player and mapent are on same team
		//trying to open a captain/scientist door
		if ((pteam == mteam) && ((mclass == CLASS_CAPTAIN) || (mclass == CLASS_SCIENTIST)))
		{
			return qfalse;
		}
		//player and mapent are not on same team
		//trying to open a captain/scientist door
		else if ((pteam != mteam) && ((mclass == CLASS_CAPTAIN) || (mclass == CLASS_SCIENTIST)))
		{
			//if level 4 or higher
			if (plevel > 3)
				return qtrue;
			else
				return qfalse;
		}
		else
			return qtrue;
	}
	if (mteam > 0)
	{
		if (pteam == mteam)
		{
			//class specific door
			if (mclass > 0)
			{
				//captain only door
				if ((mclass == CLASS_CAPTAIN) && (pclass == CLASS_CAPTAIN))
					return qtrue;
				//scientist only door
				else if ((mclass == CLASS_SCIENTIST) && (pclass == CLASS_SCIENTIST))
					return qtrue;
				//door's class == player's class
				else if (mclass == pclass)
					return qtrue;
				//not met above checks
				else
					return qfalse;

			}
			//only team specific, and teams match
			else
			{
				return qtrue;
			}
		}
		//team's do not match
		else
		{
			//captain door, captain only, at lvl 0
			if ((mclass == CLASS_CAPTAIN) && (pclass == CLASS_CAPTAIN) && (plevel < 1))
				return qtrue;
			else
				return qfalse;
		}
	}
	//door is not team specific check for class specific
	else
	{
		if (mclass > 0)
		{
			//door's class == player's class
			if (mclass & pclass)
				return qtrue;
			else
				return qfalse;
		}
		//any team, any class
		else
		{
			return qtrue;
		}
	}
	//all else fails, let them have it.
	return qtrue;
}

qboolean BOTS_CanUseTeleporter(gentity_t *teleporter, gentity_t *player)
{
	return BOTS_MapEntityCheck(teleporter, player);
}

qboolean BOTS_CanUseMover(gentity_t *mover, gentity_t *player)
{
	return BOTS_MapEntityCheck(mover, player);
}

qboolean BOTS_CanTouchMulti(gentity_t *mover, gentity_t *player)
{
	return BOTS_MapEntityCheck(mover, player);
}

qboolean BOTS_CanUseJumppad(gentity_t *jumppad, gentity_t *player)
{
	return BOTS_MapEntityCheck(jumppad, player);
}

qboolean BOTS_CanTouchHurt(gentity_t *hurt, gentity_t *player)
{
	return BOTS_MapEntityCheck(hurt, player);
}

void BOTS_GoalAllThink(gentity_t *self)
{
	self->nextthink = level.time + FRAMETIME;

	if (level.time > self->wait)
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

		self->wait = level.time + (self->capPad->waitTime * 1000);
	}
}

void BOTS_GoalTouch (gentity_t *ent, gentity_t *other, trace_t *trace)
{
	int			team;

	if (!ent) return;
	if (!other) return;
	if (!other->client) return;
	if (other->health < 0) return;

	team = ent->bots_team == TEAM_RED || ent->bots_team == TEAM_BLUE ? ent->bots_team : TEAM_RED;

	if (team == other->bots_team)
		Team_TouchOurFlag(ent, other, ent, team);
}

void SP_bots_goal(gentity_t *goal)
{
	trace_t tr;
	vec3_t dest;
	gcappad_t *capPad;

	if (level.numCapPads >= MAX_CAPPADS)
	{
		G_Printf("Too many capture pads defined in map.");
		return;
	}

	goal->capPad = capPad = &level.cappads[level.numCapPads];
	level.numCapPads++;

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
	
	goal->touch = BOTS_GoalTouch;

	if (goal->bots_team == TEAM_BLUE)
		goal->s.modelindex = G_ModelIndex("models/bots/capture_pad_blue.md3");
	else if (goal->bots_team == TEAM_RED)
		goal->s.modelindex = G_ModelIndex("models/bots/capture_pad_red.md3");
	else
	{
		goal->s.modelindex = G_ModelIndex("models/bots/capture_pad_red.md3");
		goal->wait = level.time + (capPad->waitTime * 1000);
		goal->bots_team = TEAM_RED;
		goal->think = BOTS_GoalAllThink;
		goal->nextthink = level.time + FRAMETIME;
	}

	VectorSet(goal->r.mins, -30, -30, 0);
	VectorSet(goal->r.maxs, 30, 30, 6);

	trap_LinkEntity (goal);
}

void BOTS_TryToPlay(gentity_t *ent)
{
	int	teamLeader;
	gentity_t *	captain;
	gentity_t *	scientist;
	qboolean changedTeams = qfalse;
	qboolean changedClasses = qfalse;
	team_t oldTeam = ent->bots_team;
	class_t oldClass = ent->bots_class;
	gclient_t *	client = ent->client;
	int clientNum = client - level.clients;

	if (ent->next_team != TEAM_SPECTATOR && changedClasses)
	{
		if (ent->next_class == CLASS_CAPTAIN) 
		{
			// and i want to do it on the other team?!
			captain = BOTS_GetTeamCaptain(changedTeams ? ent->next_team : ent->bots_team);
			if (captain)
				ent->next_class = CLASS_SOLDIER;
		}
		else if(ent->next_class == CLASS_SCIENTIST)
		{
			scientist = BOTS_GetTeamScientist(changedTeams ? ent->next_team : ent->bots_team);
			if (scientist)
				ent->next_class = CLASS_SOLDIER;
		}
	}
		
	changedTeams = oldTeam != ent->next_team ? qtrue : qfalse;
	changedClasses = oldClass != ent->next_class ? qtrue : qfalse;

	trap_Printf(va("Current Team: %s Next Team: %s Current Class: %s Next Class: %s\n", TeamName(ent->bots_team), TeamName(ent->next_team), BOTS_ClassName(ent->bots_class), BOTS_ClassName(ent->next_class)));

	if (ent->next_team == TEAM_SPECTATOR || 
			((ent->next_team == TEAM_RED || ent->next_team == TEAM_BLUE) && ent->next_class != CLASS_NONE && (changedClasses || changedTeams)))
	{

		// if the player was dead leave the body
		if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
			CopyToBodyQue(ent);
		}

		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) 
		{
			// Kill him (makes sure he loses flags, etc)
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
			player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		}

		client->sess.sessionTeam = ent->bots_team = ent->next_team;
		client->pers.teamState.state = TEAM_BEGIN;
		client->sess.spectatorState = ent->next_team == TEAM_SPECTATOR ? SPECTATOR_FREE : SPECTATOR_NOT;
		client->sess.spectatorClient = 0;
		client->sess.sessionClass = ent->bots_class = ent->next_class;

		if ( ent->bots_team == TEAM_RED || ent->bots_team == TEAM_BLUE ) {
			teamLeader = TeamLeader( ent->bots_team );
			// if there is no team leader or the team leader is a bot and this client is not a bot
			if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
				SetLeader( ent->bots_team, clientNum );
			}
		}
		// make sure there is a team leader on the team the player came from
		if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
			CheckTeamLeader( oldTeam );
		}

		ClientUserinfoChanged( clientNum );
		ClientBegin( clientNum );
		ClientRespawn( ent );

		BOTS_Print(-1, va("%s" S_COLOR_WHITE " is now playing a %s on the %s team.", client->pers.netname, BOTS_ClassName(ent->bots_class), TeamName(ent->bots_team)));
	}
}

void BOTS_SetClass(gentity_t *ent, char *s)
{
	class_t		cls;
	gentity_t *	captain;
	gentity_t *	scientist;
	gclient_t *	client = ent->client;
	if (client)
	{
		cls = BOTS_ClassNumber(s);
		if (cls != CLASS_NONE)
		{
			if (cls == CLASS_CAPTAIN)
			{
				captain = BOTS_GetTeamCaptain(ent->bots_team);
				if (captain)
				{
					if (captain == ent)
						BOTS_Print(ent->s.clientNum, "You are already the team's Captain.");
					else
						BOTS_Print(ent->s.clientNum, "Your team already has a Captain.");
					return;
				}
			}
			else if (cls == CLASS_SCIENTIST)
			{
				scientist = BOTS_GetTeamScientist(ent->bots_team);
				if (scientist)
				{
					if (scientist == ent)
						BOTS_Print(ent->s.clientNum, "You are already the team's Scientist.");
					else
						BOTS_Print(ent->s.clientNum, "Your team already has a Scientist.");
					return;
				}
			}

			ent->next_class = cls;

			BOTS_TryToPlay(ent);
		}
	}
}

void BOTS_Pickup_Key(gentity_t *key, gentity_t *player )
{
	team_t keyTeam = key->item->giTag == KEY_RED_PROMO || key->item->giTag == KEY_RED_TECH ? TEAM_RED : TEAM_BLUE;
	class_t keyClass = key->item->giTag == KEY_RED_PROMO || key->item->giTag == KEY_BLUE_PROMO ? CLASS_CAPTAIN : CLASS_SCIENTIST;
	gentity_t *owner = keyClass == CLASS_CAPTAIN ? BOTS_GetTeamCaptain(keyTeam) : BOTS_GetTeamScientist(keyTeam);

	if (level.time - key->keyDropTime < 1000)
		return;

	//teammate touched it
	if (player->bots_team == keyTeam)
	{
		//key is a promo and the captain touched it or key is a tech and the scientist touched it
		if (player->bots_class == keyClass)
			player->client->ps.stats[STAT_KEY] = key->item->giTag;
		else
			return;
	}
	else
	{
		char *teamName = keyTeam == TEAM_RED ? "red" : "blue";
		char *keyName = keyClass == CLASS_CAPTAIN ? "promotion" : "tech";
		BOTS_Print(-1, va("%s" S_COLOR_WHITE " stole the %s team's %s key!", player->client->pers.netname, teamName, keyName));
		//enemy touched it
		if (owner != NULL && owner->client)
		{
			owner->client->ps.stats[STAT_KEY] = key->item->giTag;
			G_AddEvent( owner, EV_ITEM_PICKUP, key->s.modelindex );
		}
	}

	if (keyClass == CLASS_CAPTAIN)
		BOTS_SetTeamPromotionKey(keyTeam, NULL);
	else
		BOTS_SetTeamTechKey(keyTeam, NULL);

	G_AddEvent( player, EV_ITEM_PICKUP, key->s.modelindex );
	G_FreeEntity(key);
}

void BOTS_Spawn_Goal(gentity_t *ent)
{
}

void BOTS_SpawnSetup(gentity_t *ent)
{
		char	*bots_team;
	char	*bots_class;
	team_t	team;
	class_t cls;

	G_SpawnString( "bots_team", "", &bots_team);
	if (!Q_stricmp(bots_team, "red") || !Q_stricmp(bots_team, "r"))
		team = TEAM_RED;
	else if (!Q_stricmp(bots_team, "blue") || !Q_stricmp(bots_team,"b"))
		team = TEAM_BLUE;
	else
	{
		G_SpawnInt("bots_team", "0", &team);
		if (team != TEAM_RED && team != TEAM_BLUE)
			team = TEAM_FREE;
	}
	ent->bots_team = team;

	G_SpawnString("bots_class","", &bots_class);
	cls = BOTS_ClassNumber(bots_class);
	if (cls == CLASS_NONE)
	{
		G_SpawnInt("bots_class","0", &cls);
		if (cls <= CLASS_NONE || cls >= CLASS_NUM_CLASSES)
			cls = CLASS_NONE;
	}
	ent->bots_class = cls;

	//G_DPrintf( "%s has team '%s' and class '%s' (%d,%d)\n", ent->classname, bots_team, bots_class, team, cls );
}