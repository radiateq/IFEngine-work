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

}



