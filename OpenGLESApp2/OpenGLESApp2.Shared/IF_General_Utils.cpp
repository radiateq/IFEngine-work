#include "IF_General_Utils.h"


namespace IFGeneralUtils {

 int strcmprev( char const * const string1, char const * const string2, unsigned int compare_len){
  unsigned int len1 = strlen(string1), len2 = strlen(string2);
  while(compare_len-- && len1-- && len2--){
   if(string1[len1]!= string2[len2]){
    if(string1[len1] < string2[len2]) return -1;
    return 1;
   }
  }
  return 0;
 }


 void CopyFloat16ToMatrix(MatrixXf &mf, float *mfa) {
  for (int cnt = 0; cnt < 16; cnt++) {
   mf(cnt) = mfa[cnt];
  }
 }
 void CopyMatrix16ToFloat(MatrixXf &mf, float *mfa) {
  for (int cnt = 0; cnt < 16; cnt++) {
   mfa[cnt] = mf(cnt);
  }
 }

 void LoadIdentityMatrix(float *matrix4x4) {
  //1000
  //0100
  //0010
  //0001
  memset(matrix4x4, 0.0, sizeof(float) * 16);
  matrix4x4[0] = 1.0;
  matrix4x4[5] = 1.0;
  matrix4x4[10] = 1.0;
  matrix4x4[15] = 1.0;
 }

}



