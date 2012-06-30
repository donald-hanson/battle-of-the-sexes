#include "../qcommon/q_shared.h"
#include "../renderer/tr_types.h"
#include <jsapi.h>

jsval JS_MakeString(JSContext *cx, char *v)
{
	return STRING_TO_JSVAL(JS_NewStringCopyZ(cx, v));
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
	a = OBJECT_TO_JSVAL(arr);
	JS_AddObjectRoot(cx, &arr);
	for (i=0;i<c;i++)
	{
		JS_NewNumberValue(cx,v[i],&z);
		JS_SetElement(cx,arr,i,&z);
	}
	JS_RemoveObjectRoot(cx, &arr);
	JS_SetProperty(cx,o,prop,&a);
}

void JS_Object_SetPropertyArrayString(JSContext *cx, JSObject *o, char *prop, char **v, int c)
{
	jsval z;
	jsval a;
	char *text;
	int i=0;
	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	a = OBJECT_TO_JSVAL(arr);
	JS_AddObjectRoot(cx, &arr);
	for (i=0;i<c;i++)
	{
		text = v[i];
		z = JS_MakeString(cx, text);
		JS_SetElement(cx,arr,i,&z);
	}
	JS_RemoveObjectRoot(cx, &arr);
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