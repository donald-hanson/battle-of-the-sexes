#include "../qcommon/q_shared.h"
#include "cg_local.h"
#include <jsapi.h>

static JSClass cvar_class = {
    "Cvar",
    0,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,   JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool cvar_getint(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *s;
	char *message;
	jsval rval;
	int result;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &s))
		return JS_FALSE;
	message = JS_EncodeString(cx, s);
	result = trap_Cvar_VariableIntegerValue(message);
	JS_free(cx, message);
	JS_NewNumberValue(cx, result, &rval);
	JS_SET_RVAL(cx, vp, rval);
	return JS_TRUE;	
}

static JSBool cvar_getstring(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *cvarNameString;
	char *cvarName;
	char cvarValue[MAX_STRING_CHARS];
	JSString *returnString;
	jsval returnValue;

	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &cvarNameString))
		return JS_FALSE;
	cvarName = JS_EncodeString(cx, cvarNameString);
	trap_Cvar_VariableStringBuffer(cvarName, cvarValue, sizeof(cvarValue));
	JS_free(cx, cvarName);

	returnString = JS_NewStringCopyN(cx, cvarValue, sizeof(cvarValue));
	returnValue = STRING_TO_JSVAL(returnString);

	JS_SET_RVAL(cx, vp, returnValue);
	return JS_FALSE;
}

static JSFunctionSpec cvar_static_methods[] = {
	JS_FS("GetInt", cvar_getint, 1, 0),
	JS_FS("GetString", cvar_getstring, 1, 0),
	JS_FS_END
};

void CG_JS_Sys_Cvar_Init(JSContext *ctx, JSObject *Sys)
{
	JSObject *cvar = JS_DefineObject(ctx, Sys, "Cvar", &cvar_class, NULL, 0);
    if (!cvar)
        CG_Error("Failed to Define javascript Cvar object\n");
    if (!JS_DefineFunctions(ctx, cvar, cvar_static_methods))
        CG_Error("Failed to Define javascript Cvar functions\n");
}
