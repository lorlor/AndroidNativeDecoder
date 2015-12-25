#include <jni.h>

jstring Java_com_example_myprj_MainActivity_fromNativeInter(JNIEnv* env,
															jobject thiz)
{
	return (*env)->NewStringUTF(env, "can c me");
}
