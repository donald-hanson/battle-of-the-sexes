#include "../qcommon/q_shared.h"
#include "cg_local.h"
#include <jsapi.h>

static JSClass sys_class = {
    "Sys",
    0,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,   JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool sys_print(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *s;
	char *message;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &s))
		return JS_FALSE;
	message = JS_EncodeString(cx, s);
	trap_Print(message);
	JS_free(cx, message);
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_TRUE;
}

static JSBool sys_error(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *s;
	char *message;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &s))
		return JS_FALSE;
	message = JS_EncodeString(cx, s);
	trap_Error(message);
	JS_free(cx, message);
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_TRUE;
}

static JSBool sys_milliseconds(JSContext *cx, unsigned argc, jsval *vp)
{
	jsval rval;
	JS_NewNumberValue(cx, trap_Milliseconds(), &rval);
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;
}

static JSBool sys_registershader(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *s;
	char *message;
	jsval rval;
	qhandle_t handle;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &s))
		return JS_FALSE;
	message = JS_EncodeString(cx, s);
	handle = trap_R_RegisterShader(message);
	JS_free(cx, message);
	JS_NewNumberValue(cx, handle, &rval);
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;	
}

static JSBool sys_registershadernomip(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *s;
	char *message;
	jsval rval;
	qhandle_t handle;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &s))
		return JS_FALSE;
	message = JS_EncodeString(cx, s);
	handle = trap_R_RegisterShaderNoMip(message);
	JS_free(cx, message);
	JS_NewNumberValue(cx, handle, &rval);
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;	
}

static JSBool sys_registermodel(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *s;
	char *message;
	jsval rval;
	qhandle_t handle;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &s))
		return JS_FALSE;
	message = JS_EncodeString(cx, s);
	handle = trap_R_RegisterModel(message);
	JS_free(cx, message);
	JS_NewNumberValue(cx, handle, &rval);
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;	
}

static JSBool sys_getglconfig(JSContext *cx, unsigned argc, jsval *vp)
{
	JSObject *res;
	jsval rval;
	glconfig_t cfg;
	jsval vidWidth, vidHeight;

	memset(&cfg,0,sizeof(glconfig_t));
	trap_GetGlconfig(&cfg);

	res = JS_NewObject(cx,NULL,NULL,NULL);
	if (!res)
	{
		JS_ReportError(cx, "Unable to create glConfig object");
		return JS_FALSE;
	}
	rval = OBJECT_TO_JSVAL(res);
	JS_NewNumberValue(cx, cfg.vidWidth, &vidWidth);
	JS_SetProperty(cx, res, "vidWidth", &vidWidth);
	JS_NewNumberValue(cx, cfg.vidHeight, &vidHeight);
	JS_SetProperty(cx, res, "vidHeight", &vidHeight);

	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;
}

static JSBool sys_drawstretchpic(JSContext *cx, unsigned argc, jsval *vp)
{
	double x, y, w, h, s1, t1, s2, t2;
	int hShader;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ddddddddi", &x, &y, &w, &h, &s1, &t1, &s2, &t2, &hShader))
		return JS_FALSE;

	//CG_Printf("DrawStretchPic(%f, %f, %f, %f, %f, %f, %f, %f, %d)\n", x, y, w, h, s1, t1, s2, t2, hShader);

	trap_R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hShader);
	return JS_TRUE;
}

static JSBool sys_setcolor(JSContext *cx, unsigned argc, jsval *vp)
{
	vec4_t color;
	double r, g, b, a;
	if (argc == 0)
	{
		trap_R_SetColor(NULL);
		return JS_TRUE;
	}
	else
	{
		if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &r, &g, &b, &a))
			return JS_FALSE;

		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;

		trap_R_SetColor(color);
		return JS_TRUE;
	}
}

static JSBool sys_clearscene(JSContext *cx, unsigned argc, jsval *vp)
{
	trap_R_ClearScene();
	return JS_TRUE;
}


void JS_Object_ParseRefEntity(JSContext *cx, JSObject *table, refEntity_t *r);

static JSBool sys_addrefentitytoscene(JSContext *cx, unsigned argc, jsval *vp)
{
	JSObject *obj;
	refEntity_t refent;
	memset(&refent, 0, sizeof(refEntity_t));
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &obj))
		return JS_FALSE;
	JS_Object_ParseRefEntity(cx, obj, &refent);
	trap_R_AddRefEntityToScene(&refent);
	return JS_TRUE;
}

void JS_Object_ParseRefDef(JSContext *cx, JSObject *table, refdef_t *r);

static JSBool sys_renderscene(JSContext *cx, unsigned argc, jsval *vp)
{
	JSObject *obj;
	refdef_t refdef;
	memset(&refdef, 0, sizeof(refdef_t));
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &obj))
		return JS_FALSE;
	JS_Object_ParseRefDef(cx, obj, &refdef);
	trap_R_RenderScene(&refdef);
	return JS_TRUE;
}

void JS_Object_SetPlayerState(JSContext *cx, JSObject *o, playerState_t ps);

static JSBool sys_getplayerstate(JSContext *cx, unsigned argc, jsval *vp)
{
	playerState_t ps = cg.snap->ps;
	JSObject *psobj = JS_NewObject(cx,NULL,NULL,NULL);
	jsval returnValue;
	
	if (!psobj)
	{
		JS_ReportError(cx, "Unabel to create playerState object");
		return JS_FALSE;
	}

	JS_Object_SetPlayerState(cx,psobj, ps);

	returnValue = OBJECT_TO_JSVAL(psobj);
	JS_SET_RVAL(cx, vp, returnValue);
	return JS_TRUE;
}

void JS_Object_SetItem(JSContext *cx, JSObject *o, gitem_t *item);

static JSBool sys_getitems(JSContext *cx, unsigned argc, jsval *vp)
{
	gitem_t	*it;
	JSObject *j;
	jsval val;
	int i;
	jsval returnValue;
	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	JS_AddObjectRoot(cx, &arr);
	
	for ( i=0, it = bg_itemlist + 1 ; it->classname ; it++,i++) {
		j = JS_NewObject(cx, NULL, NULL, NULL);
		JS_Object_SetItem(cx, j, it);
		val = OBJECT_TO_JSVAL(j);
		JS_SetElement(cx,arr,i,&val);
	}

	JS_RemoveObjectRoot(cx, &arr);
	returnValue = OBJECT_TO_JSVAL(arr);
	JS_SET_RVAL(cx, vp, returnValue);
	return JS_TRUE;
}

static JSFunctionSpec sys_static_methods[] = {
	JS_FS("Print", sys_print, 1, 0),
	JS_FS("Error", sys_error, 1, 0),
	JS_FS("Milliseconds", sys_milliseconds, 0, 0),

	JS_FS("RegisterShader", sys_registershader, 1, 0),
	JS_FS("RegisterShaderNoMip", sys_registershadernomip, 1, 0),
	JS_FS("RegisterModel", sys_registermodel, 1, 0),

	JS_FS("GetGLConfig", sys_getglconfig, 0, 0),

	JS_FS("DrawStretchPic", sys_drawstretchpic, 9, 0),
	JS_FS("SetColor", sys_setcolor, 4, 0),

	JS_FS("ClearScene", sys_clearscene, 0, 0),
	JS_FS("AddRefEntityToScene", sys_addrefentitytoscene, 1, 0),
	JS_FS("RenderScene", sys_renderscene, 1, 0),

	JS_FS("GetPlayerState", sys_getplayerstate, 0, 0),

	JS_FS("GetItems", sys_getitems, 0, 0),

	JS_FS_END
};

static JSConstDoubleSpec sys_constants[] = {
	{0,0,0,{0,0,0}}
};

void CG_JS_Sys_Cvar_Init(JSContext *ctx, JSObject *Sys);

void CG_JS_Sys_Init(JSContext *ctx, JSObject *global)
{
    JSObject *Sys;
	int len = strlen(GAME_VERSION);
	char *gv = (char *)JS_malloc(ctx, len+1);

    Sys = JS_DefineObject(ctx, global, "Sys", &sys_class, NULL, 0);
    if (!Sys)
        CG_Error("Failed to Define javascript Sys object\n");
    if (!JS_DefineFunctions(ctx, Sys, sys_static_methods))
        CG_Error("Failed to Define javascript Sys functions\n");
    if (!JS_DefineConstDoubles(ctx, Sys, sys_constants))
        CG_Error("Failed to Define javascript Sys constants\n");

	CG_JS_Sys_Cvar_Init(ctx, Sys);
}