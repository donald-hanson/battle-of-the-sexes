#include "g_local.h"

#define BOTS_PACK_AMMO 100
#define BOTS_PACK_ARMOR 100

typedef struct classCommandInfo_s {
	char *command;
	void (*handler)(int clientNum);
} classCommandInfo_t;

typedef struct classInfo_s {
	class_t value;
	char *name;
	weapon_t primaryWeapon;
	int hasHook;
	qboolean hasStinger;
	grenadeType_t offhandGrenadeType;
	qboolean (*fireWeaponHandler)(gentity_t *ent);
	void (*spawnHandler)(gentity_t *ent);
	void (*deathHandler)(gentity_t *killed, gentity_t *killedBy, gentity_t *killer, int damage, int meansOfDeath);
	void (*killerHandler)(gentity_t *killer, gentity_t *killedBy, gentity_t *killed, int damage, int meansOfDeath);
	void (*networkHandler)(int clientNum);
	void (*modifyEntityHandler)();
	void (*rollbackEntityHandler)();
	classCommandInfo_t *commands;
} classInfo_t;

classCommandInfo_t g_commonCommands[] = {
	{ "class",			BOTS_CommonCommand_Class },
	{ "locatepromo",	BOTS_CommonCommand_LocatePromo },
	{ "locatetech",		BOTS_CommonCommand_LocateTech },
	{ "level",			BOTS_CommonCommand_SetLevel },
	{ "promos",			BOTS_CommonCommand_SetPromos },
	{ "techs",			BOTS_CommonCommand_SetTechs },
	{ NULL, NULL }
};

classCommandInfo_t g_noneCommands[] = {
	{ NULL, NULL }
};
classCommandInfo_t g_captainCommands[] = {
	{ "droppromote",	BOTS_CaptainCommand_DropPromote },
	{ "promote",		BOTS_CaptainCommand_Promote },
	{ "demote",			BOTS_CaptainCommand_Demote },
	{ NULL, NULL }
};
classCommandInfo_t g_bodyguardCommands[] = {
	{ "laser",		BOTS_BodyguardCommand_Laser },
	{ "laseroff",	BOTS_BodyguardCommand_LaserOff },
	{ "laseron",	BOTS_BodyguardCommand_LaserOn },
	{ "laserkill",	BOTS_BodyguardCommand_LaserKill },
	{ "protect",	BOTS_BodyguardCommand_Protect },
	{ "decoy",		BOTS_BodyguardCommand_Decoy },
	{ NULL, NULL }
};
classCommandInfo_t g_sniperCommands[] = {
	{ NULL, NULL }
};
classCommandInfo_t g_soldierCommands[] = {
	{"rapid",	BOTS_SoldierCommand_Rapid },
	{"guide1",	BOTS_SoldierCommand_Guide1 },
	{"guide2",	BOTS_SoldierCommand_Guide2 },
	{"tag",		BOTS_SoldierCommand_Tag },
	{"split1",	BOTS_SoldierCommand_Split1 },
	{"split2",	BOTS_SoldierCommand_Split2 },
	{"split3",	BOTS_SoldierCommand_Split3 },
	{ NULL, NULL }
};
classCommandInfo_t g_berzerkerCommands[] = {
	{ NULL, NULL }
};
classCommandInfo_t g_infiltratorCommands[] = {
	{ "disguise", BOTS_InfiltratorCommand_Disguise },
	{ NULL, NULL }
};
classCommandInfo_t g_kamikazeeCommands[] = {
	{ NULL, NULL }
};
classCommandInfo_t g_nurseCommands[] = {
	{ NULL, NULL }
};
classCommandInfo_t g_scientistCommands[] = {
	{ "droptech", BOTS_ScientistCommand_DropTech },
	{ NULL, NULL }
};

classInfo_t g_classList[] = {
	// value,			name,			primaryWeapon,		hasHook,hasStinger,	offhandGrenadeType,	fireWeaponHandler,		spawnHandler,		deathHandler,		killerHandler,			networkHandler,			modifyEntityHandler,				rollbackEntityHandler,					commandList
	{ CLASS_NONE,		"None",			WP_NONE,			-1,		qfalse,		GRENADE_NORMAL,		NULL,					NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_noneCommands			},
	{ CLASS_CAPTAIN,	"Captain",		WP_BFG,				2,		qtrue,		GRENADE_PROXIMITY,	NULL,					BOTS_CaptainSpawn,	BOTS_CaptainDeath,	NULL,					NULL,					NULL,								NULL,									g_captainCommands		},
	{ CLASS_BODYGUARD,	"Bodyguard",	WP_SHOTGUN,			0,		qtrue,		GRENADE_PROXIMITY,	NULL,					NULL,				NULL,				NULL,					BOTS_Bodyguard_Network,	BOTS_Bodyguard_Modify_EntityState,	BOTS_Bodyguard_Rollback_EntityState,	g_bodyguardCommands		},
	{ CLASS_SNIPER,		"Sniper",		WP_RAILGUN,			0,		qtrue,		GRENADE_NORMAL,		NULL,					NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_sniperCommands		},
	{ CLASS_SOLDIER,	"Soldier",		WP_ROCKET_LAUNCHER,	-1,		qtrue,		GRENADE_NORMAL,		BOTS_Soldier_FireWeapon,NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_soldierCommands		},
	{ CLASS_BERZERKER,	"Berzerker",	WP_GAUNTLET,		-1,		qfalse,		GRENADE_NORMAL,		NULL,					NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_berzerkerCommands		},
	{ CLASS_INFILTRATOR,"Infiltrator",	WP_PLASMAGUN,		0,		qtrue,		GRENADE_DECOY,		NULL,					NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_infiltratorCommands	},
	{ CLASS_KAMIKAZEE,	"Kamikazee",	WP_GRENADE_LAUNCHER,0,		qtrue,		GRENADE_NORMAL,		NULL,					NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_kamikazeeCommands		},
	{ CLASS_NURSE,		"Nurse",		WP_MACHINEGUN,		0,		qtrue,		GRENADE_FLASH,		NULL,					NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_nurseCommands			},
	{ CLASS_SCIENTIST,	"Scientist",	WP_LIGHTNING,		0,		qtrue,		GRENADE_FREEZE,		NULL,					BOTS_ScientistSpawn,BOTS_ScientistDeath,BOTS_ScientistKiller,	NULL,					NULL,								NULL,									g_scientistCommands		},
	{ CLASS_NUM_CLASSES,NULL,			WP_NUM_WEAPONS,		-1,		qfalse,		GRENADE_NORMAL,		NULL,					NULL,				NULL,				NULL,					NULL,					NULL,								NULL,									g_noneCommands			}
};

//the health/armor values are based on the following:
//Light
//		Sniper
//		Infil
//		Nurse
//Medium
//		Kami
//		Bodyguard
//		Scientist
//Heavy
//		Soldier
//		Zerker
//Captain
//		Captain

#define MALE_CAPTAIN	250
#define MALE_HEAVY		225
#define	MALE_MEDIUM		200
#define MALE_LIGHT		175

#define FEMALE_CAPTAIN	275
#define FEMALE_HEAVY	250
#define	FEMALE_MEDIUM	225
#define FEMALE_LIGHT	200

int maleClassHealthLimits[] = {
	//none	
	0,		
	//captain		bodyguard		sniper		
	MALE_CAPTAIN,	MALE_MEDIUM,	MALE_LIGHT, 
	//soldier		berzerker		infiltrator	
	MALE_HEAVY,		MALE_HEAVY,		MALE_LIGHT,
	//kamikazee		nurse			scientist
	MALE_MEDIUM,	MALE_LIGHT,		MALE_LIGHT, 
	// max
	0
};

int femaleClassHealthLimits[] = {
	//none	
	0,		
	//captain		bodyguard		sniper		
	FEMALE_CAPTAIN,	FEMALE_MEDIUM,	FEMALE_LIGHT, 
	//soldier		berzerker		infiltrator	
	FEMALE_HEAVY,	FEMALE_HEAVY,	FEMALE_LIGHT,
	//kamikazee		nurse			scientist
	FEMALE_MEDIUM,	FEMALE_LIGHT,	FEMALE_LIGHT, 
	// max
	0
};

#define MALE_CAPTAIN_A		250
#define MALE_HEAVY_A		225
#define	MALE_MEDIUM_A		200
#define MALE_LIGHT_A		175

#define FEMALE_CAPTAIN_A	275
#define FEMALE_HEAVY_A		250
#define	FEMALE_MEDIUM_A		225
#define FEMALE_LIGHT_A		200

int maleClassArmorLimits[] = {
	//none	
	0,		
	//captain		bodyguard		sniper		
	MALE_CAPTAIN_A,	MALE_MEDIUM_A,	MALE_LIGHT_A, 
	//soldier		berzerker		infiltrator	
	MALE_HEAVY_A,	MALE_HEAVY_A,	MALE_LIGHT_A,
	//kamikazee		nurse			scientist
	MALE_MEDIUM_A,	MALE_LIGHT_A,	MALE_LIGHT_A, 
	// max
	0
};

int femaleClassArmorLimits[] = {
	//none	
	0,		
	//captain			bodyguard			sniper		
	FEMALE_CAPTAIN_A,	FEMALE_MEDIUM_A,	FEMALE_LIGHT_A, 
	//soldier			berzerker			infiltrator	
	FEMALE_HEAVY_A,		FEMALE_HEAVY_A,		FEMALE_LIGHT_A,
	//kamikazee			nurse				scientist
	FEMALE_MEDIUM_A,	FEMALE_LIGHT_A,		FEMALE_LIGHT_A, 
	// max
	0
};

int		g_numClasses = sizeof(g_classList) / sizeof(g_classList[0]) - 1;

typedef struct teamInfo_s {
	team_t		team;
	gentity_t *	captain;
	gentity_t *	scientist;
	gentity_t *	promokey;
	gentity_t *	techkey;
	int			promopoints;
	int			techpoints;
	int			*classHealthLimits;
	int			*classArmorLimits;
} teamInfo_t;

teamInfo_t g_teamList[] = {
	{ TEAM_FREE,		NULL, NULL, NULL, NULL, 0, 0, NULL, NULL },
	{ TEAM_RED,			NULL, NULL, NULL, NULL, 0, 0, maleClassHealthLimits,	maleClassArmorLimits },
	{ TEAM_BLUE,		NULL, NULL, NULL, NULL, 0, 0, femaleClassHealthLimits,	femaleClassArmorLimits },
	{ TEAM_SPECTATOR,	NULL, NULL, NULL, NULL, 0, 0, NULL, NULL },
	{ TEAM_NUM_TEAMS,	NULL, NULL, NULL, NULL, 0, 0, NULL, NULL }
};

int g_numTeams = sizeof(g_teamList) / sizeof(g_teamList[0]) - 1;

void BOTS_InitGame(void)
{
	int i;
	for (i=0;i<g_numTeams;i++)
	{
		teamInfo_t teamInfo = g_teamList[i];
		teamInfo.captain = NULL;
		teamInfo.promokey = NULL;
		teamInfo.scientist = NULL;
		teamInfo.techkey = NULL;
		teamInfo.promopoints = 0;
		teamInfo.techpoints = 0;
	}
}

void BOTS_ClientSpawn(gentity_t *ent)
{
	gclient_t	*client;
	int			defaultAmmo;
	int			maxAmmo;
	teamInfo_t teamInfo = g_teamList[ent->bots_team];
	classInfo_t classInfo = g_classList[ent->bots_class];
	int currentLevel = ent->client->ps.persistant[PERS_LEVEL];

	if (classInfo.value > CLASS_NONE && classInfo.value < CLASS_NUM_CLASSES)
	{
		//sync the known captain and scientist values
		if (classInfo.value == CLASS_CAPTAIN)
			teamInfo.captain = ent;
		else if (classInfo.value == CLASS_SCIENTIST)
			teamInfo.scientist = ent;
		else if (teamInfo.captain == ent)
			teamInfo.captain = NULL;
		else if (teamInfo.scientist == ent)
			teamInfo.scientist = NULL;

		client = ent->client;

		//give the player a default weapon and base ammo
		client->ps.stats[STAT_WEAPONS] = ( 1 << classInfo.primaryWeapon );
		defaultAmmo = BG_FindItemForWeapon(classInfo.primaryWeapon)->quantity;
		if (defaultAmmo == 0)
		{
			defaultAmmo = -1;
			maxAmmo = -1;
		}
		else
		{
			defaultAmmo *= 10;
			maxAmmo = defaultAmmo * 1.5;
		}

		if (currentLevel > 0)
				defaultAmmo = maxAmmo;

		client->ps.maxammo[WP_BFG] = 50;
		client->ps.maxammo[WP_GRENADE_LAUNCHER] = 50;
		client->ps.maxammo[WP_GAUNTLET] = -1;

		client->ps.ammo[classInfo.primaryWeapon] = defaultAmmo;
		client->ps.maxammo[classInfo.primaryWeapon] = maxAmmo;

		//give the player a hook if the class allows it and the player meets the minimum level requirement
		if (classInfo.hasHook >= 0 && currentLevel >= classInfo.hasHook)
		{
			client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GRAPPLING_HOOK );
			client->ps.ammo[WP_GRAPPLING_HOOK] = -1;
			client->ps.maxammo[WP_GRAPPLING_HOOK] = -1;
		}
		//give the player a stinger if the class allows it
		if (classInfo.hasStinger)
		{
			client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_STINGER );
			client->ps.ammo[WP_STINGER] = -1;
			client->ps.maxammo[WP_STINGER] = -1;
		}
		
		//Set the player's max health based on their class and level
		if (teamInfo.classHealthLimits != NULL)
		{
			client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth = teamInfo.classHealthLimits[classInfo.value] * (1.0f + (currentLevel / 4.0f));
			//if they are level 1 or higher start them with more health
			if (currentLevel > 0)
				ent->health = client->ps.stats[STAT_HEALTH] = teamInfo.classHealthLimits[classInfo.value];
			else
				ent->health = client->ps.stats[STAT_HEALTH] = 100;
		}
		else
		{
			client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth = 100;
			ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] + 25;
		}

		//Set the player's max armor based on their class and level
		if (teamInfo.classArmorLimits != NULL)
		{
			client->ps.stats[STAT_MAX_ARMOR] = teamInfo.classArmorLimits[classInfo.value] * (1.0f + (currentLevel / 4.0f));
			//if they are level 1 or higher give them a bit of armor to start with
			if (currentLevel > 0)
				client->ps.stats[STAT_ARMOR] = teamInfo.classArmorLimits[classInfo.value];
		}

		if (classInfo.spawnHandler)
			classInfo.spawnHandler(ent);
	}
}

void BOTS_PlayerDeath(gentity_t *killed, gentity_t *killedBy, gentity_t *killer, int damage, int meansOfDeath)
{
	if (!killed->client && !killer->client)
		return;

	if (g_classList[killed->bots_class].deathHandler)
		g_classList[killed->bots_class].deathHandler(killed, killedBy, killer, damage, meansOfDeath);

	if (g_classList[killer->bots_class].killerHandler)
		g_classList[killer->bots_class].killerHandler(killer, killedBy, killed, damage, meansOfDeath);

	BOTS_Common_ApplyStriping(killed, killer);
}

qboolean BOTS_Client_FireWeapon(gentity_t *ent)
{
	if (g_classList[ent->bots_class].fireWeaponHandler)
		return g_classList[ent->bots_class].fireWeaponHandler(ent);
	return qfalse;
}

void BOTS_ClientDisconnect(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	teamInfo_t teamInfo = g_teamList[ent->bots_team];

	//if i am the captain or scientist, remove me
	if (teamInfo.captain == ent)
		teamInfo.captain = NULL;
	else if (teamInfo.scientist == ent)
		teamInfo.scientist = NULL;

	//auto-demote me to level 0 because i'm outta here!
	BOTS_AutoDemote(clientNum);
}

char *BOTS_ClassName(class_t cls)
{
	return g_classList[cls].name;
}

grenadeType_t BOTS_GetGrenadeType(class_t cls)
{
	return g_classList[cls].offhandGrenadeType;
}

class_t BOTS_ClassNumber(char *s)
{
	int i;
	classInfo_t info;
	for (i=0;i<g_numClasses;i++)
	{
		info = g_classList[i];
		if (Q_stricmp(s,info.name) == 0)
			return info.value;
	}
	return CLASS_NONE;
}

team_t BOTS_TeamNumber(char *s)
{
	if (Q_stricmp(s, "red") == 0)
		return TEAM_RED;
	else if(Q_stricmp(s, "blue") == 0)
		return TEAM_BLUE;
	return TEAM_FREE;
}

void BOTS_Print(int clientNum, char* text)
{
	if (clientNum == -1)
		trap_SendServerCommand( -1, va("print \"%s\n\"", text));
	else
	{
		gentity_t* ent = g_entities + clientNum;
		if (ent->client) 
			trap_SendServerCommand(clientNum, va("print \"%s\n\"", text));
	}
}

void BOTS_Print_Team(team_t team, char *text)
{
	int i;
	for (i = 0; i < level.maxclients; i++)
	{
		gentity_t* player = g_entities + i;
		if (!player->client)
			continue;
		if (player->client->sess.sessionTeam != team)
			continue;
		BOTS_Print(i, text);
	}
}

qboolean BOTS_IsClassName(char *s)
{
	return BOTS_ClassNumber(s) == CLASS_NONE ? qfalse : qtrue;
}

qboolean BOTS_CanPickupWeapon(gentity_t *weapon, gentity_t *player)
{
	return g_classList[player->bots_class].primaryWeapon == weapon->item->giTag ? qtrue : qfalse;
}

qboolean BOTS_CanPickupAmmo(gentity_t *ammo, gentity_t *player)
{
	return qtrue;
}

qboolean BOTS_Add_Ammo(gentity_t *player, weapon_t weapon, int amount)
{
	if (!player->client)
		return qfalse;

	if (player->client->ps.ammo[weapon] < player->client->ps.maxammo[weapon])
	{
		player->client->ps.ammo[weapon] += amount;
		if (player->client->ps.ammo[weapon] > player->client->ps.maxammo[weapon])
			player->client->ps.ammo[weapon] = player->client->ps.maxammo[weapon];
		return qtrue;
	}
	return qfalse;
}

qboolean BOTS_Add_Stat(gentity_t *player, statIndex_t stat, statIndex_t statMax, int amount)
{
	if (!player->client)
		return qfalse;

	if (player->client->ps.stats[stat] < player->client->ps.stats[statMax])
	{
		player->client->ps.stats[stat] += amount;
		if (player->client->ps.stats[stat] > player->client->ps.stats[statMax])
			player->client->ps.stats[stat] = player->client->ps.stats[statMax];
		return qtrue;
	}
	return qfalse;
}

int BOTS_Pickup_Ammo(gentity_t *ammo, gentity_t *player)
{
	int i = 0;
	qboolean consumed = qfalse;
	for (i=WP_NONE+1;i<WP_NUM_WEAPONS;i++)
		if (BOTS_Add_Ammo(player, (weapon_t)i, BOTS_PACK_AMMO))
			consumed = qtrue;

	if (BOTS_Add_Stat(player, STAT_ARMOR, STAT_MAX_ARMOR, BOTS_PACK_ARMOR))
		consumed = qtrue;

	if (consumed)
		return 40;
	return 5;
	
}

gentity_t *BOTS_GetTeamCaptain(team_t team)
{
	return g_teamList[team].captain;
}

gentity_t *BOTS_GetTeamPromotionKey(team_t team)
{
	return g_teamList[team].promokey;
}

void BOTS_SetTeamPromotionKey(team_t team, gentity_t *key)
{
	g_teamList[team].promokey = key;
}

void BOTS_AddPromotionPoints(team_t team, int points)
{
	g_teamList[team].promopoints += points;
}

void BOTS_SetPromotionPoints(team_t team, int points)
{
	g_teamList[team].promopoints = points;
}

int BOTS_GetPromotionPoints(team_t team)
{
	return g_teamList[team].promopoints;
}

gentity_t *BOTS_GetTeamScientist(team_t team)
{
	return g_teamList[team].scientist;
}

gentity_t *BOTS_GetTeamTechKey(team_t team)
{
	return g_teamList[team].techkey;
}

void BOTS_SetTeamTechKey(team_t team, gentity_t *key)
{
	g_teamList[team].techkey = key;
}

void BOTS_AddTechPoints(team_t team, int points)
{
	g_teamList[team].techpoints += points;
}

void BOTS_SetTechPoints(team_t team, int points)
{
	g_teamList[team].techpoints = points;
}

int BOTS_GetTechPoints(team_t team)
{
	return g_teamList[team].techpoints;
}

qboolean BOTS_ClientCommand_Handler(int clientNum, char *cmd, classCommandInfo_t *commands)
{
	int i;
	if (commands)
	{
		for (i=0;;i++)
		{
			classCommandInfo_t cmdInfo = commands[i];
			if (cmdInfo.command == NULL)
				break;
			if (Q_stricmp(cmdInfo.command, cmd) == 0)
			{
				cmdInfo.handler(clientNum);
				return qtrue;
			}
		}
	}
	return qfalse;
}

void BOTS_ClientCommand(int clientNum)
{
	gentity_t *	ent;
	char		cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client )
		return;		// not fully in game yet

	trap_Argv( 0, cmd, sizeof( cmd ) );
	if (BOTS_IsClassName(cmd)) //class name?
		BOTS_SetClass(ent, cmd);
	else if (!BOTS_ClientCommand_Handler(clientNum, cmd, g_commonCommands))	//not a common command
		if (!BOTS_ClientCommand_Handler(clientNum, cmd, g_classList[ent->bots_class].commands)) //not a class command
			trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}

void BOTS_UpdateCaptainLevel(team_t team)
{
	int playerLevel;
	int i;
	qboolean alone = qtrue;
	int lowest = 4;
	gentity_t *captain = BOTS_GetTeamCaptain(team);

	if (captain)
	{
		for (i = 0; i < level.maxclients; i++)
		{
			gentity_t* player = g_entities + i;
			if (!player->client)
				continue;
			if (player->client->sess.sessionTeam != team)
				continue;
			if (player->bots_class == CLASS_CAPTAIN)
				continue;
			
			playerLevel = player->client->ps.persistant[PERS_LEVEL];
			if (playerLevel < lowest)
				lowest = playerLevel;
			alone = qfalse;
		}
		if (alone)
			lowest = 0;
		captain->client->ps.persistant[PERS_LEVEL] = lowest;
	}	
}

void BOTS_AutoDemote(int clientNum)
{
	gentity_t *player = g_entities + clientNum;
	team_t team = player->bots_team;
	if (!player || !player->client)
		return;

	//only auto-demote non-captains above level 0
	if (player->bots_class != CLASS_CAPTAIN && player->client->ps.persistant[PERS_LEVEL] > 0)
	{
		while (player->client->ps.persistant[PERS_LEVEL] > 0)
		{
			//demote the player
			player->client->ps.persistant[PERS_LEVEL]--;

			//add the promotion points back to the team
			BOTS_AddPromotionPoints(team, player->client->ps.persistant[PERS_LEVEL] + 1);
		}

		//sync the captain's level
		BOTS_UpdateCaptainLevel(team);
	}
}
void BOTS_ClientSkin(gentity_t *ent, char *outModel, char *outHeadModel)
{
	char *model;

	team_t team = ent->client->sess.sessionTeam;
	class_t cls = ent->client->sess.sessionClass;

	BOTS_Infiltrator_AdjustClientSkin(ent, &team, &cls);

	switch (cls)
	{
		case CLASS_CAPTAIN:
			model = team == TEAM_BLUE ? "fcaptain/fcaptain2" : "mcaptain/captain";
			break;
		case CLASS_BODYGUARD:
			model = team == TEAM_BLUE ? "female/fbodyguard" : "male/bodyguard";
			break;
		case CLASS_SNIPER:
			model = team == TEAM_BLUE ? "female/fsniper" : "male/sniper";
			break;
		case CLASS_SOLDIER:
			model = team == TEAM_BLUE ? "female/fsoldier" : "male/soldier";
			break;
		case CLASS_BERZERKER:
			model = team == TEAM_BLUE ? "female/fberzerker" : "male/berzerker";
			break;
		case CLASS_INFILTRATOR:
			model = team == TEAM_BLUE ? "female/finfiltrator" : "male/infiltrator";
			break;
		case CLASS_KAMIKAZEE:
			model = team == TEAM_BLUE ? "female/fkamikazee" : "male/kamikazee";
			break;
		case CLASS_NURSE:
			model = team == TEAM_BLUE ? "female/fnurse" : "male/nurse";
			break;
		case CLASS_SCIENTIST:
			model = team == TEAM_BLUE ? "fscientist/fscientist" : "mscientist/scientist";
			break;
		default:
			model = team == TEAM_BLUE ? "female/fsniper" : "male/sniper";
			break;
	}

	Q_strncpyz( outModel, model, MAX_QPATH );
	Q_strncpyz( outHeadModel, model, MAX_QPATH );
}

char *BOTS_BuildTeamInfoConfigString(team_t team)
{
	return va("%i %i %i", level.teamScores[team], BOTS_GetPromotionPoints(team), BOTS_GetTechPoints(team));
}

void BOTS_SyncScoresConfigStrings()
{
	trap_SetConfigstring( CS_SCORES1, BOTS_BuildTeamInfoConfigString(TEAM_RED) );
	trap_SetConfigstring( CS_SCORES2, BOTS_BuildTeamInfoConfigString(TEAM_BLUE) );
}


qboolean BOTS_ClassState_Changed(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;
	if (ent == NULL)
		return qfalse;
	if (!ent->inuse)
		return qfalse;
	if (!ent->client)
		return qfalse;
	if (ent->bots_team != TEAM_RED && ent->bots_team != TEAM_BLUE)
		return qfalse;
	if (ent->bots_class == CLASS_NONE || ent->bots_class == CLASS_NUM_CLASSES)
		return qfalse;

	// must be a valid entity, in use, have a client, have a valid team and a valid class
	return qtrue;
}

void BOTS_ClassState_Append(int clientNum)
{
	gentity_t *ent;
	classInfo_t *info;

	if (!BOTS_ClassState_Changed(clientNum))
		return;

	ent = g_entities + clientNum;
	info = &g_classList[ent->bots_class];

	// write the class so the client side knows how to delegate the remainder of the message
	trap_Net_WriteBits(ent->bots_class, 4);
	if (info->networkHandler)
		info->networkHandler(clientNum);
}

void BOTS_Modify_EntityState()
{
	classInfo_t *info;
	int i=0;
	for (i=CLASS_NONE;i<CLASS_NUM_CLASSES;i++)
	{
		info = &g_classList[i];
		if (info->modifyEntityHandler)
			info->modifyEntityHandler();
	}
}

void BOTS_Rollback_EntityState()
{
	classInfo_t *info;
	int i=0;
	for (i=CLASS_NONE;i<CLASS_NUM_CLASSES;i++)
	{
		info = &g_classList[i];
		if (info->rollbackEntityHandler)
			info->rollbackEntityHandler();
	}
}