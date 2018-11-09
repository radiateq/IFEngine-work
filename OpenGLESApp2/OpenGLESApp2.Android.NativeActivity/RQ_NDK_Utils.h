#ifndef __RQ_NDK_Utils__
#define __RQ_NDK_Utils__

#include <stdint.h>
#include <time.h>

#include <list>

#include <android/asset_manager.h> 
#include <android/asset_manager_jni.h>

#include "android_fopen.h"

int64_t timespec2ms64(struct timespec *pt);
int64_t timespec2us64(struct timespec *pt);
void *getAssetFileToBuffer(android_app* state, const char *pFileName, size_t &size);

//Usage example
//int64_t xxval = timespec2ms64(&t0);


template<typename Tptr_type> class RQTCAutoDelete {
 public:
 Tptr_type *managed_pointer;
 RQTCAutoDelete(Tptr_type *_managed_pointer = NULL) {
  managed_pointer = _managed_pointer;
 }
 ~RQTCAutoDelete() {
  if(managed_pointer==NULL)
   return;
  delete managed_pointer;
  managed_pointer = NULL;
 }
};

template<typename TDataType_np, bool delete_pointers_on_destructor = false, bool pointer_array = false > class S_listWrap_ptr : public std::list<TDataType_np*>
{
public:
 void removeElement(TDataType_np *input_var) {
  for (typename std::list<TDataType_np*>::iterator iter = this->begin(); iter != this->end(); iter++) {
   if ((*iter) == input_var) {
    if (pointer_array)
     delete[] input_var;
    else
     delete input_var;
    this->erase(iter);//this->erase(iter++) to continue execution if search again for multiple elements
    break;
   }
  }
 }
 ~S_listWrap_ptr()
 {
  if (delete_pointers_on_destructor)
  {
   TDataType_np *temp_var;
   while (this->size() > 0)
   {
    temp_var = this->back();
    if (pointer_array)
     delete[] temp_var;
    else
     delete temp_var;
    this->pop_back();
   }
  }

 }
};


//void Test4mleaks() {
// S_listWrap_ptr<int, true> pointerlist;
// int *findme = new int(1);
// pointerlist.push_back(findme);
// pointerlist.push_back(new int(2));
// pointerlist.push_back(new int(3));
// pointerlist.push_back(new int(4));
// pointerlist.removeElement(findme);
// pointerlist.removeElement(*(pointerlist.begin()));
// S_listWrap_ptr<int, true, true> pointerArraylist;
// int *findmea = new int[100];
// pointerArraylist.push_back(new int[10]);
// pointerArraylist.push_back(new int[50]);
// pointerArraylist.push_back(findmea);
// pointerArraylist.push_back(new int[150]);
// pointerArraylist.removeElement(findmea);
// pointerArraylist.removeElement(*(pointerArraylist.begin()));
//}
//
//int main()
//{
// _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
// while (!_kbhit()) {
//  Test4mleaks();
// }
//
//
// return 0;
//}



#define IF_NULL_DELETE(PARAM) if( NULL != PARAM ) { delete PARAM; PARAM = NULL; }
#define IF_NULL_DELETE_ARRAY(PARAM) if( NULL != PARAM ) { delete [] PARAM; PARAM = NULL; }



#endif
