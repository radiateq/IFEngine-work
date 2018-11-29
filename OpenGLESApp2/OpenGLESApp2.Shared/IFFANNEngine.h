#pragma once

#include "IFFANN.h"
#include <vector>



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>




namespace IFFANNEngine{

 using namespace IFFANN;

 typedef struct SConnection{
  unsigned int output_index_connect;
  unsigned int to_input_index;
 };

 class CFANNCapsule;

 class CFANNSocket {
 public:
  CFANNCapsule *connect_to_ifann;
  std::vector < SConnection > connections;
 };

 class CFANNCapsule{
 public:
  fann_type *inputs, *outputs;
  IFS_Cascade_FANN ifann;
  std::vector < CFANNSocket > sockets;
  //If input values are not set before running, array may be passed to be copied to inputs before running
  fann_type *Run(fann_type *optional_inputs = NULL ){
   if(optional_inputs){
    for(unsigned int cnt = 0; cnt < ifann.ann->num_input; cnt++ ){
     inputs[cnt] = optional_inputs[cnt];
    }
   }
   return (outputs = IFFANN::Run_Cascade_FANN(&ifann, inputs));
  }  
 };



 /* This is just a sample code, modify it to meet your need */
 int ListDirs(char *in_dir);


 class CFANNNetwork{
 public:
  typedef std::vector<CFANNCapsule> TLayer;
  typedef std::vector<TLayer> TLayers;
  TLayers Layers;
  //n starts with 1, n and m are positive integers
  //1. First execute all networks on layer n, then set outputs of connected upper n+m layers to produced inputs
  //2. Increase n by 1 until number of layers reached
  //3. If n less than numlayers goto 1
  void OneEpoch(){
   for( size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++){
    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt].size(); net_cnt++) {
     Layers[layer_cnt][net_cnt].Run();
    }
    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt].size(); net_cnt++) {
     for (size_t socket_cnt = 0; socket_cnt < Layers[layer_cnt][net_cnt].sockets.size(); socket_cnt++) {
      for (size_t connection_cnt = 0; connection_cnt < Layers[layer_cnt][net_cnt].sockets[socket_cnt].connections.size(); connection_cnt++) {
       Layers[layer_cnt][net_cnt].sockets[socket_cnt].connect_to_ifann -> inputs [Layers[layer_cnt][net_cnt].sockets[socket_cnt].connections[connection_cnt].to_input_index]
        =
       Layers[layer_cnt][net_cnt] . outputs [Layers[layer_cnt][net_cnt].sockets[socket_cnt].connections[connection_cnt].output_index_connect];
      }
     }     
    }
   }
  }
  //unique_name is file name created by Save_Cascade_FANN
  //On load network gets layer and index in layer numbers appended to unique_name and resaved with socket data
  bool AddNetwork(char const * const unique_name, unsigned int layer_no){
   struct stat st;
   char path_name[1024];
   char dir_name[1024];
   strcpy(dir_name,"xyz");
   RQNDKUtils::Make_storageDataPath(path_name, 1024, dir_name);
   

   int res = mkdir(path_name, 0777);
   RQNDKUtils::Make_storageDataPath(dir_name, 1024, "xyz/file.del");
   FILE *testf = fopen(dir_name, "w");
   if(testf){
    fclose(testf);
   }


   ListDirs(path_name);

   res = rmdir(path_name);

   res = remove(dir_name);
   res = rmdir(path_name);
  int a = 1;
   //int res = stat(path_name, &st);   
   //if (0 == res && st.st_mode & S_IFREG){//LOGI("Application config file already present");
   // int a = 0;
   // a++;
   //}
   //else
   //{
   // res = mkdir(path_name, 0770);
   // if (0 != res) {
   //  int a = 0;
   //  a++;
   // }
   //}
   //res = rmdir(path_name);
   //if (0 != res) {
   // int a = 0;
   // a++;
   //}



  }
  bool SaveNetwork(char const * const network_name){
   for (size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++) {
    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt].size(); net_cnt++) {
     Layers[layer_cnt][net_cnt].Run();
    }
   }
   return true;
  }
  bool LoadNetwork(char const * const network_name) {
   return true;
  }
 };

 //class CFANNFarm{
 //public: 
 //};
 

}