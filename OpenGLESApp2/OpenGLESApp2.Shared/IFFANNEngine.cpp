#include "IFFANNEngine.h"


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
#include <dirent.h>
//#include <unistd.h>
//#include <errno.h>


namespace IFFANNEngine {

 int ListDirs(char *in_dir)
 {
  DIR* FD;
  struct dirent* in_file;
  FILE    *entry_file;
  char    buffer[BUFSIZ];

  /* Scanning the in directory */
  if (NULL == (FD = opendir(in_dir)))
  {
   //fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));

   return 1;
  }
  while ((in_file = readdir(FD)))
  {
   /* On linux/Unix we don't want current and parent directories
    * On windows machine too, thanks Greg Hewgill
    */
   if (!strcmp(in_file->d_name, "."))
    continue;
   if (!strcmp(in_file->d_name, ".."))
    continue;
   /* Open directory entry file for common operation */
   /* TODO : change permissions to meet your need! */
   entry_file = fopen(in_file->d_name, "rw");
   if (entry_file == NULL)
   {

    return 1;
   }

   /* Doing some struf with entry_file : */
   /* For example use fgets */
   while (fgets(buffer, BUFSIZ, entry_file) != NULL)
   {
    /* Use fprintf or fwrite to write some stuff into common_file*/
   }

   /* When you finish with the file, close it */
   fclose(entry_file);
  }

  /* Don't forget to close common file before leaving */

  return 0;
 }
}