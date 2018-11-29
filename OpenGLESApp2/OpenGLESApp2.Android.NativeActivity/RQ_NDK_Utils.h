#ifndef __RQ_NDK_Utils__
#define __RQ_NDK_Utils__

#include <stdint.h>
#include <time.h>

#include <list>

#include <queue>
#include <pthread.h>

#include <android/asset_manager.h> 
#include <android/asset_manager_jni.h>

#include "android_fopen.h"

#include <Eigen/Dense>


using Eigen::MatrixXf;

#define IF_NULL_DELETE(PARAM) if( NULL != PARAM ) { delete PARAM; PARAM = NULL; }
#define IF_NULL_DELETE_ARRAY(PARAM) if( NULL != PARAM ) { delete [] PARAM; PARAM = NULL; }

namespace RQNDKUtils {


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
  delete [] managed_pointer;
  managed_pointer = NULL;
 }
};

template<typename TDataType_np, bool delete_pointers_on_destructor = false, bool pointer_array = false > class S_listWrap_ptr : public std::list<TDataType_np*>
{
public:
 bool removeElement(TDataType_np *input_var) {
  for (typename std::list<TDataType_np*>::iterator iter = this->begin(); iter != this->end(); iter++) {
   if ((*iter) == input_var) {
    if (pointer_array)
     delete[] input_var;
    else
     delete input_var;
    this->erase(iter);//this->erase(iter++) to continue execution if search again for multiple elements
    
    return true;
   }
  }
  return false;
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
 TDataType_np* operator []( size_t index){  
  for (typename std::list<TDataType_np*>::iterator iter = this->begin(); iter != this->end(); iter++) {
   if( index-- == 0 ){
    return (*iter);
   }
  }
  return NULL;
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


template<typename TDataType_np, bool delete_pointers_on_destructor = false, bool pointer_array = false > class S_vectorWrap_ptr : public std::vector<TDataType_np*>
{
public:
 ~S_vectorWrap_ptr()
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









class ICSLock
{
public:
 pthread_mutex_t mutex;
 ICSLock()
 {
  pthread_mutex_init(&mutex, NULL);
 }
 ~ICSLock()
 {
  pthread_mutex_destroy(&mutex);
 }
 void EnterAndLock()
 {
  pthread_mutex_lock(&mutex);
 }
 bool TryEnteringAndLock()
 {
  if(pthread_mutex_trylock(&mutex)){
   return false;
  }
  return true;
 }
 void LeaveAndUnlock()
 {
  pthread_mutex_unlock(&mutex);
 }
};

struct SAutoICSLock
{
private:
 ICSLock *pCSLock;
public:
 SAutoICSLock(ICSLock *_pCSLock, bool EnterLock = true)
 {
  if (EnterLock)
   _pCSLock->EnterAndLock();
  pCSLock = _pCSLock;
 }
 ~SAutoICSLock()
 {
  pCSLock->LeaveAndUnlock();
 }
};

template<typename TDataType> class TC_THSafe_Queue : public std::queue<TDataType>, public ICSLock
{
#define DEF_TC_THSafe_Queue_LOCK SAutoICSLock AutoLock(this);
#define OVERRIDE_QUEUE(FUNC_NAME,RET_VAL,PARAMT,PARAM) \
	RET_VAL THS##FUNC_NAME(PARAMT) \
	{ \
		DEF_TC_THSafe_Queue_LOCK \
        return this->FUNC_NAME(PARAM); \
	}
public:
 //queue<TDataType> THSQueue;
 //Returns a reference to the last and most recently added element at the back of the queue. 
 OVERRIDE_QUEUE(back, TDataType&, , )
  //Tests if the queue is empty.  
  OVERRIDE_QUEUE(empty, bool, , )
  //Returns a reference to the first element at the front of the queue. 
  OVERRIDE_QUEUE(front, TDataType&, , )
  //Removes an element from the front of the queue. 
  OVERRIDE_QUEUE(pop, void, , )
  //Adds an element to the back of the queue. 
  OVERRIDE_QUEUE(push, void, const TDataType& val, val)
  //Returns the number of elements in the queue. 
  OVERRIDE_QUEUE(size, size_t, , )
#undef DEF_TC_THSafe_Queue_LOCK
#undef OVERRIDE_QUEUE
};
/*
 Usage Example

 TC_THSafe_Queue<int> THSStdInEventFIFO;
 THSStdInEventFIFO.push(1);
 THSStdInEventFIFO.THSpush(2);
 cout << THSStdInEventFIFO.THSback() << endl;
 cout << THSStdInEventFIFO.back() << endl;

*/



void CopyFloat16ToMatrix(MatrixXf &mf, float *mfa);
void CopyMatrix16ToFloat(MatrixXf &mf, float *mfa);


void LoadIdentityMatrix(float *matrix4x4);


int32_t getDensityDpi(android_app* app);


struct SRangeScale
{
 SRangeScale()
 {
  SetRange(0, 1);
 }
 SRangeScale(double _RangeMin, double _RangeMax)
 {
  SetRange(_RangeMin, _RangeMax);
 }
 void SetRange(double _RangeMin, double _RangeMax)
 {
  RangeMin = _RangeMin;
  RangeMax = _RangeMax;
 }
 double Scale(double Value, double ValueMin, double ValueMax)
 {
  return ((RangeMax - RangeMin) / (ValueMax - ValueMin)) * Value;
 }
 double ScaleAndClip(double Value, double ValueMin, double ValueMax)
 {
  double Result = ((RangeMax - RangeMin) / (ValueMax - ValueMin)) * Value;
  if (RangeMin > Result)
   Result = RangeMin;
  else if (RangeMax < Result)
   Result = RangeMax;
  return Result;
 }
 double RangeMin;
 double RangeMax;
};

struct SRangeScale2
{
 SRangeScale2()
 {
  SetRange(0, 1);
 }
 SRangeScale2(double _RangeMin, double _RangeMax)
 {
  SetRange(_RangeMin, _RangeMax);
 }
 void SetRange(double _RangeMin, double _RangeMax)
 {
  RangeMin = _RangeMin;
  RangeMax = _RangeMax;
 }
 double Scale(double Value, double ValueMin, double ValueMax)
 {
  //double Result = RangeMin + ( ( RangeMax - RangeMin ) / ( ValueMax - ValueMin ) ) * ( Value - ValueMin );
  return (RangeMin + ((RangeMax - RangeMin) / (ValueMax - ValueMin)) * (Value - ValueMin));
 }
 double ScaleAndClip(double Value, double ValueMin, double ValueMax)
 {
  double Result = RangeMin + ((RangeMax - RangeMin) / (ValueMax - ValueMin)) * (Value - ValueMin);
  if (RangeMin > Result)
   Result = RangeMin;
  else if (RangeMax < Result)
   Result = RangeMax;
  return Result;
 }
 double RangeMin;
 double RangeMax;
};

 
 char *Make_storageDataPath(char *path_buffer, int buffer_length, char *file_name);

}
#endif
