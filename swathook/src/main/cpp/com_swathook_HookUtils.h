/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_swathook_HookUtils */

#ifndef _Included_com_swathook_HookUtils
#define _Included_com_swathook_HookUtils
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_swathook_HookUtils
 * Method:    stringFromJNI
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_swathook_HookUtils_stringFromJNI
  (JNIEnv *, jobject);

/*
 * Class:     com_swathook_HookUtils
 * Method:    hookMethodNativeOr
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL Java_com_swathook_HookUtils_hookMethodNativeOr
  (JNIEnv *, jclass, jstring, jstring, jstring, jboolean);

/*
 * Class:     com_swathook_HookUtils
 * Method:    hookMethodNative
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Lcom/swathook/HookCallBack;)I
 */
JNIEXPORT jint JNICALL Java_com_swathook_HookUtils_hookMethodNative
  (JNIEnv *, jclass, jstring, jstring, jstring, jobject);

#ifdef __cplusplus
}
#endif
#endif