/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

#include "rhodes/JNIRhodes.h"
#include "ruby/ext/rho/rhoruby.h"

#include "rhodes/jni/com_rhomobile_rhodes_geolocation_GeoLocationImpl.h"

#include "rubyext/GeoLocation.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "GeoLocationJNI"

RHO_GLOBAL void JNICALL Java_com_rhomobile_rhodes_geolocation_GeoLocation_geoCallback
  (JNIEnv *, jclass)
{
    RAWTRACE("Call geo callback");
    rho_geo_callcallback();
}

RHO_GLOBAL void JNICALL Java_com_rhomobile_rhodes_geolocation_GeoLocation_geoCallbackError
  (JNIEnv *, jclass)
{
    RAWTRACE("Call geo callback error");
    rho_geo_callcallback_error();
}

RHO_GLOBAL void JNICALL Java_com_rhomobile_rhodes_geolocation_GeoLocation_geoCallbackStop
  (JNIEnv *, jclass)
{
    RAWTRACE("Call geo callback stop");
    rho_geo_callcallback_stop();
}

static std::string return_string;

RHO_GLOBAL const char* rho_geo_location_string()
{
	RAWTRACE("Call rho_geo_location_string");
	
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return "no class";
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "getLocationString", "()Ljava/lang/String;");
    if (!mid) return "no method";
    jstring jstr = (jstring) env->CallStaticObjectMethod(cls, mid);
    
    RAWTRACE("got jstring");
    
    if (jstr) {
    	return_string = rho_cast<std::string>(env, jstr);
    	RAWTRACE("got return_string");
    	env->DeleteLocalRef(jstr);
    	RAWTRACE(return_string.c_str());
    	return return_string.c_str();
    }
    
    return NULL;
    
    //char* buf = (char*) env->GetStringUTFChars(jstr,0);
    //VALUE result = rho_ruby_create_string(buf);
    //env->ReleaseStringUTFChars(jstr, buf);
	//env->DeleteLocalRef(jstr);
    //return buf;
    //return env->CallStaticObjectMethod(cls, mid);
}

RHO_GLOBAL double rho_geo_latitude()
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return 0;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "getLatitude", "()D");
    if (!mid) return 0;
    return env->CallStaticDoubleMethod(cls, mid);
}

RHO_GLOBAL double rho_geo_longitude()
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return 0;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "getLongitude", "()D");
    if (!mid) return 0;
    return env->CallStaticDoubleMethod(cls, mid);
}

RHO_GLOBAL double rho_geo_altitude()
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return 0;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "getAltitude", "()D");
    if (!mid) return 0;
    return env->CallStaticDoubleMethod(cls, mid);
}

RHO_GLOBAL float rho_geo_accuracy()
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return 0;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "getAccuracy", "()F");
    if (!mid) return 0;
   	return env->CallStaticFloatMethod(cls, mid);
}

RHO_GLOBAL int rho_geo_known_position()
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return 0;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "isKnownPosition", "()Z");
    if (!mid) return 0;
    return env->CallStaticBooleanMethod(cls, mid);
}

RHO_GLOBAL void rho_geoimpl_settimeout(int nTimeoutSec)
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "setTimeout", "(I)V");
    if (!mid) return;
    return env->CallStaticVoidMethod(cls, mid, nTimeoutSec);
}

RHO_GLOBAL int rho_geo_is_available()
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return 0;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "isAvailable", "()Z");
    if (!mid) return 0;
    return env->CallStaticBooleanMethod(cls, mid);
}

RHO_GLOBAL void rho_geoimpl_turngpsoff()
{
    JNIEnv *env = jnienv();
    static jclass cls = getJNIClass(RHODES_JAVA_CLASS_GEO_LOCATION);
    if (!cls) return;
    static jmethodID mid = getJNIClassStaticMethod(env, cls, "stop", "()V");
    if (!mid) return;

    return env->CallStaticVoidMethod(cls, mid);
}

