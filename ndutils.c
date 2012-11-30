#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void 
die(char *s)
{
  fprintf(stderr,"%s\n",s);
  exit(1);
}

char *
basename(char *path)
{
  char *p = strrchr(path,'/');
  if(!p){
    return path;
  }

  return p++;
}

long 
get_file_size(char *path)
{
  long size;
  FILE *fp;

  fp = fopen(path,"rb");
  fseek(fp,0,SEEK_END);
  size = ftell(fp);
  fclose(fp);

  return size;
}

int 
nd_file_exists(char *path){

  FILE *fp = fopen(path,"r");

  if(fp){
    fclose(fp);
    return 1;
  }

  return 0;
}