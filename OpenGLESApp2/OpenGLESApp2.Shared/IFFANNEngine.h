#pragma once

#include "IFFANN.h"
#include <vector>


namespace IFFANNEngine{

 using namespace IFFANN;

 typedef struct SConnection{
  unsigned int output_index_connect;
  unsigned int to_input_index;
 };

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
 };


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
   
  }
 }

 //class CFANNFarm{
 //public: 
 //};
 

 

 /*
  CFANNFarm makes different types of neural networks
  Each network is encapsulated in 
 */

}