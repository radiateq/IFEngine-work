#pragma once

#include <set>

#include "IFFANN.h"
#include "IF_General_Utils.h"








namespace IFFANNEngine{

 using namespace IFFANN;

 extern IFGeneralUtils::CFileSystem IFFileSystem;







 


 //IFS_Cascade_FANN

 class CNodeRegister;

 //input pin set:
 //all input pins that are not connected are listed here(user set before execution)
 // -list of pin IDs 
 class CInputPinRegister {
 public:
  CNodeRegister *NodeRegister;
  IFGeneralUtils::SMapWrap<unsigned int, fann_type*> input_pins;
 };
 //output pin set:
 //all output pins that are not connected are listed here (user read after execution)
 class COutputPinRegister {
 public:
  CNodeRegister *NodeRegister;
  IFGeneralUtils::SMapWrap<unsigned int, fann_type*> output_pins;
 };
 //pin:
 // -pin ID 
// -list of IDs of pins this pin is connected to 
 class CPin{
 public:
  unsigned int ID;
  fann_type *data;
  unsigned int fann_index;//index in fann input/output array
  std::set<unsigned int> connected_pins;
 };
 //pin map:
 // -map pin ID = key, owner node ID = value
 class CPinToNode {
 public:
  CNodeRegister *NodeRegister;
  IFGeneralUtils::SMapWrap<unsigned int, unsigned int> pin_to_node_ID;
  void Register(CPin *Pin, unsigned int node_ID) {
   Pin->ID = NewID();
   pin_to_node_ID.Add(Pin->ID, node_ID);
  
  }
  void Unregister(CPin *Pin) {
   Pin->ID = UINT_MAX;
   pin_to_node_ID.Remove(Pin->ID);
  }
  unsigned int NewID() {
   if (pin_to_node_ID.Map.empty()) {
    return 1;
   }
   pin_to_node_ID.Iter_Map = --pin_to_node_ID.Map.end();
   unsigned int newID = pin_to_node_ID.Iter_Map->first + 1;
   bool looping = false;
   while (pin_to_node_ID.IsPresent(newID)) {
    if (newID == UINT_MAX) {
     if (looping) return UINT_MAX;
     looping = true;
    }
    else {
     newID++;
    }
   }
   return newID;
  }
 };
 //node:
 // -input pins with IDs
 // -output pins with IDs
 // -ifs cascade fann
 // -node ID
 class CNode{
 public:
  CNodeRegister *NodeRegister;
  unsigned int ID;
  IFS_Cascade_FANN ifann;
  fann_type *inputs = NULL, *outputs;  
  IFGeneralUtils::SMapWrap<unsigned int, CPin*> input_pins;//Key is index of input for fann
  IFGeneralUtils::SMapWrap<unsigned int, CPin*> output_pins;//Key is index of output for fann
  CPin *GetInPinByIndex(unsigned int index){
   CPin *pin = NULL;
   input_pins.Get(index,pin);
   return pin;
  }
  CPin *GetOutPinByIndex(unsigned int index) {
   CPin *pin = NULL;
   output_pins.Get(index, pin);
   return pin;
  }
  CPin *GetInPinByID(unsigned int ID) {
   CPin *pin = NULL;
   unsigned int index;
   input_pins.ResetIterator();
   while(0 <= input_pins.GetNextIterator(index,pin)){
    if(pin->ID == ID ){
     break;
    }
    pin = NULL;
   }
   return pin;
  }
  CPin *GetOutPinByID(unsigned int ID) {
   CPin *pin = NULL;
   unsigned int index;
   output_pins.ResetIterator();
   while (0<=output_pins.GetNextIterator(index, pin)) {
    if (pin->ID == ID) {
     break;
    }
    pin = NULL;
   }
   return pin;
  }
  bool LoadCore(char const * const core_name);
  bool UnloadCore();
  bool SaveCore() {
   IFFANN::Save_Cascade_FANN(&ifann, IFFANN::CnFinalFannPostscript);
  }
 };
 //node map:
 // -map node ID = key, pointer to node = value
 class CNodeRegister{
 public: 
  CInputPinRegister InputPinRegister;
  COutputPinRegister OutputPinRegister;
  CPinToNode PinToNode;
 public:
  CNodeRegister(){
   InputPinRegister.NodeRegister = OutputPinRegister.NodeRegister = PinToNode.NodeRegister = this;
  }
  IFGeneralUtils::SMapWrap<unsigned int, CNode*> nodes_by_ID;
  void Register(CNode *Node){
   Node->ID = NewID();
   Node->NodeRegister = this;
   nodes_by_ID.Add(Node->ID,Node);
  }
  void Unregister(CNode *Node) {
   Node->UnloadCore();
   Node->ID = UINT_MAX;
   Node->NodeRegister = NULL;
   nodes_by_ID.Remove(Node->ID);
  }

  unsigned int NewID(){
   if(nodes_by_ID.Map.empty()){
    return 1;
   }
   nodes_by_ID.Iter_Map = --nodes_by_ID.Map.end();
   unsigned int newID = nodes_by_ID.Iter_Map->first + 1;
   bool looping = false;
   while(nodes_by_ID.IsPresent(newID)){
    if(newID == UINT_MAX){
     if(looping) return UINT_MAX;
     looping = true;
    }else{
     newID++;
    }
   }
   return newID;
  }
 };

 //pin order:
 // every pin has a counter that presents its order
 // input pin always has odd number
 // output pin always has even number
 // each pin that accepts connection, has to increase counter by one
 // when pins are deleted but counters reach maximum value consolidate (see running the network)
 // - map sorted by key, counter int = key, list of pin IDs = value
 // - map pin ID = key, counter = value 
 //example:
 // first n pins (input) have conters set to 1
 // output n pins have counter 2
 // output n pins connect to input pins for the first time and input pins set their counters to 3
 class CNetwork{
 public:
  CNodeRegister NodeRegister;
  IFGeneralUtils::SMapWrap<int, std::set<unsigned int> > LayerToPins;
  IFGeneralUtils::SMapWrap<unsigned int, int > PinToLayer;
  CNode *GetNodeByPinID(unsigned int pinID) {
   CNode *Node;
   NodeRegister.PinToNode.pin_to_node_ID.Get(pinID, pinID);
   NodeRegister.nodes_by_ID.Get(pinID, Node);
   return Node;
  }
 private:
  CPin *pin_out, *pin_in;
  CNode *Node1, *Node2;
 public:
  bool ConnectPins(CPin *outPin, CPin *inPin) {
   return ConnectPins(outPin->ID, inPin->ID);
  }
  bool ConnectPins(unsigned int outID, unsigned int inID){

   if (NodeRegister.InputPinRegister.input_pins.IsPresent(inID)) {
    NodeRegister.InputPinRegister.input_pins.Remove(inID);
   }else{
    return false;
   }

   Node1 = GetNodeByPinID(outID);
   Node1->output_pins.Get(outID, pin_out);
   Node2 = GetNodeByPinID(inID);
   Node2->input_pins.Get(inID, pin_in);

   if (NodeRegister.OutputPinRegister.output_pins.IsPresent(outID)) {
    NodeRegister.OutputPinRegister.output_pins.Remove(outID);
   }

   pin_out = Node1->GetOutPinByID( outID);
   pin_out->connected_pins.insert(inID);
   pin_in = Node2->GetInPinByID(inID);
   pin_in->connected_pins.insert(outID);

   return true;
  }
  bool DisconnectPins(CPin *outPin, CPin *inPin) {
   return DisconnectPins(outPin->ID, inPin->ID);
  }
  bool DisconnectPins(unsigned int outID, unsigned int inID) {

   Node1 = GetNodeByPinID(outID);
   Node1->output_pins.Get(outID, pin_out);
   Node2 = GetNodeByPinID(inID);
   Node2->input_pins.Get(inID, pin_in);

   pin_out = Node1->GetOutPinByID(outID);
   pin_out->connected_pins.erase(inID);
   pin_in = Node2->GetInPinByID(inID);
   pin_in->connected_pins.erase(outID);

   if (pin_out->connected_pins.size()==0) {
    NodeRegister.OutputPinRegister.output_pins.pAdd(outID, pin_out->data);
   }
   if (pin_in->connected_pins.size() == 0) {
    NodeRegister.InputPinRegister.input_pins.pAdd(inID, pin_in->data);
   }

   return true;
  }
 };
 //running the network:
 // before network is executed 
 //  when counters are deleted (no associated pin IDs) delete their keys from map, sort map and rename all keys so that counters are consecutive again if even n and n+1 counters both are missing (all input and output pins of one layer)
 // executing stage
 //  1. using counter map, find pin IDs with lowest odd counter - 
 //  2. using pin map find node IDs of all nodes with odd_low_n counter
 //  3. select nodes that have all input pins with counter odd_low_n or lower
 //  4. execute selected networks
 //  5. using counter map, find pin IDs with lowest even counter larger than odd_low_n - even_low_n
 //  6. find node pointer and go threw list of all input pins selected even_low_n pins are connected to
 //  7. if there are no connections end execution
 //  8. copy their values (results of execution) to corresponding input pins
 //  9. find next odd counter larger than even_low_n - odd_low_n and repeat from step 2
 //loading/saving
 // TBD
 void TestNetwork();


//
// //If index = -1 owner of the address is capsule
// //If connection = -1 owner is socket
// //If all values are set owner is connection
// //Layers[layer][index]->socket.connection[connection]
// typedef struct SCFANN_Address {
//  unsigned int layer = -1, index = -1, connection = -1;
// };
// 
// class IFANNEngine{
//  char address_file_name[BUFSIZ+1];
// public:
//  std::vector<char*> matching_files;
//  ~IFANNEngine(){
//  }
//  bool Save(){  
//  }
//  bool Load(){   
//  }
//  char const * const GetAddressFileName(SCFANN_Address &address){
//   sprintf(address_file_name, "%10u", address.layer );
//   sprintf(&address_file_name[strlen(address_file_name)], "%10u", address.index);
//   sprintf(&address_file_name[strlen(address_file_name)], "%10u", address.connection);
//   if(address.index!=-1){
//    sprintf(&address_file_name[strlen(address_file_name)], "%s", ".cap" );
//    return address_file_name;
//   }
//   if (address.connection != -1) {
//    sprintf(&address_file_name[strlen(address_file_name)], "%s", ".soc");
//    return address_file_name;
//   }
//   sprintf(&address_file_name[strlen(address_file_name)], "%s", ".con");
//   return address_file_name;
//  }
//  void StringToAddress(SCFANN_Address &address, char * const string){
//   char tmp_char = string[10];
//   string[10] = '\0';
//  
//   char *stop_string; 
//   address.layer = strtoul(string, &stop_string,10);
//   string[10] = tmp_char;
//   tmp_char = string[20];
//   string[20] = '\0';
//   address.index = strtoul(&string[10], &stop_string, 10);
//   tmp_char = string[30];
//   string[30] = '\0';
//   address.connection = strtoul(&string[20], &stop_string, 10);   
//   tmp_char = string[30];
//  }
//  //In order to match all files under certain layer set index and connection to -1
//  //In order to match all connections of socket set connection to -1
//  unsigned int MatchToAddress(SCFANN_Address &_address){
//   int compare_len = 30;
//   if(_address.connection == -1 ){
//    compare_len-=10;
//   }
//   if (_address.index == -1) {
//    compare_len -= 10;
//   }
//   GetAddressFileName(_address);
//   Free();
//   IFFileSystem.Directory(IFFileSystem.CurrentDirectory());
//   while(IFFileSystem.DirectoryNext()){
//   
//    //
//    if(0==strncmp(address_file_name, IFFileSystem.GetLastDirectoryEntry(), compare_len)){
//     //matching_files
//     matching_files.push_back(strcpy((char*)malloc((strlen(IFFileSystem.GetLastDirectoryEntry())+1)*sizeof(IFFileSystem.GetLastDirectoryEntry()[0])), IFFileSystem.GetLastDirectoryEntry()));
//    }
//    //
//
//   }
//   return matching_files.size();
//  }
//  unsigned int MatchToType(char const * const type_string ) {
//   Free();
//   IFFileSystem.Directory(IFFileSystem.CurrentDirectory());
//   while (IFFileSystem.DirectoryNext()) {
//
//    //
//    if (0 == IFGeneralUtils::strcmprev(IFFileSystem.GetLastDirectoryEntry(), type_string, strlen(type_string))){
//     //matching_files
//     matching_files.push_back(strcpy((char*)malloc((strlen(IFFileSystem.GetLastDirectoryEntry()) + 1) * sizeof(IFFileSystem.GetLastDirectoryEntry()[0])), IFFileSystem.GetLastDirectoryEntry()));
//    }
//    //
//
//   }
//   return matching_files.size();
//  }
//  void Free(){
//   for( int cnt = 0; cnt < matching_files.size(); cnt++){
//    free(matching_files[cnt]);
//   }
//   matching_files.clear();
//  }
// };
//
//
//
// typedef struct SConnection{
//  unsigned int output_index_connect;
//  unsigned int to_input_index;
//  SCFANN_Address address;
// };
//
// class CFANNCapsule;
// 
// class CFANNSocket : public IFANNEngine {
// public:
//  CFANNCapsule *connect_to_ifann;
//  std::vector < SConnection > connections;
//  SCFANN_Address address;//address.connection should be set to -1
//  bool Save();
//  bool Load() {   
//   //connect_to_ifann is loaded by parrent CFANNNetwork after loading is complete
//   connections.clear();
//   SConnection temp_con;
//   if(MatchToAddress(address)){
//    for( size_t cnt = 0; cnt < matching_files.size(); cnt++ ){
//     IFFileSystem.OpenFile(matching_files[cnt], "r");
//     IFFileSystem.Read(&temp_con,sizeof(temp_con));
//     IFFileSystem.Free();
//     connections.push_back(temp_con);
//    }
//   }
//   return true;
//  }
// };
   //bool CFANNSocket::Save() {
   // char file_path[BUFSIZ + 1];
   // if (chdir(RQNDKUtils::Make_storageDataPath(file_path, 1024, ""))) return false;

   // char const *file_name = GetAddressFileName(connect_to_ifann->address);
   // char const *file_name2 = GetAddressFileName(address);
   // if (!IFFileSystem.OpenFile(file_name2, "w")) return false;
   // IFFileSystem.Write(file_name, sizeof(file_name[0]) * strlen(file_name));
   // IFFileSystem.Free();

   // for (size_t cnt = 0; cnt < connections.size(); cnt++) {
   //  file_name = GetAddressFileName(connections[cnt].address);
   //  IFFileSystem.Remove(file_name);
   //  if (!IFFileSystem.OpenFile(file_name, "w")) return false;
   //  IFFileSystem.Write(&connections[cnt], sizeof(connections[cnt]));
   //  IFFileSystem.Free();
   // }

   // return true;
   //}
//
// class CFANNCapsule : public IFANNEngine {
// public:
//  ~CFANNCapsule(){
//   free(inputs);
//   inputs = NULL;
//   outputs = NULL;
//  }  
//  //If outputs is null AI has not been run yet
//  fann_type *inputs = NULL, *outputs = NULL;
//  IFS_Cascade_FANN ifann;
//  //If capsule has no sockets it is output network
//  std::vector < CFANNSocket* > sockets;
//  SCFANN_Address address;
//  //If input values are not set before running, array may be passed to be copied to inputs before running
//  fann_type *Run(fann_type *optional_inputs = NULL ){
//   if(optional_inputs){
//    for(unsigned int cnt = 0; cnt < ifann.ann->num_input; cnt++ ){
//     inputs[cnt] = optional_inputs[cnt];
//    }
//   }
//   return (outputs = IFFANN::Run_Cascade_FANN(&ifann, inputs));
//  }  
//
//  bool Save() {
//   //ifann.unique_name should be address connected to sockets of this instance
//   IFFANN::Save_Cascade_FANN(&ifann, IFFANN::CnFinalFannPostscript);
//
//   //Save 
//   char const *file_name = GetAddressFileName(address);   
//   if (!IFFileSystem.OpenFile(file_name, "w")) return false;
//   size_t string_len;
//   string_len = strlen(ifann.unique_name) + 1;
//   IFFileSystem.Write(&string_len, sizeof(string_len));
//   IFFileSystem.Write(ifann.unique_name, sizeof(ifann.unique_name[0]) * string_len);
//   string_len = strlen(IFFANN::CnFinalFannPostscript) + 1;
//   IFFileSystem.Write(&string_len, sizeof(string_len));
//   IFFileSystem.Write(IFFANN::CnFinalFannPostscript, sizeof(IFFANN::CnFinalFannPostscript[0]) * string_len);
//   IFFileSystem.Free();
//
//   for( size_t cnt = 0; cnt < sockets.size(); cnt++){
//    sockets[cnt]->Save();
//   }   
//
//   return true;
//  }
//  bool Load() {
//   char file_path[BUFSIZ + 1], file2_path[BUFSIZ + 1];
//
//   char const *file_name = GetAddressFileName(address);
//   if (!IFFileSystem.OpenFile(file_name, "r")) return false;
//   size_t string_len;
//   IFFileSystem.Read(&string_len, sizeof(string_len));
//   IFFileSystem.Read(file_path, sizeof(file_path[0]) * string_len);
//   IFFileSystem.Read(&string_len, sizeof(string_len));
//   IFFileSystem.Read(file2_path, sizeof(file2_path[0]) * string_len);
//   IFFileSystem.Free();
//
//   IFFANN::Load_Cascade_FANN(&ifann, file_path, file2_path);
//
//   free(inputs);
//   inputs = (fann_type*)malloc(sizeof(fann_type)*ifann.ann->num_input);
//
//   Free();
//   CFANNSocket *temp_con;
//   if (MatchToAddress(address)) {
//    for (size_t cnt = 0; cnt < matching_files.size(); cnt++) {
//     size_t string_len, string2_len;//file_path
//     IFFileSystem.OpenFile(matching_files[cnt], "r");
//     temp_con = new CFANNSocket;     
//     StringToAddress(temp_con->address, matching_files[cnt]);
//     temp_con->Load();
//     IFFileSystem.Free();
//     sockets.push_back(temp_con);     
//    }
//   }
//
//
//   return true;
//  }
//  void Free(){
//   for( size_t cnt = 0; cnt < sockets.size(); cnt++){
//    delete(sockets[cnt]);
//   }
//   sockets.clear();
//  }
////------------------------------------------------------------------
// };
//
//
//
// 
//
// class CFANNNetwork : public IFANNEngine {
// public:
//  typedef std::vector<CFANNCapsule*> TLayer;
//  typedef std::vector<TLayer*> TLayers;
//  TLayers Layers;
//  //n starts with 1, n and m are positive integers
//  //1. First execute all networks on layer n, then set outputs of connected upper n+m layers to produced inputs
//  //2. Increase n by 1 until number of layers reached
//  //3. If n less than numlayers goto 1
//  void OneEpoch(){
//   for( size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++){
//    TLayer *layer = Layers[layer_cnt];
//    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
//     (*layer)[net_cnt]->Run();
//    }
//    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
//     CFANNCapsule *FANNCapsule = (*layer)[net_cnt];
//     for (size_t socket_cnt = 0; socket_cnt < FANNCapsule->sockets.size(); socket_cnt++) {
//      CFANNSocket *FANNSocket = (*Layers[layer_cnt])[net_cnt]->sockets[socket_cnt];
//      for (size_t connection_cnt = 0; connection_cnt < FANNSocket->connections.size(); connection_cnt++) {
//       FANNSocket->connect_to_ifann -> inputs [FANNSocket->connections[connection_cnt].to_input_index]
//        =
//       FANNCapsule->outputs [FANNSocket->connections[connection_cnt].output_index_connect];
//      }
//     }     
//    }
//   }
//  }
//  //unique_name is file name created by Save_Cascade_FANN
//  //On load network gets layer and index in layer numbers appended to unique_name and resaved with socket data
//  //returns index in layer
//  unsigned int AddToNetwork(char const * const unique_name, unsigned int layer_no){
//   while (layer_no >= Layers.size() ){
//    Layers.push_back(new TLayer);
//   }
//   unsigned int layer_index = (*Layers[layer_no]).size();
//   (*Layers[layer_no]).push_back(new CFANNCapsule);
//   Load_Cascade_FANN(&(*Layers[layer_no])[layer_index]->ifann, unique_name, CnFinalFannPostscript );
//   (*Layers[layer_no])[layer_index]->inputs = (fann_type*)malloc((*Layers[layer_no])[layer_index]->ifann.ann->num_input * sizeof(fann_type));
//   (*Layers[layer_no])[layer_index]->outputs = NULL;
//   return layer_index;
//  }
//  bool SaveNetwork(char const * const network_name){
//   char file_path[BUFSIZ + 1];
//   if (chdir(RQNDKUtils::Make_storageDataPath(file_path, 1024, network_name))) return false;
//   for (size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++) {
//    TLayer *layer = Layers[layer_cnt];
//    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
//     CFANNCapsule *FANNCapsule = (*layer)[net_cnt];
//     FANNCapsule->Save();
//    }
//    return true;
//   }
//   //for (size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++) {
//   // TLayer *layer = Layers[layer_cnt];
//   // for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
//   //  CFANNCapsule *FANNCapsule = (*layer)[net_cnt];
//   //  for (size_t socket_cnt = 0; socket_cnt < FANNCapsule->sockets.size(); socket_cnt++) {
//   //   CFANNSocket *FANNSocket = (*Layers[layer_cnt])[net_cnt]->sockets[socket_cnt];
//   //   if( FANNSocket->connect_to_ifann ){
//   //    FANNSocket->connect_to_ifann;
//   //   }else{
//   //    NULL;
//   //   }
//   //   for (size_t connection_cnt = 0; connection_cnt < FANNSocket->connections.size(); connection_cnt++) {       
//   //    FANNSocket->connections[connection_cnt];  
//   //   }
//   //  }
//   // }
//   //}
//  }
//  bool LoadNetwork(char const * const network_name) {
//
//   Free();
//
//   char file_path[BUFSIZ + 1];
//   if (chdir(RQNDKUtils::Make_storageDataPath(file_path, 1024, network_name))) return false;
//   //AddToNetwork
//
//   MatchToType(".cap");
//   CFANNCapsule *temp_cap;
//   for (size_t cnt = 0; cnt < matching_files.size(); cnt++) {
//    //temp_cap = new CFANNCapsule();
//
//    AddToNetwork
//
//
//    StringToAddress(temp_cap->address,  matching_files[cnt]);
//    temp_cap->Load();
//    
//   }
//
//   return true;
//  }
//  void Free(){
//   for (size_t layer_cnt = 0; layer_cnt < Layers.size(); layer_cnt++) {
//    TLayer *layer = Layers[layer_cnt];
//    for (size_t net_cnt = 0; net_cnt < Layers[layer_cnt]->size(); net_cnt++) {
//     CFANNCapsule *FANNCapsule = (*layer)[net_cnt];
//     free(FANNCapsule->inputs);
//     FANNCapsule->inputs = NULL;
//     for( size_t socket_cnt = 0; socket_cnt < (*Layers[layer_cnt])[net_cnt]->sockets.size(); socket_cnt++ ){
//      delete(FANNCapsule->sockets[socket_cnt] );
//     }
//     Cleanup_Cascade_FANN(&FANNCapsule->ifann);
//     delete(FANNCapsule);
//    }    
//    delete(layer);
//   }
//  }
// };
//
// //class CFANNFarm{
// //public: 
// //};
// 

}