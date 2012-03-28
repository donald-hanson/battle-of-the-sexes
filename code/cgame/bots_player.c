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