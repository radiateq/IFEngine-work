#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

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



}