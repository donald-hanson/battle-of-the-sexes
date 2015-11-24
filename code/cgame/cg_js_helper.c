#include "cg_local.h"
#include <jsapi.h>

jsval JS_MakeString(JSContext *cx, char *v)
{
	JSString *s = JS_NewStringCopyZ(cx, v);
	jsval returnValue = STRING_TO_JSVAL(s);
	return returnValue;
}

void JS_Object_SetPropertyBit(JSContext *cx, JSObject *o, char *prop, int v)
{
	jsval bit = JSVAL_VOID;
	if (v == 1)
		bit = JSVAL_TRUE;
	else
		bit = JSVAL_FALSE;
	JS_SetProperty(cx, o, prop, &bit);
}

void JS_Object_SetPropertyString(JSContext *cx, JSObject *o, char *prop, char *v)
{
	jsval s = JS_MakeString(cx, v);
	JS_SetProperty(cx, o, prop, &s);
}

void JS_Object_SetPropertyByte(JSContext *cx, JSObject *o, char *prop, byte v)
{
	jsval d;
	if (!JS_NewNumberValue(cx,v,&d))
		return;
	JS_SetProperty(cx, o, prop, &d);	
}

void JS_Object_GetPropertyByte(JSContext *cx, JSObject *o, char *prop, byte *v)
{
	jsval j;
	if (JS_GetProperty(cx,o,prop,&j))
	{
		if (JSVAL_IS_VOID(j))
			return;
		if (JSVAL_IS_NULL(j))
			return;
		if (!JSVAL_IS_INT(j))
			return;
		*v = JSVAL_TO_INT(j);
	}
}

void JS_Object_SetPropertyInt(JSContext *cx, JSObject *o, char *prop, int v)
{
	jsval d;
	if (!JS_NewNumberValue(cx,v,&d))
		return;
	JS_SetProperty(cx, o, prop, &d);	
}

void JS_Object_GetPropertyInt(JSContext *cx, JSObject *o, char *prop, int *v)
{
	jsval j;
	if (JS_GetProperty(cx,o,prop,&j))
	{
		if (JSVAL_IS_VOID(j))
			return;
		if (JSVAL_IS_NULL(j))
			return;
		if (!JSVAL_IS_INT(j))
			return;
		*v = JSVAL_TO_INT(j);
	}
}

void JS_Object_SetPropertyFloat(JSContext *cx, JSObject *o, char *prop, float v)
{
	jsval d;
	if (!JS_NewNumberValue(cx,v,&d))
		return;
	JS_SetProperty(cx, o, prop, &d);
}

void JS_Object_GetPropertyFloat(JSContext *cx, JSObject *o, char *prop, float *v)
{
	jsval j;
	if (JS_GetProperty(cx,o,prop,&j))
	{
		if (JSVAL_IS_VOID(j))
			return;
		if (JSVAL_IS_NULL(j))
			return;

		if (JSVAL_IS_DOUBLE(j))
			*v = JSVAL_TO_DOUBLE(j);
		else if (JSVAL_IS_INT(j))
			*v = JSVAL_TO_INT(j);
		else
			return;
	}
}

JSBool JS_NewVector3Value(JSContext *cx, vec3_t v, jsval *rval);
JSBool JS_NewVector4Value(JSContext *cx, vec4_t v, jsval *rval);

void JS_Object_SetPropertyVector(JSContext *cx, JSObject *o, char *prop, vec3_t v)
{
	jsval d;
	if (!JS_NewVector3Value(cx, v, &d))
		return;
	JS_SetProperty(cx, o, prop, &d);
}

void JS_Object_GetPropertyVector(JSContext *cx, JSObject *o, char *prop, vec3_t *v)
{
	jsval j;
	JSObject *p;
	vec3_t r;

	memcpy(&r,v,sizeof(vec3_t));

	if (JS_GetProperty(cx,o,prop,&j))
	{
		if (JSVAL_IS_VOID(j))
			return;
		if (JSVAL_IS_NULL(j))
			return;
		if (!JSVAL_IS_OBJECT(j))
			return;
		p = JSVAL_TO_OBJECT(j);

		JS_Object_GetPropertyFloat(cx,p,"X",&(r[0]));
		JS_Object_GetPropertyFloat(cx,p,"Y",&(r[1]));
		JS_Object_GetPropertyFloat(cx,p,"Z",&(r[2]));

		memcpy(v,&r,sizeof(vec3_t));
	}
}

void JS_Object_GetPropertyObject(JSContext *cx, JSObject *o, char *prop, JSObject **v)
{
	jsval j;
	if (JS_GetProperty(cx,o,prop,&j))
	{
		if (JSVAL_IS_VOID(j))
			return;
		if (JSVAL_IS_NULL(j))
			return;
		if (!JSVAL_IS_OBJECT(j))
			return;
		*v = JSVAL_TO_OBJECT(j);
	}
}

void JS_Object_SetPropertyObject(JSContext *cx, JSObject *o, char *prop, JSObject *p)
{
	jsval v;
	v = OBJECT_TO_JSVAL(p);
	JS_SetProperty(cx,o,prop, &v);
}

void JS_Object_SetPropertyArray(JSContext *cx, JSObject *o, char *prop, int *v, int c)
{
	jsval z;
	jsval a;
	int i=0;
	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	for (i=0;i<c;i++)
	{
		JS_NewNumberValue(cx,v[i],&z);
		JS_SetElement(cx,arr,i,&z);
	}
	a = OBJECT_TO_JSVAL(arr);
	JS_SetProperty(cx,o,prop, &a);
}

void JS_Object_SetPropertyArrayString(JSContext *cx, JSObject *o, char *prop, char **v, int c)
{
	jsval z;
	jsval a;
	char *text;
	int i=0;
	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	for (i=0;i<c;i++)
	{
		text = v[i];
		z = JS_MakeString(cx, text);
		JS_SetElement(cx,arr,i,&z);
	}
	a = OBJECT_TO_JSVAL(arr);
	JS_SetProperty(cx,o,prop,&a);
}

void JS_Object_ParseRefEntity(JSContext *cx, JSObject *table, refEntity_t *r)
{
	JSObject *axisobj;
	JSObject *shaderObject;

#define _U_INT(n,p) JS_Object_GetPropertyInt(cx,table,##n,(int *)&r->##p);
#define _U_VEC(n,p)	JS_Object_GetPropertyVector(cx,table,##n,&r->##p);
#define _U_FLT(n,p) JS_Object_GetPropertyFloat(cx,table,##n,&r->##p);

	_U_INT("reType", reType);
	_U_INT("renderfx", renderfx);
	_U_INT("hModel", hModel);

	//most recent data
	_U_VEC("lightingOrigin", lightingOrigin);
	_U_FLT("shadowPlane", shadowPlane);

	JS_Object_GetPropertyObject(cx,table,"axis",&axisobj);
	if (axisobj != (JSObject *)NULL)
	{
		JS_Object_GetPropertyVector(cx,axisobj,"X", &r->axis[0]);
		JS_Object_GetPropertyVector(cx,axisobj,"Y", &r->axis[1]);
		JS_Object_GetPropertyVector(cx,axisobj,"Z", &r->axis[2]);
	}
	_U_INT("nonNormalizedAxes", nonNormalizedAxes);
	_U_VEC("origin", origin);	//Hack?
	_U_INT("frame", frame);

	//previous data for frame interpolation
	_U_VEC("oldorigin", oldorigin);	//Hack?
	_U_INT("oldframe", oldframe);
	_U_FLT("backlerp", backlerp);

	//texturing
	_U_INT("skinNum", skinNum);
	_U_INT("customSkin", customSkin);
	_U_INT("customShader", customShader);

	//misc
	JS_Object_GetPropertyObject(cx, table, "shader", &shaderObject);
	if (shaderObject != (JSObject *)NULL)
	{
		JS_Object_GetPropertyByte(cx, shaderObject, "R", &r->shaderRGBA[0]);
		JS_Object_GetPropertyByte(cx, shaderObject, "G", &r->shaderRGBA[1]);
		JS_Object_GetPropertyByte(cx, shaderObject, "B", &r->shaderRGBA[2]);
		JS_Object_GetPropertyByte(cx, shaderObject, "A", &r->shaderRGBA[3]);
		JS_Object_GetPropertyFloat(cx, shaderObject, "X", &r->shaderTexCoord[0]);
		JS_Object_GetPropertyFloat(cx, shaderObject, "Y", &r->shaderTexCoord[1]);
		JS_Object_GetPropertyFloat(cx, shaderObject, "Time", &r->shaderTime);
	}

	//Extra Sprite Information
	_U_FLT("radius", radius);
	_U_FLT("rotation", rotation);


#undef _U_INT
#undef _U_VEC
#undef _U_FLT
}


void JS_Object_ParseRefDef(JSContext *cx, JSObject *table, refdef_t *r)
{
	JSObject *viewaxisobj;

#define _U_INT(n,p) JS_Object_GetPropertyInt(cx,table,##n,&r->##p);
#define _U_VEC(n,p)	JS_Object_GetPropertyVector(cx,table,##n,&r->##p);
#define _U_FLT(n,p) JS_Object_GetPropertyFloat(cx,table,##n,&r->##p);

	//THIS SHOULD MIRROR THE refdef_t STRUCT!!!
	_U_INT("x", x);
	_U_INT("y", y);
	_U_INT("width", width);
	_U_INT("height", height);
	_U_FLT("fov_x", fov_x);
	_U_FLT("fov_y", fov_y);
	_U_VEC("vieworg", vieworg);
	_U_INT("time",time);
	_U_INT("rdflags",rdflags);

	JS_Object_GetPropertyObject(cx,table,"viewaxis",&viewaxisobj);
	if (viewaxisobj != (JSObject *)NULL)
	{
		JS_Object_GetPropertyVector(cx,viewaxisobj,"X", &r->viewaxis[0]);
		JS_Object_GetPropertyVector(cx,viewaxisobj,"Y", &r->viewaxis[1]);
		JS_Object_GetPropertyVector(cx,viewaxisobj,"Z", &r->viewaxis[2]);
	}

#undef _U_INT
#undef _U_VEC
#undef _U_FLT
}

void JS_Object_SetPlayerState(JSContext *cx, JSObject *o, playerState_t ps)
{

#define _INT(x,y) JS_Object_SetPropertyInt(cx,o,##y,##x);
#define _VEC(x,y) JS_Object_SetPropertyVector(cx,o,##y,##x);
#define _FLT(x,y) JS_Object_SetPropertyFloat(cx,o,##y,##x);
#define _ARR_INT(v,c,n) JS_Object_SetPropertyArray(cx,o,##n,##v,##c);

	_ARR_INT(ps.stats, MAX_STATS, "stats");
	_ARR_INT(ps.persistant, MAX_PERSISTANT, "persistant");
	_ARR_INT(ps.powerups, MAX_POWERUPS, "powerups");
	_ARR_INT(ps.ammo, MAX_WEAPONS, "ammo");
	_ARR_INT(ps.maxammo, MAX_WEAPONS, "maxammo");

#undef _INT
#undef _VEC
#undef _FLT
#undef _ARR_INT
}

void JS_Object_SetItem(JSContext *cx, JSObject *o, gitem_t *item)
{

#define _INT(x,y) JS_Object_SetPropertyInt(cx,o,##y,##x);
#define _STR(x,y) JS_Object_SetPropertyString(cx,o,##y,##x);
#define _ARR_STR(v,c,n) JS_Object_SetPropertyArrayString(cx,o,##n,##v,##c)

	_STR(item->classname, "classname");
	_STR(item->pickup_sound, "pickupSound");
	_ARR_STR(item->world_model, MAX_ITEM_MODELS, "worldModel");
	_STR(item->icon, "icon");
	_STR(item->pickup_name, "pickupName");
	_INT(item->quantity, "quantity");
	_INT(item->giType, "giType");
	_INT(item->giTag, "giTag");
	_STR(item->precaches, "precaches");
	_STR(item->sounds, "sounds");

#undef _INT
#undef _STR
#undef _ARR_STR
}

#define MAX_WRAPPERS 32
jsWrapper_t js_wrappers[MAX_WRAPPERS];

void JS_Wrapper_SetPropertyInt(jsWrapper_t *wrapper, char *propertyName, int value)
{
	JS_Object_SetPropertyInt((JSContext *)wrapper->jsContext, (JSObject *)wrapper->jsObject, propertyName, value);
}

void JS_Wrapper_SetPropertyString(jsWrapper_t *wrapper, char *propertyName, char *value)
{
	JS_Object_SetPropertyString((JSContext *)wrapper->jsContext, (JSObject *)wrapper->jsObject, propertyName, value);
}

void JS_Wrapper_SetPropertyBit(jsWrapper_t *wrapper, char *propertyName, int value)
{
	JS_Object_SetPropertyBit((JSContext *)wrapper->jsContext, (JSObject *)wrapper->jsObject, propertyName, value);	
}

void JS_Wrapper_SetPropertyFloat(jsWrapper_t *wrapper, char *propertyName, float value)
{
	JS_Object_SetPropertyFloat((JSContext *)wrapper->jsContext, (JSObject *)wrapper->jsObject, propertyName, value);
}

void JS_Wrapper_SetPropertyByte(jsWrapper_t *wrapper, char *propertyName, byte value)
{
	JS_Object_SetPropertyByte((JSContext *)wrapper->jsContext, (JSObject *)wrapper->jsObject, propertyName, value);
}

void JS_Wrapper_SetPropertyArray(jsWrapper_t *wrapper, char *propertyName, int *values, int count) {
	JS_Object_SetPropertyArray((JSContext *)wrapper->jsContext, (JSObject *)wrapper->jsObject, propertyName, values, count);
}

jsWrapper_t *JS_FindOpenWrapper(JSContext *jsContext, JSObject *jsObject, jsWrapper_t *parent);

jsWrapper_t *JS_Wrapper_NewObject(jsWrapper_t *wrapper)
{
	JSContext *cx = (JSContext *)wrapper->jsContext;
	JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);
	return JS_FindOpenWrapper(cx, obj, wrapper);
}

void JS_Wrapper_AddObjects(jsWrapper_t *wrapper, char *propertyName, jsWrapper_t **children, int length)
{
	jsval z;
	jsval a;
	int i=0;
	JSContext *cx = (JSContext *)wrapper->jsContext;
	JSObject *o = (JSObject *)wrapper->jsObject;
	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	for (i=0;i<length;i++)
	{
		z = OBJECT_TO_JSVAL((JSObject *)children[i]->jsObject);
		JS_SetElement(cx,arr,i,&z);
	}
	a = OBJECT_TO_JSVAL(arr);
	JS_SetProperty(cx, o, propertyName, &a);

}

void JS_PrepareFunctionPointers(jsWrapper_t *js_wrapper)
{
	js_wrapper->setPropertyInt = JS_Wrapper_SetPropertyInt;
	js_wrapper->setPropertyString = JS_Wrapper_SetPropertyString;
	js_wrapper->setPropertyBit = JS_Wrapper_SetPropertyBit;
	js_wrapper->setPropertyFloat = JS_Wrapper_SetPropertyFloat;
	js_wrapper->setPropertyByte = JS_Wrapper_SetPropertyByte;
	js_wrapper->setPropertyArray = JS_Wrapper_SetPropertyArray;
	js_wrapper->newObject = JS_Wrapper_NewObject;
	js_wrapper->addObjects = JS_Wrapper_AddObjects;
}

jsWrapper_t *JS_FindOpenWrapper(JSContext *jsContext, JSObject *jsObject, jsWrapper_t *parent)
{
	jsWrapper_t *wrapper;
	int i=0;
	for (i=0;i<MAX_WRAPPERS;i++)
	{
		wrapper = &js_wrappers[i];
		if (!wrapper->inUse)
		{
			wrapper->inUse = qtrue;
			wrapper->jsContext = jsContext;
			wrapper->jsObject = jsObject;
			wrapper->parent = parent;
			JS_PrepareFunctionPointers(wrapper);
			return wrapper;
		}
	}
	return (jsWrapper_t *)NULL;
}

jsWrapper_t *JS_MakeWrapper(JSContext *jsContext, JSObject *jsObject)
{
	return JS_FindOpenWrapper(jsContext, jsObject, (jsWrapper_t *)NULL);
}

void JS_FreeWrapper(jsWrapper_t *wrapper)
{
	int i;
	jsWrapper_t *child;
	wrapper->inUse = qfalse;
	wrapper->jsContext = (void *)NULL;
	wrapper->jsObject = (void *)NULL;
	for (i=0;i<MAX_WRAPPERS;i++)
	{
		child = &js_wrappers[i];
		if (child && child != wrapper && child->inUse && child->parent == wrapper)
			JS_FreeWrapper(child);
	}
}