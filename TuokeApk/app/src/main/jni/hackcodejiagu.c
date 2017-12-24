#include "org_hackcode_ProxyApplication.h"
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
//#define LOG_TAG "demo"
//#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define JNIREG_CLASS "org/hackcode/ProxyApplication"

jbyteArray decrypt(JNIEnv *env, jbyteArray srcdata){

	jbyte* byteArray = (*env)->GetByteArrayElements(env, srcdata, JNI_FALSE);
	jsize len = (*env)->GetArrayLength(env, srcdata);

	jbyte buffer[len];
	jbyteArray decodedArray = (*env)->NewByteArray(env, len);

	jsize i;
	for(i = 0; i < len; i++){
		buffer[i] = ~byteArray[i];
	}
	(*env)->SetByteArrayRegion(env, decodedArray, 0, len, buffer);

	(*env)->ReleaseByteArrayElements(env, srcdata, byteArray, 0);
	return decodedArray;
}


jbyteArray readClassesDexFromApk(JNIEnv *env, jobject obj){
	//LOGI("===============readClassesDexFromApk start===============");
	//////////////////ByteArrayOutputStream dexByteArrayOutputStream = new ByteArrayOutputStream();
	jclass byteArrayOutputStreamClass = (*env)->FindClass(env, "java/io/ByteArrayOutputStream");
	jmethodID byteArrayOutputStreamMethodID = (*env)->GetMethodID(env, byteArrayOutputStreamClass, "<init>", "()V");
	jmethodID writeMethodID = (*env)->GetMethodID(env, byteArrayOutputStreamClass, "write", "([BII)V"); //ByteArrayOutputStream.write(byte[] buffer, int offset, int len)
	jmethodID toByteArrayMethodID = (*env)->GetMethodID(env, byteArrayOutputStreamClass, "toByteArray", "()[B"); //ByteArrayOutputStream.toByteArray()
	jobject byteArrayOutputStreamObj = (*env)->NewObject(env, byteArrayOutputStreamClass, byteArrayOutputStreamMethodID);

	//////////////////ZipInputStream localZipInputStream = new ZipInputStream(new BufferedInputStream(new FileInputStream( this.getApplicationInfo().sourceDir)));
	jclass applicationClass = (*env)->GetObjectClass(env, obj);
	jmethodID getApplicationInfoMethodID = (*env)->GetMethodID(env, applicationClass, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
	jobject applicationInfoObj = (*env)->CallObjectMethod(env, obj, getApplicationInfoMethodID); //获取ApplicationInfo对象

	jclass applicationInfoClass =(*env)->GetObjectClass(env, applicationInfoObj);
	jfieldID sourceDirFieldID = (*env)->GetFieldID(env, applicationInfoClass, "sourceDir", "Ljava/lang/String;");
	jstring sourceDirString = (jstring)(*env)->GetObjectField(env, applicationInfoObj, sourceDirFieldID);
	const char* str = (*env)->GetStringUTFChars(env, sourceDirString, 0);

	jclass fileInputStreamClass = (*env)->FindClass(env, "java/io/FileInputStream");
	jmethodID fileInputStreamMethodID = (*env)->GetMethodID(env, fileInputStreamClass, "<init>", "(Ljava/lang/String;)V");
	jobject fileInputStreamObj = (*env)->NewObject(env, fileInputStreamClass, fileInputStreamMethodID, sourceDirString);

	jclass bufferedInputStreamClass = (*env)->FindClass(env, "java/io/BufferedInputStream");
	jmethodID bufferedInputStreamMethodId = (*env)->GetMethodID(env, bufferedInputStreamClass, "<init>", "(Ljava/io/InputStream;)V");
	jobject bufferedInputStreamObj = (*env)->NewObject(env, bufferedInputStreamClass, bufferedInputStreamMethodId, fileInputStreamObj);

	jclass zipInputStreamClass = (*env)->FindClass(env, "java/util/zip/ZipInputStream");
	jmethodID zipInputStreamMethodID = (*env)->GetMethodID(env, zipInputStreamClass, "<init>", "(Ljava/io/InputStream;)V");
	jobject zipInputStreamObj = (*env)->NewObject(env, zipInputStreamClass, zipInputStreamMethodID, bufferedInputStreamObj);

	jmethodID closeMethodID = (*env)->GetMethodID(env, zipInputStreamClass, "close", "()V"); //InputStream.close()
	jmethodID readMethodID = (*env)->GetMethodID(env, zipInputStreamClass, "read", "([B)I"); //InputStream.read(byte[] buffer)
	jmethodID getNextEntryMethodID = (*env)->GetMethodID(env, zipInputStreamClass, "getNextEntry", "()Ljava/util/zip/ZipEntry;");//ZipInputStream.getNextEntry()
	jmethodID closeEntryMethodID = (*env)->GetMethodID(env, zipInputStreamClass, "closeEntry", "()V");//ZipInputStream.closeEntry()

	jclass zipEntryClass = (*env)->FindClass(env, "java/util/zip/ZipEntry");
	jmethodID getNameMethodID = (*env)->GetMethodID(env, zipEntryClass, "getName", "()Ljava/lang/String;");
	while(1){
		jobject zipEntryObj = (*env)->CallObjectMethod(env, zipInputStreamObj, getNextEntryMethodID);

		if(zipEntryObj == NULL){
			(*env)->CallVoidMethod(env, zipInputStreamObj, closeMethodID);
			break;
		}

		jstring entryName = (jstring)(*env)->CallObjectMethod(env, zipEntryObj, getNameMethodID);
		const char* entryNameStr = (*env)->GetStringUTFChars(env, entryName, 0);
		if(strcmp(entryNameStr, "classes.dex") == 0){
			//LOGI("Find classes.dex!!!!!!!!!");
			jbyteArray buffer = (*env)->NewByteArray(env, 1024);
			while(1){
				int ret = (*env)->CallIntMethod(env, zipInputStreamObj, readMethodID, buffer);
				if(-1 == ret){
					break;
				}
				(*env)->CallVoidMethod(env, byteArrayOutputStreamObj, writeMethodID, buffer, 0, ret);
			}
			(*env)->ReleaseStringUTFChars(env, entryName, entryNameStr);
			(*env)->CallVoidMethod(env, zipInputStreamObj, closeEntryMethodID);
			break;
		}
		(*env)->ReleaseStringUTFChars(env, entryName, entryNameStr);
		(*env)->DeleteLocalRef(env, entryName); //GetStringUTFChars需要ReleaseStringUTFChars，DeleteLocalRef释放局部引用计数
		(*env)->DeleteLocalRef(env, zipEntryObj);
		(*env)->CallVoidMethod(env, zipInputStreamObj, closeEntryMethodID);
	}
	(*env)->CallVoidMethod(env, zipInputStreamObj, closeMethodID);
	(*env)->ReleaseStringUTFChars(env, sourceDirString, str);

	jbyteArray retArray = (jbyteArray)(*env)->CallObjectMethod(env, byteArrayOutputStreamObj, toByteArrayMethodID);
	//LOGI("===============readClassesDexFromApk end===============");

	return retArray;
}


void extractTargetZipFileFromDex(JNIEnv *env, jobject obj, jbyteArray classesDexData, jstring targetFilename){

	//LOGI("===============extractTargetZipFileFromDex start===============");

	jsize dexLen = (*env)->GetArrayLength(env, classesDexData);
	jbyteArray targetZipLenArray = (*env)->NewByteArray(env, 4);

	jclass systemClass = (*env)->FindClass(env, "java/lang/System");
	jmethodID arrayCopyMethodID = (*env)->GetStaticMethodID(env, systemClass, "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V");//arraycopy(Object src, int srcPos, Object dst, int dstPos, int length)
	(*env)->CallStaticVoidMethod(env, systemClass, arrayCopyMethodID, classesDexData, dexLen-4, targetZipLenArray, 0, 4);

	jclass baisClass = (*env)->FindClass(env, "java/io/ByteArrayInputStream");
	jmethodID baisMethodID = (*env)->GetMethodID(env, baisClass, "<init>", "([B)V"); //ByteArrayInputStream(byte[] buf)
	jobject baisObj = (*env)->NewObject(env, baisClass, baisMethodID, targetZipLenArray);

	jclass dataInputStreamClass = (*env)->FindClass(env, "java/io/DataInputStream");
	jmethodID dataInputStreamMethodID = (*env)->GetMethodID(env, dataInputStreamClass, "<init>", "(Ljava/io/InputStream;)V");
	jmethodID readIntMethodID = (*env)->GetMethodID(env, dataInputStreamClass, "readInt", "()I");
	jobject dataInputStreamObj =(*env)->NewObject(env, dataInputStreamClass, dataInputStreamMethodID, baisObj);

	int targetZipLen = (*env)->CallIntMethod(env, dataInputStreamObj, readIntMethodID);

	//LOGI("targetZipLen===>%d", targetZipLen);

	jbyteArray targetZipData = (*env)->NewByteArray(env, targetZipLen);
	(*env)->CallStaticVoidMethod(env, systemClass, arrayCopyMethodID, classesDexData, dexLen-targetZipLen-4, targetZipData, 0, targetZipLen);

	//解密
	jbyteArray decodedTargetZipData = decrypt(env, targetZipData);

	jclass fileClass = (*env)->FindClass(env, "java/io/File");
	jmethodID fileMethodID = (*env)->GetMethodID(env, fileClass, "<init>", "(Ljava/lang/String;)V");
	jobject fileObj = (*env)->NewObject(env, fileClass, fileMethodID, targetFilename);

	//保存解密后的文件
	jclass fileOutputStreamClass = (*env)->FindClass(env, "java/io/FileOutputStream");
	jmethodID fileOutputStreamMethodID = (*env)->GetMethodID(env, fileOutputStreamClass, "<init>", "(Ljava/io/File;)V");
	jmethodID fos_write_methodID =(*env)->GetMethodID(env, fileOutputStreamClass, "write", "([B)V");
	jmethodID fos_flush_methodID =(*env)->GetMethodID(env, fileOutputStreamClass, "flush", "()V");
	jmethodID fos_close_methodID =(*env)->GetMethodID(env, fileOutputStreamClass, "close", "()V");

	jobject fileOutputStreamObj = (*env)->NewObject(env, fileOutputStreamClass, fileOutputStreamMethodID, fileObj);
	(*env)->CallVoidMethod(env, fileOutputStreamObj, fos_write_methodID, decodedTargetZipData);
	(*env)->CallVoidMethod(env, fileOutputStreamObj, fos_flush_methodID);
	(*env)->CallVoidMethod(env, fileOutputStreamObj, fos_close_methodID);

	//LOGI("===============extractTargetZipFileFromDex end===============");

}

void run(JNIEnv *env, jobject obj, jstring dexPath, jstring optimizedDirectory, jstring libraryPath, jobject parent, jobject loadedApk){
	////run(targetFilename, odexPath, "/data/data/" + packageName + "/lib", base.getClassLoader().getParent(), wr.get());
	jclass dexClassLoaderClass =(*env)->FindClass(env, "dalvik/system/DexClassLoader");
	jmethodID dexClassLoaderMethodID = (*env)->GetMethodID(env, dexClassLoaderClass, "<init>","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
	jobject dlLoaderObj = (*env)->NewObject(env, dexClassLoaderClass, dexClassLoaderMethodID, dexPath, optimizedDirectory, libraryPath, parent);

	jclass reflectUtilsClass = (*env)->FindClass(env, "org/hackcode/ReflectUtils");
	jmethodID setFieldObjectMethodId = (*env)->GetStaticMethodID(env, reflectUtilsClass, "setFieldObject", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/Object;Ljava/lang/Object;)V");

	jstring para1 = (*env)->NewStringUTF(env, "android.app.LoadedApk");
	jstring para2 = (*env)->NewStringUTF(env, "mClassLoader");
	(*env)->CallStaticVoidMethod(env, reflectUtilsClass, setFieldObjectMethodId, para1, para2, loadedApk, dlLoaderObj);
}

JNIEXPORT void JNICALL native_start
  (JNIEnv *env, jobject obj, jobject application, jstring libraryPath, jobject parent, jobject loadedApk){
	/////////File odex = this.getDir("assets", MODE_PRIVATE);
	jclass contextWrapperClass = (*env)->FindClass(env, "android/content/ContextWrapper");
	jmethodID getDirMethodID = (*env)->GetMethodID(env, contextWrapperClass, "getDir", "(Ljava/lang/String;I)Ljava/io/File;");
	jstring path_name = (*env)->NewStringUTF(env, "assets");

	jclass contextClass = (*env)->FindClass(env, "android/content/Context");
	jfieldID fid = (*env)->GetStaticFieldID(env, contextClass, "MODE_PRIVATE", "I");
	jint i = (*env)->GetStaticIntField(env, contextClass, fid);
	jobject fileObj = (*env)->CallObjectMethod(env, application, getDirMethodID, path_name, i);

	////////String odexPath = odex.getAbsolutePath();
	jclass fileClass = (*env)->FindClass(env, "java/io/File");
	jmethodID getAbsolutePath_methodID = (*env)->GetMethodID(env, fileClass, "getAbsolutePath", "()Ljava/lang/String;");
	jstring odexPath = (jstring)(*env)->CallObjectMethod(env, fileObj, getAbsolutePath_methodID);

	///////String targetFilename = odexPath + "/TargetApk.zip";
	jclass StringBufferClass = (*env)->FindClass(env, "java/lang/StringBuffer");
	jmethodID initStringBufferMethod =(*env)->GetMethodID(env, StringBufferClass, "<init>","()V");
	jobject stringBufferObj = (*env)->NewObject(env, StringBufferClass,initStringBufferMethod);
	jmethodID append_methodID =(*env)->GetMethodID(env, StringBufferClass, "append","(Ljava/lang/String;)Ljava/lang/StringBuffer;");

	(*env)->CallObjectMethod(env, stringBufferObj, append_methodID, odexPath);
	jstring zip_str = (*env)->NewStringUTF(env, "/TargetApk.zip");
	(*env)->CallObjectMethod(env, stringBufferObj, append_methodID, zip_str);

	jmethodID toString_methodID = (*env)->GetMethodID(env, StringBufferClass, "toString","()Ljava/lang/String;");
	jstring targetFilename = (jstring)(*env)->CallObjectMethod(env, stringBufferObj, toString_methodID);

	//////////////////////File targetApkZipFile = new File(targetFilename);
	jmethodID initFile_methodID = (*env)->GetMethodID(env, fileClass, "<init>", "(Ljava/lang/String;)V");
	jobject targetApkZipFileObj = (*env)->NewObject(env, fileClass, initFile_methodID, targetFilename);

	//////////////////////targetApkZipFile.createNewFile();
	jmethodID createNewFile_methodID = (*env)->GetMethodID(env, fileClass, "createNewFile", "()Z");
	(*env)->CallBooleanMethod(env, targetApkZipFileObj,createNewFile_methodID);

	//TODO 解密
	jbyteArray classesDexData = readClassesDexFromApk(env, obj);
	extractTargetZipFileFromDex(env, obj, classesDexData, targetFilename);

	run(env, obj, targetFilename, odexPath, libraryPath, parent, loadedApk);
	
	/*
	//删除/data/data/{packagename}/assets/目录下的缓存文件TargetApk.zip和TargetApk.dex两个文件
	jobject stringBufferObj2 = (*env)->NewObject(env, StringBufferClass, initStringBufferMethod);
	(*env)->CallObjectMethod(env, stringBufferObj2, append_methodID, odexPath);
	jstring dex_str = (*env)->NewStringUTF(env, "/TargetApk.dex");
	(*env)->CallObjectMethod(env, stringBufferObj2, append_methodID, dex_str);
	jstring dexFilename = (jstring)(*env)->CallObjectMethod(env, stringBufferObj2, toString_methodID);

	jclass deleteFileClass = (*env)->FindClass(env, "java/io/File");
	jobject file1 = (*env)->NewObject(env, deleteFileClass, initFile_methodID, targetFilename);
	jobject file2 = (*env)->NewObject(env, deleteFileClass, initFile_methodID, dexFilename);

	jmethodID exists_methodID = (*env)->GetMethodID(env, deleteFileClass, "exists", "()Z");
	jmethodID delete_methodID = (*env)->GetMethodID(env, deleteFileClass, "delete", "()Z");

	if((*env)->CallBooleanMethod(env, file1, exists_methodID)){
		(*env)->CallBooleanMethod(env, file1, delete_methodID);
	}

	if((*env)->CallBooleanMethod(env, file2, exists_methodID)){
		(*env)->CallBooleanMethod(env, file2, delete_methodID);
	}
	*/

}

////////////////////////////////////
///JNI方法注册
////////////////////////////////////

static JNINativeMethod gMethods[] = 
{
	{"start", "(Landroid/app/Application;Ljava/lang/String;Ljava/lang/ClassLoader;Ljava/lang/Object;)V", (void*)native_start}
};

static int registerNativeMethods(JNIEnv* env, const char* className, 
	JNINativeMethod* gMethods, int numMethods)
{
	jclass clazz;
	clazz = (*env)->FindClass(env, className);
	if(clazz == NULL)
		return JNI_FALSE;

	if((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0)
		return JNI_FALSE;

	return JNI_TRUE;
}

static int registerNatives(JNIEnv* env)
{
	if(!registerNativeMethods(env, JNIREG_CLASS, gMethods, sizeof(gMethods)/sizeof(gMethods[0])))
		return JNI_FALSE;
	return JNI_TRUE;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	if( (*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_4) != JNI_OK)
		return -1;

	if(env == NULL)
		return -1;

	if(!registerNatives(env))
		return -1;

	result = JNI_VERSION_1_4;
	// __android_log_print(ANDROID_LOG_INFO, "JNITag", "pid = %d\n", getpid());
	return result;

}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved){
	JNIEnv* env = NULL;

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return;
	}
	(*env) -> UnregisterNatives(env, JNIREG_CLASS);
}

