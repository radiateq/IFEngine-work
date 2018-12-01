#pragma once

#include <vector>

#include "IFFANN.h"
#include "IF_General_Utils.h"








namespace IFFANNEngine{

 using namespace IFFANN;

 extern IFGeneralUtils::CFileSystem IFFileSystem;

 //If index = -1 owner of the address is capsule
 //If connection = -1 owner is socket
 //If all values are set owner is connection
 //Layers[layer][index]->socket.connection[connection]
 typedef struct SCFANN_Address {
  unsigned int layer = -1, index = -1, connection = -1;
 };
 
 class IFANNEngine{
  char address_file_name[BUFSIZ+1];
 public:
  std::vector<char*> matching_files;
  ~IFANNEngine(){
  }
  bool Save(){  
  }
  bool Load(){   
  }
  char const * const GetAddressFileName(SCFANN_Address &address){
   sprintf(address_file_name, "%10u", address.layer );
   sprintf(&address_file_name[strlen(address_file_name)], "%10u", address.index);
   sprintf(&address_file_name[strlen(address_file_name)], "%10u", address.connection);
   if(address.index!=-1){
    sprintf(&address_file_name[strlen(address_file_name)], "%s", ".cap" );
    return address_file_name;
   }
   if (address.connection != -1) {
    sprintf(&address_file_name[strlen(address_file_name)], "%s", ".soc");
    return address_file_name;
   }
   sprintf(&address_file_name[strlen(address_file_name)], "%s", ".con");
   return address_file_name;
  }
  //In order to match all files under certain layer set index and connection to -1
  //In order to match all connections of socket set connection to -1
  unsigned int MatchToAddress(SCFANN_Address &_address){
   int compare_len = 30;
   if(_address.connection == -1 ){
    compare_len-=10;
   }
   if (_address.index == -1) {
    compare_len -= 10;
   }
   GetAddressFileName(_address);
   Free();
   IFFileSystem.Directory(IFFileSystem.CurrentDirectory());
   while(IFFileSystem.DirectoryNext()){
   
    //
    if(0==strncmp(address_file_name, IFFileSystem.GetLastDirectoryEntry(), compare_len)){
     //matching_files
     matching_files.push_back(strcpy((char*)malloc((strlen(IFFileSystem.GetLastDirectoryEntry())+1)*sizeof(IFFileSystem.GetLastDirectoryEntry()[0])), IFFileSystem.GetLastDirectoryEntry()));
    }
    //

   }
   return matching_files.size();
  }
  unsigned int MatchToType(char const * const type_string ) {
   Free();
   IFFileSystem.Directory(IFFileSystem.CurrentDirectory());
   while (IFFileSystem.DirectoryNext()) {

    //
    if (0 == IFGeneralUtils::strcmprev(IFFileSystem.GetLastDirectoryEntry(), type_string, strlen(type_string))){
     //matching_files
     matching_files.push_back(strcpy((char*)malloc((strlen(IFFileSystem.GetLastDirectoryEntry()) + 1) * sizeof(IFFileSystem.GetLastDirectoryEntry()[0])), IFFileSystem.GetLastDirectoryEntry()));
    }
    //

   }
   return matching_files.size();
  }
  void Free(){
   for( int cnt = 0; cnt < matching_files.size(); cnt++){
    free(matching_files[cnt]);
   }
   matching_files.clear();
  }
 };



 typedef struct SConnection{
  unsigned int output_index_connect;
  unsigned int to_input_index;
  SCFANN_Address address;
 };

 class CFANNCapsule;
 
 class CFANNSocket : public IFANNEngine {
 public:
  CFANNCapsule *connect_to_ifann;
  std::vector < SConnection > connections;
  SCFANN_Address address;//address.connection should be set to -1
  bool Save() {
   char file_path[BUFSIZ+1];
   if( chdir(RQNDKUtils::Make_storageDataPath(file_path,1024,"")) ) return false;

   for( size_t cnt = 0; cnt < connections.size(); cnt++ ){
    char const *file_name = GetAddressFileName(connections[cnt].address);
    IFFileSystem.Remove(file_name);
    if( !IFFileSystem.OpenFile(file_name, "w") ) return false;
    IFFileSystem.Write(&connections[cnt],sizeof(connections[cnt]));
    IFFileSystem.Free();
   }     

   return true;
  }
  bool Load() {   
   char file_path[BUFSIZ + 1];
   if (chdir(RQNDKUtils::Make_storageDataPath(file_path, 1024, ""))) return false;
   connections.clear();
   SConnection temp_con;
   if(MatchToAddress(address)){
    for( size_t cnt = 0; cnt < matching_files.size(); cnt++ ){
     IFFileSystem.OpenFile(matching_files[cnt], "r");
     IFFileSystem.Read(&temp_con,sizeof(temp_con));
     IFFileSystem.Free();
     connections.push_back(temp_con);
    }
   }
   return true;
  }
 };

 class CFANNCapsule : public IFANNEngine {
 public:
  
  //If outputs is null AI has not been run yet
  fann_type *inputs, *outputs;
  IFS_Cascade_FANN ifann;
  //If capsule has no sockets it is output network
  std::vector < CFANNSocket* > sockets;
  SCFANN_Address address;
  //If input values are not set before running, array may be passed to be copied to inputs before running
  fann_type *Run(fann_type *optional_inputs = NULL ){
   if(optional_inputs){
    for(unsigned int cnt = 0; cnt < ifann.ann->num_input; cnt++ ){
     inputs[cnt] = optional_inputs[cnt];
    }
   }
   return (outputs = IFFANN::Run_Cascade_FANN(&ifann, inputs));
  }  

  bool Save() {

   return true;
  }
  bool Load() {

   return true;
  }
//------------------------------------------------------------------
 };



 

 class CFANNNetwork{
 public:
  typedef std::vector<CFANNCapsule*> TLayer;
  typedef std::vector<TLayer*> TLayers;
  TLayers Layers;
  //n starts with 1, n and m are positive integers
  //1. First execute all networks on layer n, then set outputs of connected upper n+m layers to produced inputs
  //2. Increase n by 1 until number of layers reached
  //3. If n less than numlayers goto 1
  void OneEpoch(){
   for( size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++){
    TLayer *layer = Layers[layer_cnt];
    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
     (*layer)[net_cnt]->Run();
    }
    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
     CFANNCapsule *FANNCapsule = (*layer)[net_cnt];
     for (size_t socket_cnt = 0; socket_cnt < FANNCapsule->sockets.size(); socket_cnt++) {
      CFANNSocket *FANNSocket = (*Layers[layer_cnt])[net_cnt]->sockets[socket_cnt];
      for (size_t connection_cnt = 0; connection_cnt < FANNSocket->connections.size(); connection_cnt++) {
       FANNSocket->connect_to_ifann -> inputs [FANNSocket->connections[connection_cnt].to_input_index]
        =
       FANNCapsule->outputs [FANNSocket->connections[connection_cnt].output_index_connect];
      }
     }     
    }
   }
  }
  //unique_name is file name created by Save_Cascade_FANN
  //On load network gets layer and index in layer numbers appended to unique_name and resaved with socket data
  //returns index in layer
  unsigned int AddToNetwork(char const * const unique_name, unsigned int layer_no){
   while (layer_no >= Layers.size() ){
    Layers.push_back(new TLayer);
   }
   unsigned int layer_index = (*Layers[layer_no]).size();
   (*Layers[layer_no]).push_back(new CFANNCapsule);
   Load_Cascade_FANN(&(*Layers[layer_no])[layer_index]->ifann, unique_name, CnFinalFannPostscript );
   (*Layers[layer_no])[layer_index]->inputs = (fann_type*)malloc((*Layers[layer_no])[layer_index]->ifann.ann->num_input * sizeof(fann_type));
   (*Layers[layer_no])[layer_index]->outputs = NULL;
   return layer_index;
  }
  bool SaveNetwork(char const * const network_name){
   for (size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++) {
    TLayer *layer = Layers[layer_cnt];
    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
     CFANNCapsule *FANNCapsule = (*layer)[net_cnt];
     for (size_t socket_cnt = 0; socket_cnt < FANNCapsule->sockets.size(); socket_cnt++) {
      CFANNSocket *FANNSocket = (*Layers[layer_cnt])[net_cnt]->sockets[socket_cnt];
      if( FANNSocket->connect_to_ifann ){
       FANNSocket->connect_to_ifann;
      }else{
       NULL;
      }
      for (size_t connection_cnt = 0; connection_cnt < FANNSocket->connections.size(); connection_cnt++) {       
       FANNSocket->connections[connection_cnt];  
      }
     }
    }
   }
  }
  bool LoadNetwork(char const * const network_name) {
   Free();
   return true;
  }
  void Free(){
   for (size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++) {
    TLayer *layer = Layers[layer_cnt];
    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
     CFANNCapsule *FANNCapsule = (*layer)[net_cnt];
     free(FANNCapsule->inputs);
     FANNCapsule->inputs = NULL;
     for( size_t socket_cnt = 0; socket_cnt < (*Layers[layer_cnt])[net_cnt]->sockets.size(); socket_cnt++ ){
      delete(FANNCapsule->sockets[socket_cnt] );
     }
     Cleanup_Cascade_FANN(&FANNCapsule->ifann);
     delete(FANNCapsule);
    }    
    delete(layer);
   }
  }
 };

 //class CFANNFarm{
 //public: 
 //};
 

}