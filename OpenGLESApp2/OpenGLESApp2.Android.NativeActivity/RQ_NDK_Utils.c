#include "RQ_NDK_Utils.h"

#include "IFGlobals.h"


namespace RQNDKUtils {


int64_t timespec2ms64(struct timespec *pt) {
 return (((int64_t)(*pt).tv_sec) * 1000000000LL + (*pt).tv_nsec) / 1000000;
}
int64_t timespec2us64(struct timespec *pt) {
 return (((int64_t)(*pt).tv_sec) * 1000000000LL + (*pt).tv_nsec) / 1000;
}


//User free to free passed buffer
void *getAssetFileToBuffer(android_app* state, const char *pFileName, size_t &size){
 void *pBuffer = NULL;

 ANativeActivity* activity = state->activity;
 JNIEnv* env = 0;

 activity->vm->AttachCurrentThread(&env, NULL);

 jclass activity_class = env->GetObjectClass(activity->clazz);

 jmethodID activity_class_getAssets = env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
 jobject asset_manager = env->CallObjectMethod(activity->clazz, activity_class_getAssets); // activity.getAssets();
 jobject global_asset_manager = env->NewGlobalRef(asset_manager);

 AAssetManager *pAssetManager = AAssetManager_fromJava(env, global_asset_manager);


 activity->vm->DetachCurrentThread();


 android_fopen_set_asset_manager(pAssetManager);

 FILE* android_file = android_fopen(pFileName);
 fseek(android_file, 0L, SEEK_END);
 size = ftell(android_file);
 rewind(android_file);
 pBuffer = malloc(size + 1);
 fread(pBuffer, size, 1, android_file);
 fclose(android_file);

 return pBuffer; 
}


int32_t getDensityDpi(android_app* app) {
 AConfiguration* config = AConfiguration_new();
 AConfiguration_fromAssetManager(config, app->activity->assetManager);
 int32_t density = AConfiguration_getDensity(config);
 AConfiguration_delete(config);
 return density;
}

char *Make_storageDataPath(char * const path_buffer, int buffer_length, char const * const file_name) {

 TS_User_Data *user_data = (TS_User_Data *)p_user_data;
 int intpathlen = strlen(user_data->state->activity->externalDataPath);

 if (buffer_length < (intpathlen + strlen(file_name) + 2)) {
  return NULL;
 } 

 strcpy(path_buffer, user_data->state->activity->externalDataPath);
 strcpy(&path_buffer[intpathlen], "/");
 intpathlen++;
 strcpy(&path_buffer[intpathlen], file_name);

 return path_buffer;
}

char *Make_privateeDataPath(char * const path_buffer, int buffer_length, char const * const file_name){

 TS_User_Data *user_data = (TS_User_Data *)p_user_data;
 int intpathlen = strlen(user_data->state->activity->internalDataPath);

 if (buffer_length < (intpathlen + strlen(file_name) + 2)) {
  return NULL;
 }

 strcpy(path_buffer, user_data->state->activity->internalDataPath);
 strcpy(&path_buffer[intpathlen], "/");
 intpathlen++;
 strcpy(&path_buffer[intpathlen], file_name);

 return path_buffer;
}


}
