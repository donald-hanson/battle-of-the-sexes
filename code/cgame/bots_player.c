#include "cg_local.h"

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
} gameStateInfo_t;

void BOTS_Bodyguard_Network(int clientNum);

gameStateInfo_t gameStateInfos[] = {
	{ CLASS_NONE,			NULL },
	{ CLASS_CAPTAIN,		NULL },
	{ CLASS_BODYGUARD,		BOTS_Bodyguard_Network },
	{ CLASS_SNIPER,			NULL },
	{ CLASS_SOLDIER,		NULL },
	{ CLASS_BERZERKER,		NULL },
	{ CLASS_INFILTRATOR,	NULL },
	{ CLASS_KAMIKAZEE,		NULL },
	{ CLASS_NURSE,			NULL },
	{ CLASS_SCIENTIST,		NULL },
	{ CLASS_NUM_CLASSES,	NULL }
};

void BOTS_ClassState_Parse(int clientNum)
{
	class_t cls = (class_t)trap_Net_ReadBits(4);
	gameStateInfo_t *info = &gameStateInfos[cls];
	if (info->networkHandler)
		info->networkHandler(clientNum);
}