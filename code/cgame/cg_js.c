#include "../qcommon/q_shared.h"
#include "cg_local.h"
#include <jsapi.h>

void CG_JS_GenericShutdown(void);

#define JS_RUNTIME_SPACE		8L * 1024L * 1024L
#define JS_STACK_CHUNK_SIZE		8192 * 4
#define MAX_JSFILE				32768

static JSRuntime *_jsruntime;
static JSContext *_jscontext;
static JSObject *_jsglobal;

/* The class of the global object. */  
static JSClass global_class = {  
    "global", JSCLASS_GLOBAL_FLAGS,  
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,  
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,  
    JSCLASS_NO_OPTIONAL_MEMBERS  
}; 

static JSBool global_load(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *s;
	char *file;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &s))
		return JS_FALSE;
	file = JS_EncodeString(cx, s);
	CG_JS_LoadFile(file);
	JS_free(cx, file);
	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return JS_TRUE;
}

static JSFunctionSpec global_functions[] = {
	JS_FS("Load", global_load, 0, 0),
	JS_FS_END
};

void CG_JS_ErrorReporter(JSContext *cx, const char *message,JSErrorReport *report)
{
	CG_Printf(S_COLOR_RED " Javascript Error: %s (%s) (%d) (%s)\n", message, report->linebuf, report->lineno, report->tokenptr);
}


void CG_JS_Sys_Init(JSContext *ctx, JSObject *global);
void CG_JS_Vector_Init(JSContext *ctx, JSObject *global);

void CG_JS_Init(void)
{
	_jsruntime = JS_NewRuntime(JS_RUNTIME_SPACE);
	if (_jsruntime == (JSRuntime *)NULL)
	{
		CG_Error("Failed to initialize Javscript Runtime\n");
		return;
	}

	CG_Printf("JS Runtime Space: %d\n",JS_RUNTIME_SPACE);

	_jscontext = JS_NewContext(_jsruntime, JS_STACK_CHUNK_SIZE);
	if (_jscontext == (JSContext *)NULL)
	{
		CG_JS_GenericShutdown();
		CG_Error("Failed to initialize Javascript Context\n");
		return;
	}

	JS_SetOptions(_jscontext, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT);
	JS_SetVersion(_jscontext, JSVERSION_LATEST);
	JS_SetErrorReporter(_jscontext, CG_JS_ErrorReporter);

	CG_Printf("JS Stack Chunk Size: %d\n",JS_STACK_CHUNK_SIZE);
	CG_Printf("JS Version: %s\n",JS_VersionToString(JS_GetVersion(_jscontext)));

	_jsglobal = JS_NewCompartmentAndGlobalObject(_jscontext, &global_class, NULL);
	if (_jsglobal == (JSObject *)NULL)
	{
		CG_JS_GenericShutdown();
		CG_Error("Failed to initialize Javascript Global Object\n");
		return;
	}

    if (!JS_InitStandardClasses(_jscontext, _jsglobal))
	{
		CG_JS_GenericShutdown();
		CG_Error("Failed to initialize Javascript Standard Classes\n");
		return;
	}

	JS_DefineFunctions(_jscontext, _jsglobal, global_functions);

	CG_JS_Sys_Init(_jscontext, _jsglobal);
	CG_JS_Vector_Init(_jscontext, _jsglobal);

	CG_Printf ("------- Javascript Initialize Completed -------\n");
}

void CG_JS_GenericShutdown(void)
{
	if (_jscontext != (JSContext *)NULL)
		JS_DestroyContext(_jscontext);

	if (_jsruntime != (JSRuntime *)NULL)
		JS_DestroyRuntime(_jsruntime);
}

void CG_JS_Shutdown(void)
{
	CG_Printf ("==== Shutdown Javascript ====\n");

	CG_JS_GenericShutdown();

	JS_ShutDown();
}


void CG_JS_LoadFile(char *filename)
{
	int				len;
	fileHandle_t	f;
	static char buf[MAX_JSFILE];
	jsval returnval;
	JSString *retstr = (JSString *)NULL;
	JSBool ran = JS_FALSE;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f )
	{
		CG_Printf(S_COLOR_RED "Failed to load javascript file: %s\n",filename);
		return;
	}

	if (len >= MAX_JSFILE)
	{
		CG_Printf(S_COLOR_RED "Failed to run javascript file: %s. File exceeds maximum allowed size: %d\n",filename, len);
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ran = JS_EvaluateScript(_jscontext, _jsglobal, buf, len, filename, 0, &returnval);

	if (ran == JS_FALSE)
		CG_Error("Failed to run javascript file: %s\n", filename);
	else
	{
		retstr = JS_ValueToString(_jscontext, returnval);
		CG_Printf("Ran javascript file: %s (%s)\n", filename, JS_EncodeString(_jscontext, retstr));
	}
}

void CG_JS_Eval(char *script)
{
	jsval returnval;
	JSBool ran = JS_FALSE;
	ran = JS_EvaluateScript(_jscontext, _jsglobal, script, strlen(script), NULL, 0, &returnval);
	if (ran == JS_FALSE)
		CG_Printf(S_COLOR_RED "Failed to run script: %s\n", script);
}

qboolean CG_JS_EvalToBool(char *script)
{
	jsval returnval;
	JSBool b = JS_FALSE;
	JSBool ran = JS_FALSE;
	ran = JS_EvaluateScript(_jscontext, _jsglobal, script, strlen(script), NULL, 0, &returnval);
	if (ran == JS_FALSE)
	{
		CG_Printf(S_COLOR_RED "Failed to run script: %s\n", script);	
		return qfalse;
	}

	if (JS_ValueToBoolean(_jscontext, returnval, &b) == JS_TRUE)
		return b == JS_TRUE ? qtrue : qfalse;
	else
	{
		CG_Printf(S_COLOR_RED "Unable to convert return value to a boolean %s\n", script);
		return qfalse;
	}
}