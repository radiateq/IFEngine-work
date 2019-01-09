#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


#include <list>
#include <vector>
#include <map>
#include <set>
#include <Eigen/Dense>

#include "../OpenGLESApp2.Android.NativeActivity/IFGlobals.h"
#include "./Common/External/AudioFile/AudioFile.h"

using Eigen::MatrixXf;

#define IF_NULL_DELETE(PARAM) if( NULL != PARAM ) { delete PARAM; PARAM = NULL; }
#define IF_NULL_DELETE_ARRAY(PARAM) if( NULL != PARAM ) { delete [] PARAM; PARAM = NULL; }

extern TS_User_Data User_Data;

namespace IFGeneralUtils{
 
 int strcmprev(char const * const string1, char const * const string2, unsigned int compare_len);

 class CFileSystem{
 public:
  FILE *file_handle = NULL;
  //"r" Opens a file for reading.The file must exist.
  //"w" Creates an empty file for writing.If a file with the same name already exists, its content is erased and the file is considered as a new empty file.
  //"a" Appends to a file.Writing operations, append data at the end of the file.The file is created if it does not exist.
  //"r+" Opens a file to update both reading and writing.The file must exist.
  //"w+" Creates an empty file for both reading and writing.
  //"a+" Opens a file for reading and appending.
  bool OpenFile(char const * const file_path, char const * const fopen_flags){
   Free();
   file_handle = fopen(file_path, fopen_flags);
   if(file_handle) return true;
   return false;
  }
  unsigned long int Write( void const * const write_data, unsigned long int const data_len ){
   if(file_handle==NULL) return 0;
   return fwrite(write_data, sizeof(char), data_len, file_handle);
  }
  unsigned long int Read(void * const read_data, unsigned long int const data_len){
   if (file_handle == NULL) return 0;
   unsigned long int read_data_len = fread(read_data, sizeof(char), data_len, file_handle);
   if((data_len == read_data_len)||((data_len > read_data_len) && feof(file_handle))) return read_data_len;
   return 0;        
  }
  bool FileExists(char const * const file_path){
   struct stat st;
   int res = stat(file_path, &st);
   if (0 == res && st.st_mode & S_IFREG) return true;
   return false;
  }
  bool DirExists(char const * const file_path) {
   struct stat st;
   int res = stat(file_path, &st);
   if (0 == res && st.st_mode & S_IFDIR) return true;
   return false;    
  }
#pragma warning( push )
#pragma warning( disable : 4716 )
  bool MakeDir(char const * const file_path){
   if (mkdir(file_path, 0777)) return false; else return true;
  }
  int Remove(char const * const file_path, bool _dir = false ){
   if(_dir){
    struct stat st;    
    int res = stat(file_path, &st);
    if (0 == res && st.st_mode & S_IFDIR){//LOGI("present");
     if( rmdir(file_path) ) return -1; else return 1;
    }else {
     return -2;
    }
   }else{
    struct stat st;
    int res = stat(file_path, &st);
    if (0 == res && st.st_mode & S_IFREG) {//LOGI("present");
     if (remove(file_path)) return -3; else return 1;
    }
    else {
     return -4;
    }
   }
  }
 #pragma warning( pop )
 private:
  bool dir_initiated = false;  
  DIR *dir_handle = NULL;
  struct dirent *dir_entry;
  FILE *entry_file;
  char dir_entry_buffer[BUFSIZ + 1];
 public:
  char const * const CurrentDirectory(){
   getcwd(dir_entry_buffer, sizeof(dir_entry_buffer) - 1);
   return dir_entry_buffer;
  }
  bool Directory(char const * const file_path){
   if(dir_initiated) closedir(dir_handle), dir_handle = NULL, dir_initiated = false;
   struct stat st;
   int res = stat(file_path, &st);
   if (0 == res && st.st_mode & S_IFDIR) {//LOGI("present");
    if (NULL == (dir_handle = opendir(file_path))) return false;
    dir_initiated = true;
    return true;
   }
   return false;
  }
  char *DirectoryNext(){
   if( dir_initiated ) return (char*)NULL;
    if ((dir_entry = readdir(dir_handle)) != NULL) {
     strcpy(dir_entry_buffer, dir_entry->d_name);
     return dir_entry_buffer;
    }
    closedir(dir_handle), dir_handle = NULL, dir_initiated = false;
    return (char*)NULL;
  }
  char *GetLastDirectoryEntry(){
   return dir_entry_buffer;
  }
  void Free(){
   if(file_handle) fclose(file_handle), file_handle=NULL;
  }
 };

 template< typename TDataType > struct SSetWrap
 {
 public:
  typedef std::set< TDataType > TSet;
  TSet Set;
  typename TSet::iterator Iter_Set;
  void Add(TDataType *pData)
  {
   Set.insert(*pData);
  }
  void Add(TDataType Data)
  {
   Set.insert(Data);
  }
  bool Get(TDataType &Data)
  {
   Iter_Set = Set.find(Data);
   if (Set.end() == Iter_Set)
   {
    return false;
   }
   Data = *Iter_Set;
   return true;
  }
  //Return values
  //  2 : First element before element with sought key returned
  //  1 : Element with same key or higher returned - sought key is first in map
  // -1 : Map Empty
  //TDataType &Data is IN/OUT	
  int GetFirstBefore(TDataType &Data)
  {
   int RetVal = 1;
   bool KeyAdded = false;
   if (0 == Set.size())
    return -1;
   TDataType TempData;
   Iter_Set = Set.find(Data);
   //Key not found, add it
   if (Set.end() == Iter_Set)
   {
    Add(Data);
    KeyAdded = true;
   }
   //Get iterator to our key
   Iter_Set = Set.lower_bound(Data);
   //Is our key first in map
   if (Iter_Set == Set.begin())
   {//Yes
    //We have at least two elements in map and first element is our temporary key, so move iterator on next element
    if (KeyAdded)
    {
     Iter_Set++;
    }
    RetVal = 2;
   }
   else
   {//No
    Iter_Set--;
   }
   TempData = *Iter_Set;
   if (KeyAdded)
   {
    Remove(Data);
   }
   Iter_Set = Set.lower_bound(TempData);
   Data = *Iter_Set;
   return RetVal;
  }
  bool Remove(TDataType &Data)
  {
   Iter_Set = Set.find(Data);
   if (Set.end() == Iter_Set)
   {
    return false;
   }
   Set.erase(Data);
   return false;
  }
 };


 template< typename TKeyType, typename TDataType> struct SMapWrap
 {
 public:
  typedef std::map< TKeyType, TDataType > TMap;
  TMap Map;
  typedef std::pair< TKeyType, TDataType > TMap_KeyData;
  typename TMap::iterator Iter_Map, Iter_MapGetRef;
  void Add(TKeyType &Key, TDataType &Data)
  {
   Map.insert(TMap_KeyData(Key, Data));
  }
  void pAdd(TKeyType &Key, TDataType Data)
  {
   Map.insert(TMap_KeyData(Key, Data));
  }
  bool AddCheck(TKeyType &Key, TDataType &Data)
  {
   if (IsPresent(Key))
    return false;
   Map.insert(TMap_KeyData(Key, Data));
   return true;
  }
  //removes element with passed key if exists and then adds passed
  void AddReplace(TKeyType &Key, TDataType &Data)
  {
   if (IsPresent(Key))
    Remove(Key);
   Map.insert(TMap_KeyData(Key, Data));
  }
  //				IN				OUT
  bool Get(TKeyType &Key, TDataType &Data)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return false;
   }
   Data = Iter_Map->second;
   return true;
  }

  TDataType* GetRef(TKeyType &Key)
  {
   Iter_MapGetRef = Map.find(Key);
   if (Map.end() == Iter_MapGetRef)
   {
    return NULL;
   }

   return &(Iter_MapGetRef->second);
  }
  TDataType pGet(TKeyType &Key)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return NULL;
   }
   return Iter_Map->second;
  }
  bool IsPresent(TKeyType &Key)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return false;
   }
   return true;
  }
  void ResetIterator()
  {
   Iter_Map = Map.begin();
  }
  int GetNextIterator(TKeyType &Key, TDataType &Data)
  {
   if (Map.empty())
    return -1;
   if (Iter_Map == Map.end())
    return -2;
   Key = Iter_Map->first;
   Data = Iter_Map->second;
   ++Iter_Map;
   return 1;
  }
  //Return values
  //  2 : First element before element with sought key returned
  //  1 : Element with same key or higher returned - sought key is first in map
  // -1 : Map Empty
  int GetFirstBefore(TKeyType &Key, TDataType &Data)
  {
   int RetVal = 1;
   bool KeyAdded = false;
   if (0 == Map.size())
    return -1;
   TKeyType TempKey;
   Iter_Map = Map.find(Key);
   //Key not found, add it
   if (Map.end() == Iter_Map)
   {
    Add(Key, Data);
    KeyAdded = true;
   }
   //Get iterator to our key
   Iter_Map = Map.lower_bound(Key);
   //Is our key first in map
   if (Iter_Map == Map.begin())
   {//Yes
    //We have at least two elements in map and first element is our temporary key, so move iterator on next element
    if (KeyAdded)
    {
     Iter_Map++;
    }
    RetVal = 2;
   }
   else
   {//No
    Iter_Map--;
   }
   TempKey = Iter_Map->first;
   if (KeyAdded)
   {
    Remove(Key);
   }
   Iter_Map = Map.lower_bound(TempKey);
   Data = Iter_Map->first;
   return RetVal;
  }
  bool Remove(TKeyType &Key)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return false;
   }
   Map.erase(Key);
   return true;
  }
 };

 template< typename TKeyType, typename TDataType> struct SMultimapWrap
 {
 public:
  typedef std::multimap< TKeyType, TDataType > TMap;
  TMap Map;
  typedef std::pair< TKeyType, TDataType > TMap_KeyData;
  typename TMap::iterator Iter_Map, Iter_MapGetRef;
  void Add(TKeyType &Key, TDataType &Data)
  {
   Map.insert(TMap_KeyData(Key, Data));
  }
  void pAdd(TKeyType &Key, TDataType Data)
  {
   Map.insert(TMap_KeyData(Key, Data));
  }
  bool AddCheck(TKeyType &Key, TDataType &Data)
  {
   if (IsPresent(Key))
    return false;
   Map.insert(TMap_KeyData(Key, Data));
   return true;
  }
  //removes element with passed key if exists and then adds passed
  void AddReplace(TKeyType &Key, TDataType &Data)
  {
   if (IsPresent(Key))
    Remove(Key);
   Map.insert(TMap_KeyData(Key, Data));
  }
  //				IN				OUT
  bool Get(TKeyType &Key, TDataType &Data)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return false;
   }
   Data = Iter_Map->second;
   return true;
  }

  TDataType* GetRef(TKeyType &Key)
  {
   Iter_MapGetRef = Map.find(Key);
   if (Map.end() == Iter_MapGetRef)
   {
    return NULL;
   }

   return &(Iter_MapGetRef->second);
  }
  TDataType pGet(TKeyType &Key)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return NULL;
   }
   return Iter_Map->second;
  }
  bool IsPresent(TKeyType &Key)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return false;
   }
   return true;
  }
  void ResetIterator()
  {
   Iter_Map = Map.begin();
  }
  int GetNextIterator(TKeyType &Key, TDataType &Data)
  {
   if (Map.empty())
    return -1;
   if (Iter_Map == Map.end())
    return -2;
   Key = Iter_Map->first;
   Data = Iter_Map->second;
   ++Iter_Map;
   return 1;
  }
  //Return values
  //  2 : First element before element with sought key returned
  //  1 : Element with same key or higher returned - sought key is first in map
  // -1 : Map Empty
  int GetFirstBefore(TKeyType &Key, TDataType &Data)
  {
   int RetVal = 1;
   bool KeyAdded = false;
   if (0 == Map.size())
    return -1;
   TKeyType TempKey;
   Iter_Map = Map.find(Key);
   //Key not found, add it
   if (Map.end() == Iter_Map)
   {
    Add(Key, Data);
    KeyAdded = true;
   }
   //Get iterator to our key
   Iter_Map = Map.lower_bound(Key);
   //Is our key first in map
   if (Iter_Map == Map.begin())
   {//Yes
    //We have at least two elements in map and first element is our temporary key, so move iterator on next element
    if (KeyAdded)
    {
     Iter_Map++;
    }
    RetVal = 2;
   }
   else
   {//No
    Iter_Map--;
   }
   TempKey = Iter_Map->first;
   if (KeyAdded)
   {
    Remove(Key);
   }
   Iter_Map = Map.lower_bound(TempKey);
   Data = Iter_Map->first;
   return RetVal;
  }
  bool Remove(TKeyType &Key)
  {
   Iter_Map = Map.find(Key);
   if (Map.end() == Iter_Map)
   {
    return false;
   }
   Map.erase(Key);
   return true;
  }
 };

 template<typename Tptr_type> class RQTCAutoDelete {
 public:
  Tptr_type *managed_pointer;
  RQTCAutoDelete(Tptr_type *_managed_pointer = NULL) {
   managed_pointer = _managed_pointer;
  }
  ~RQTCAutoDelete() {
   if (managed_pointer == NULL)
    return;
   delete[] managed_pointer;
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
  TDataType_np* operator [](size_t index) {
   for (typename std::list<TDataType_np*>::iterator iter = this->begin(); iter != this->end(); iter++) {
    if (index-- == 0) {
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
   if (pthread_mutex_trylock(&mutex)) {
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



 void CopyFloat16ToMatrix(Eigen::Matrix<float, 4, 4> &mf, float *mfa);
 void CopyMatrix16ToFloat(Eigen::Matrix<float, 4, 4> &mf, float *mfa);


 void LoadIdentityMatrix(float *matrix4x4);



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


 class CSimpleWav {
 public:
  AudioFile<double> audio_file;
  unsigned int num_channels, num_samples, sample_rate;
  AudioFile<double>::AudioBuffer buffer;
  CSimpleWav() {
   sample_rate = 44100;
   num_channels = 2;
   buffer.resize(num_channels);
   num_samples = 0;
  }
  void SetBufferSize(const unsigned int samples_count) {
   /////////////////////////
   // // // Buffer Size 
   /////////////////////////////
   num_samples = samples_count;
   buffer[0].resize(num_samples);
   buffer[1].resize(num_samples);
  }
  //////////////////////////////////////
  // // //          Test Signal
  //////////////////////////////////////
  void TestTone() {
   float _sample_rate = sample_rate;
   float frequency = 440.f;
   for (int i = 0; i < num_samples; i++)
   {
    float sample = sinf(2. * M_PI * ((float)i / _sample_rate) * frequency);

    for (int channel = 0; channel < num_channels; channel++)
     buffer[channel][i] = sample * 0.5;
   }
  }
  void AddSample(float sampleL, float sampleR) {
   if (buffer[0].size() <= num_samples) {
    buffer[0].push_back(sampleL);
    buffer[1].push_back(sampleR);
   }
  }
  void SaveFile(char const * const _file_name) {
   audio_file.setAudioBuffer(buffer);
   // Set both the number of channels and number of samples per channel
   audio_file.setAudioBufferSize(num_channels, num_samples);
   // Set the number of samples per channel
   audio_file.setNumSamplesPerChannel(num_samples);
   // Set the number of channels
   audio_file.setNumChannels(num_channels);
   audio_file.setBitDepth(24);
   audio_file.setSampleRate(sample_rate);
   char file_name[BUFSIZ + 1];
   RQNDKUtils::Make_storageDataPath(file_name, BUFSIZ, _file_name);
   audio_file.save(file_name);
  }
 };
 //Usage example
 //IFGeneralUtils::CSimpleWav SensorWav;
 /////////////////////////////////
 //SensorWav.SetBufferSize(SensorWav.buffer[0].size() + User_Data.accel[0].size() );
 //for( size_t cnt = 0; cnt < User_Data.accel[0].size(); cnt++){       
 // SensorWav.AddSample(User_Data.accel[0].at(cnt), User_Data.accel[1].at(cnt));
 //}
 //if(User_Data.accel[0].size()){
 // User_Data.accel[0].clear();
 // User_Data.accel[1].clear();
 //}
 //////////////////////////
 //SensorWav.SaveFile("shake.wav");

}