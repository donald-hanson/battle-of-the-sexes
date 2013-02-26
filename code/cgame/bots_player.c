#include "cg_local.h"

#define BOTS_BLIND_TIME 3000

teamInfo_t g_teamList[] = {
	{ TEAM_FREE,		0, 0, 0 },
	{ TEAM_RED,			0, 0, 0 },
	{ TEAM_BLUE,		0, 0, 0 },
	{ TEAM_SPECTATOR,	0, 0, 0 },
	{ TEAM_NUM_TEAMS,	0, 0, 0 },
};

extern char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS];
extern qboolean	CG_ParseAnimationFile( const char *filename, clientInfo_t *ci );

static qboolean	BOTS_RegisterClientSkin( clientInfo_t *ci, const char *modelName, const char *skinName ) {
	char		filename[MAX_QPATH];

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower_%s.skin", modelName, skinName );
	ci->legsSkin = trap_R_RegisterSkin( filename );

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper_%s.skin", modelName, skinName );
	ci->torsoSkin = trap_R_RegisterSkin( filename );

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/head_%s.skin", modelName, skinName );
	ci->headSkin = trap_R_RegisterSkin( filename );

	if ( !ci->legsSkin || !ci->torsoSkin || !ci->headSkin ) {
		return qfalse;
	}

	return qtrue;
}

static qboolean BOTS_RegisterClientModelname( clientInfo_t *ci, const char *modelName, const char *skinName ) 
{
	char		filename[MAX_QPATH];

	// load cmodels before models so filecache works

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", modelName );
	ci->legsModel = trap_R_RegisterModel( filename );
	if ( !ci->legsModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", modelName );
	ci->torsoModel = trap_R_RegisterModel( filename );
	if ( !ci->torsoModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", modelName );
	ci->headModel = trap_R_RegisterModel( filename );
	if ( !ci->headModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	// if any skins failed to load, return failure
	if ( !BOTS_RegisterClientSkin( ci, modelName, skinName ) ) {
		Com_Printf( "Failed to load skin file: %s : %s\n", modelName, skinName );
		return qfalse;
	}

	// load the animations
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
	if ( !CG_ParseAnimationFile( filename, ci ) ) {
		Com_Printf( "Failed to load animation file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/icon_%s.tga", modelName, skinName );
	ci->modelIcon = trap_R_RegisterShaderNoMip( filename );
	if ( !ci->modelIcon ) {
		Com_Printf( "Failed to load icon file: %s\n", filename );
		return qfalse;
	}

	return qtrue;
}

teamInfo_t *BOTS_ParseTeamInfoConfigString(const char *configString, team_t team)
{
	teamInfo_t *teamInfo = &g_teamList[team];

	int newPoints = atoi( Info_ValueForKey( configString, "score" ) );
	int newPromos = atoi( Info_ValueForKey( configString, "promo" ) );
	int newTechs = atoi( Info_ValueForKey( configString, "tech" ) );

	if (teamInfo->points != newPoints)
	{
		teamInfo->points = newPoints;
		CG_JS_Eval(va("TeamInfo.SetScore(%d, %d)", (int)team, newPoints));
	}

	if (teamInfo->promopoints != newPromos)
	{
		teamInfo->promopoints = newPromos;
		CG_JS_Eval(va("TeamInfo.SetPromotionPoints(%d, %d)", (int)team, newPromos));
	}

	if (teamInfo->techpoints != newTechs)
	{
		teamInfo->techpoints = newTechs;
		CG_JS_Eval(va("TeamInfo.SetTechPoints(%d, %d)", (int)team, newTechs));
	}

	return teamInfo;
}

void BOTS_LoadClientInfo(clientInfo_t *ci)
{
	const char	*dir, *fallback;
	int			i;
	const char	*s;
	int			clientNum;

	if ( !BOTS_RegisterClientModelname( ci, ci->modelName, ci->skinName ) ) 
	{
		if ( cg_buildScript.integer )
			CG_Error( "BOTS_RegisterClientModelname( %s, %s ) failed", ci->modelName, ci->skinName );

		// fall back
		if ( cgs.gametype >= GT_TEAM ) 
		{
			// keep skin name
			if ( !BOTS_RegisterClientModelname( ci, DEFAULT_MODEL, ci->skinName ) ) 
				if ( !BOTS_RegisterClientModelname( ci, DEFAULT_MODEL, "sniper" ) ) 
					CG_Error( "DEFAULT_MODEL / skin (%s/sniper) failed to register", DEFAULT_MODEL, ci->skinName );
		} 
		else if ( !BOTS_RegisterClientModelname( ci, DEFAULT_MODEL, "default" ) )
			CG_Error( "DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL );
	}

	// sounds
	dir = ci->modelName;
	fallback = "sarge";

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++ ) 
	{
		s = cg_customSoundNames[i];
		if ( !s )
			break;

		ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", dir, s + 1) ,qfalse);
		if ( !ci->sounds[i] )
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", fallback, s + 1) ,qfalse);
	}

	ci->deferred = qfalse;

	// reset any existing players and bodies, because they might be in bad
	// frames for this new model
	clientNum = ci - cgs.clientinfo;
	for ( i = 0 ; i < MAX_GENTITIES ; i++ ) {
		if ( cg_entities[i].currentState.clientNum == clientNum
			&& cg_entities[i].currentState.eType == ET_PLAYER ) {
			CG_ResetPlayerEntity( &cg_entities[i] );
		}
	}
}

void BOTS_Laser( centity_t* cent, int entType )
{
	refEntity_t		beam;
	clientInfo_t	*ci;
	float*			color;

	ci = &cgs.clientinfo[ cent->currentState.clientNum ];

	memset( &beam, 0, sizeof( beam ) );

	// set the start and end of beam from previously set start/end points
	VectorCopy(cent->currentState.origin, beam.origin);
	VectorCopy(cent->currentState.origin2, beam.oldorigin);

	if (entType == ET_BOTS_LASER)
	{
		beam.reType = RT_RAIL_CORE;
		beam.customShader = cgs.media.railCoreShader;
	}
	else
	{
		beam.reType = RT_RAIL_RINGS;
		beam.customShader = cgs.media.railRingsShader;
	}

	AxisClear( beam.axis );

	color = CG_TeamColor(ci->team);
	beam.shaderRGBA[0] = color[0] * 255;
	beam.shaderRGBA[1] = color[1] * 255;
	beam.shaderRGBA[2] = color[2] * 255;
	beam.shaderRGBA[3] = color[3] * 255;

	trap_R_AddRefEntityToScene( &beam );
}

void BOTS_Command_PlusGrenade()
{
	trap_SendConsoleCommand("+button11");
}

void BOTS_Command_MinusGrenade()
{
	trap_SendConsoleCommand("-button11");
}

typedef struct grenadeModelTypeInfo_s {
	grenadeModelType_t modelType;
	qboolean isDecoy;
	qboolean adjustShaderTime;
	vec_t offset;
	qhandle_t model;
	weapon_t weapon;
} grenadeModelTypeInfo_t;

grenadeModelTypeInfo_t grenadeModelTypeInfos[] = {
	// modelType,						isDecoy,	adjustShaderTime,	offset,		model,	weapon
	{GRENADE_MODEL_NORMAL,				qfalse,		qfalse,				0.0f,		0,		WP_NONE, },

	{GRENADE_MODEL_PROXIMITY,			qfalse,		qtrue,				4.0f,		0,		WP_NONE, },
	{GRENADE_MODEL_FLASH,				qfalse,		qfalse,				4.0f,		0,		WP_NONE, },
	{GRENADE_MODEL_TELEPORT,			qfalse,		qfalse,				4.0f,		0,		WP_NONE, },
	{GRENADE_MODEL_FREEZE,				qfalse,		qfalse,				4.0f,		0,		WP_NONE, },

	{GRENADE_MODEL_SHOTGUN,				qtrue,		qfalse,				0.0f,		0,		WP_SHOTGUN, },
	{GRENADE_MODEL_GRENADE_LAUNCHER,	qtrue,		qfalse,				0.0f,		0,		WP_GRENADE_LAUNCHER, },
	{GRENADE_MODEL_ROCKET_LAUNCHER,		qtrue,		qfalse,				0.0f,		0,		WP_ROCKET_LAUNCHER, },
	{GRENADE_MODEL_LIGHTNING,			qtrue,		qfalse,				0.0f,		0,		WP_LIGHTNING, },
	{GRENADE_MODEL_RAILGUN,				qtrue,		qfalse,				0.0f,		0,		WP_RAILGUN, },
	{GRENADE_MODEL_PLASMAGUN,			qtrue,		qfalse,				0.0f,		0,		WP_PLASMAGUN, },
	{GRENADE_MODEL_BFG,					qtrue,		qfalse,				0.0f,		0,		WP_BFG, },

	{GRENADE_MODEL_NUM_GRENADES,		qfalse,		qfalse,				0.0f,		0,		WP_NONE },
};

void BOTS_Grenade_RegisterModel(grenadeModelType_t modelType, const char *modelName)
{
	grenadeModelTypeInfos[modelType].model = trap_R_RegisterModel(modelName);
}

qboolean BOTS_Grenade_IsDecoyGrenade(centity_t *cent)
{
	if (cent->currentState.modelindex2 >= GRENADE_MODEL_NORMAL && cent->currentState.modelindex2 < GRENADE_MODEL_NUM_GRENADES)
		return grenadeModelTypeInfos[cent->currentState.modelindex2].isDecoy;
	return qfalse;
}

void BOTS_Grenade_PrepareDecoyGrenade(centity_t *cent)
{
	weapon_t weapon = grenadeModelTypeInfos[cent->currentState.modelindex2].weapon;
	if (weapon > WP_NONE && weapon < WP_NUM_WEAPONS)
	{
		cent->currentState.modelindex = BG_FindItemIndexForWeapon(weapon);
		cent->lerpOrigin[2] += 20;
	}
}

void BOTS_Grenade_ChangeGrenadeModel(centity_t *cent, refEntity_t *ent, entityState_t *s1)
{
	int modelIndex = cent->currentState.modelindex2;
	grenadeModelTypeInfo_t *info;
	if (modelIndex > GRENADE_MODEL_NORMAL && modelIndex < GRENADE_MODEL_NUM_GRENADES)
	{
		info = &grenadeModelTypeInfos[modelIndex];

		ent->hModel = info->model;

		if (info->adjustShaderTime)
			ent->shaderTime = s1->pos.trTime / 1000.0f;

		if (info->offset > 0.0f)
		{
			if ((s1->pos.trDelta[0] == 0) && (s1->pos.trDelta[1] == 0) && (s1->pos.trDelta[2] == 0))
				ent->origin[2] += info->offset;

			s1->time = s1->pos.trTime;
		}
	}
}

void BOTS_Init_RegisterGraphics()
{
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_NORMAL, "models/ammo/grenade1.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_PROXIMITY, "models/bots/proxie_mine.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_FLASH, "models/bots/flash_gren.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_TELEPORT, "models/bots/temp_gren.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_FREEZE, "models/bots/temp_gren.md3");

	BOTS_Grenade_RegisterModel(GRENADE_MODEL_SHOTGUN, "models/weapons2/shotgun/shotgun.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_GRENADE_LAUNCHER, "models/weapons2/grenadel/grenadel.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_ROCKET_LAUNCHER, "models/weapons2/rocketl/rocketl.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_LIGHTNING, "models/weapons2/lightning/lightning.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_RAILGUN, "models/weapons2/railgun/railgun.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_PLASMAGUN, "models/weapons2/plasma/plasma.md3");
	BOTS_Grenade_RegisterModel(GRENADE_MODEL_BFG, "models/weapons2/bfg/bfg.md3");
}

typedef struct gameStateInfo_s {
	class_t cls;
	void (*networkHandler)(int clientNum);
	void (*clientStateHandler)(jsWrapper_t *wrapper);
} gameStateInfo_t;

void BOTS_Bodyguard_Network(int clientNum);
void BOTS_Bodyguard_ClassState(jsWrapper_t *wrapper);

void BOTS_Soldier_Network(int clientNum);
void BOTS_Sodlier_ClassState(jsWrapper_t *wrapper);

void BOTS_Infiltrator_Network(int clientNum);
void BOTS_Infiltrator_ClassState(jsWrapper_t *wrapper);

gameStateInfo_t gameStateInfos[] = {
	{ CLASS_NONE,			NULL, NULL },
	{ CLASS_CAPTAIN,		NULL, NULL  },
	{ CLASS_BODYGUARD,		BOTS_Bodyguard_Network, BOTS_Bodyguard_ClassState },
	{ CLASS_SNIPER,			NULL, NULL  },
	{ CLASS_SOLDIER,		BOTS_Soldier_Network, BOTS_Sodlier_ClassState  },
	{ CLASS_BERZERKER,		NULL, NULL  },
	{ CLASS_INFILTRATOR,	BOTS_Infiltrator_Network, BOTS_Infiltrator_ClassState },
	{ CLASS_KAMIKAZEE,		NULL, NULL  },
	{ CLASS_NURSE,			NULL, NULL  },
	{ CLASS_SCIENTIST,		NULL, NULL  },
	{ CLASS_NUM_CLASSES,	NULL, NULL  }
};

void BOTS_ClassState_Parse(int clientNum)
{
	class_t cls = (class_t)trap_Net_ReadBits(4);
	gameStateInfo_t *info = &gameStateInfos[cls];
	if (info && info->networkHandler)
		info->networkHandler(clientNum);
}

void BOTS_JS_Object_SetClassState(jsWrapper_t *wrapper)
{
	playerState_t ps = cg.snap->ps;
	class_t cls = (class_t)ps.persistant[PERS_CLASS];
	gameStateInfo_t *info = &gameStateInfos[cls];
	if (info && info->clientStateHandler)
		info->clientStateHandler(wrapper);
}

void Bots_Draw_Blind()
{
	vec4_t		color = { 1,1,1,1 }; 
	int blindTime = cg.snap->ps.powerups[PW_BLIND];
	if (blindTime > cg.time)
	{
		const int iCOMPLETE_BLINDNESS_TIME = 1000;
		int iTime = blindTime - cg.time;
		if (iTime < BOTS_BLIND_TIME - iCOMPLETE_BLINDNESS_TIME)
			color[3] = (float)iTime/(float)(BOTS_BLIND_TIME - iCOMPLETE_BLINDNESS_TIME);
		CG_FillRect(0, 0, 640, 480, color);
	}
}

#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

void BOTS_Adjust_FOV(float *x, float *y)
{
	float phase;
	float v;

	if (cg.predictedPlayerState.powerups[PW_POISON] > cg.time) 
	{
		phase = (cg.time / 1000.0) * WAVE_FREQUENCY * M_PI * 2 * 6;

		if ((cg.predictedPlayerState.powerups[PW_POISON] - cg.time)>5000) {
			v = WAVE_AMPLITUDE * sin( phase ) * 25;
		}
		else {
			v = WAVE_AMPLITUDE * sin( phase ) * 
				((cg.predictedPlayerState.powerups[PW_POISON] - cg.time) / 1000) * 
				((cg.predictedPlayerState.powerups[PW_POISON] - cg.time) / 1000);
		}

		*x = *x + v;
		*y = *y - v;
	}
}

void BOTS_HealRadius(int clientNum, int nurseNum)
{
	localEntity_t	*ex;

	// Allocate a local entity that won't be rendered directly
	ex = CG_AllocLocalEntity();
	ex->leType				= LE_HEALRADIUS;
	ex->leFlags				= clientNum;	// Store it somewhere, needed to track this entity position
	ex->pos.trType			= TR_STATIONARY;
	ex->refEntity.rotation	= random() * 2.0f * M_PI; // Initial random rotation

	ex->startTime			= cg.time;
	ex->endTime				= ex->startTime + 4000;

	ex->refEntity.reType		= RT_SPRITE;
	ex->refEntity.shaderTime	= ex->startTime / 1000.0f; 

	ex->refEntity.hModel		= cgs.media.dishFlashModel;
	ex->refEntity.customShader	= cgs.media.waterBubbleShader;

	ex->refEntity.shaderRGBA[0] = 0xff;
	ex->refEntity.shaderRGBA[1] = 0xff;
	ex->refEntity.shaderRGBA[2] = 0xff;
	ex->refEntity.shaderRGBA[3] = 0xff;

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;
}

void BOTS_AddHealRadius( localEntity_t *le ) {
	float t	= 1.0f - (( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime ));
	float color;
	refEntity_t	*ent = &le->refEntity;
	vec3_t p;
	int	 i;
	int clientNum = le->leFlags;
	const int	iMAX_FLOATING_STUFF = 4;
	const float	fDISTANCE = 40.0f, fBOBBING = 5.0f, fSTEP = (2.0f*M_PI)/iMAX_FLOATING_STUFF, fINITIAL_ROTATION = ent->rotation, fMAX_ROTATION = 4.0f * 2.0f * M_PI;

	if ( t > 1 )
		t = 1.0f;
	else if (t < 0)
		t = 0.0f;

	// If there's an alpha channel, use it to fade in/out
	// Ramp up at the beginning, then slowly ramp down
	if (t>0.2f)
		color = (t-0.2f)/0.8f;
	else
		color = 1.0f - (t/0.2f);
	ent->shaderRGBA[3] = 0xff * (1.0f - color); 


	// If we're doing it for this player, use the predicted position
	if (clientNum == cg.snap->ps.clientNum) {
		VectorCopy(cg.predictedPlayerEntity.lerpOrigin, p);
	}
	else
		VectorCopy(cg_entities[clientNum].lerpOrigin, p);
	p[2] += 56.0f/2.0f;

	ent->radius			= 10;
	ent->rotation		= 0;

	for (i=0; i<iMAX_FLOATING_STUFF; i++) {
		const float a1 = ((float)i * fSTEP), a2 = a1 + (fMAX_ROTATION * t) + fINITIAL_ROTATION;

		VectorCopy(p,ent->origin);
		ent->origin[0] += sin(a2) * fDISTANCE;
		ent->origin[1] += cos(a2) * fDISTANCE;
		ent->origin[2] += sin(a1 + (t*2.0f*M_PI)) * fBOBBING;
		VectorCopy(ent->origin,ent->oldorigin);

		trap_R_AddRefEntityToScene( ent );
	}
	ent->rotation = fINITIAL_ROTATION; // restore
}