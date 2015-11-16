#include "../qcommon/q_shared.h"
#include "cg_local.h"
#include <jsapi.h>


static JSClass vector3_class = {
	"Vec3", 0, 
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub, 
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

static JSClass vector4_class = {
	"Vec4", 0, 
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub, 
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

static JSBool vector3_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
	JSObject *returnObject;
	jsval returnValue;
	double x, y, z;
	jsval r, g, b;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ddd", &x, &y, &z))
		return JS_FALSE;
	returnObject = JS_NewObject(cx, &vector3_class, NULL, NULL);
	returnValue = OBJECT_TO_JSVAL(returnObject);

	JS_NewNumberValue(cx, x, &r);
	JS_NewNumberValue(cx, y, &g);
	JS_NewNumberValue(cx, z, &b);

	JS_SetProperty(cx, returnObject, "X", &r);
	JS_SetProperty(cx, returnObject, "Y", &g);
	JS_SetProperty(cx, returnObject, "Z", &b);

	JS_SET_RVAL(cx, vp, returnValue);
	return JS_TRUE;
}

static JSBool vector4_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
	JSObject *returnObject;
	jsval returnValue;
	double w, x, y, z;
	jsval r, g, b, a;
	if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &w, &x, &y, &z))
		return JS_FALSE;
	returnObject = JS_NewObject(cx, &vector4_class, NULL, NULL);
	returnValue = OBJECT_TO_JSVAL(returnObject);

	JS_NewNumberValue(cx, w, &r);
	JS_NewNumberValue(cx, x, &g);
	JS_NewNumberValue(cx, y, &b);
	JS_NewNumberValue(cx, z, &a);

	JS_SetProperty(cx, returnObject, "R", &r);
	JS_SetProperty(cx, returnObject, "G", &g);
	JS_SetProperty(cx, returnObject, "G", &b);
	JS_SetProperty(cx, returnObject, "A", &b);

	JS_SET_RVAL(cx, vp, returnValue);
	return JS_TRUE;
}

void CG_JS_Vector_Init(JSContext *ctx, JSObject *global)
{
	JS_InitClass(ctx, global, NULL, &vector3_class, vector3_constructor, 3, NULL, NULL, NULL, NULL);  
	JS_InitClass(ctx, global, NULL, &vector4_class, vector4_constructor, 4, NULL, NULL, NULL, NULL);  
}

JSBool JS_NewVector3Value(JSContext *cx, vec3_t v, jsval *rval)
{
	JSObject *obj;
	jsval a,b,c;
	if (!JS_NewNumberValue(cx,v[0],&a))
	{
		JS_ReportError(cx,"NewVector3_Value: Failed to get X\n");
		return JS_FALSE;
	}
	if (!JS_NewNumberValue(cx,v[1],&b))
	{
		JS_ReportError(cx,"NewVector3_Value: Failed to get Y\n");
		return JS_FALSE;
	}
	if (!JS_NewNumberValue(cx,v[2],&c))
	{
		JS_ReportError(cx,"NewVector3_Value: Failed to get Z\n");
		return JS_FALSE;
	}
	obj = JS_NewObject(cx, &vector3_class, NULL, NULL);
	*rval = OBJECT_TO_JSVAL(obj);
	JS_SetProperty(cx, obj, "X", &a);
	JS_SetProperty(cx, obj, "Y", &b);
	JS_SetProperty(cx, obj, "Z", &c);
	return JS_TRUE;
}

JSBool JS_NewVector4Value(JSContext *cx, vec4_t v, jsval *rval)
{
	JSObject *obj;
	jsval a,b,c,d;
	if (!JS_NewNumberValue(cx,v[0],&a))
	{
		JS_ReportError(cx,"NewVector4_Value: Failed to get R\n");
		return JS_FALSE;
	}
	if (!JS_NewNumberValue(cx,v[1],&b))
	{
		JS_ReportError(cx,"NewVector4_Value: Failed to get G\n");
		return JS_FALSE;
	}
	if (!JS_NewNumberValue(cx,v[2],&c))
	{
		JS_ReportError(cx,"NewVector4_Value: Failed to get B\n");
		return JS_FALSE;
	}
	if (!JS_NewNumberValue(cx,v[3],&d))
	{
		JS_ReportError(cx,"NewVector4_Value: Failed to get A\n");
		return JS_FALSE;
	}
	obj = JS_NewObject(cx, &vector4_class, NULL, NULL);
	*rval = OBJECT_TO_JSVAL(obj);
	JS_SetProperty(cx, obj, "R", &a);
	JS_SetProperty(cx, obj, "G", &b);
	JS_SetProperty(cx, obj, "B", &c);
	JS_SetProperty(cx, obj, "A", &d);
	return JS_TRUE;

}