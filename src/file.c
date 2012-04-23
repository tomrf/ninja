#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"


int file_isregular(char *fpath)
{
  struct stat st;

  if (lstat(fpath, &st) == 0) {

    if (!S_ISREG(st.st_mode))
      return 0;

  } else {

    return -1;

  }

  return 1;
}


long file_size(char *fpath)
{
  FILE *fp;
  long fsize;

  if ((fp = fopen(fpath, "r")) == NULL)
    return -1;

  if (fseek(fp, 0, SEEK_END) != 0)
    return -2;

  fsize = ftell(fp);
  fclose(fp);

  return fsize;
}
