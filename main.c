#include <stdio.h>
#include <stdlib.h>

#include "nomad.h"

int main()
{
  nd_file file;
  nd_create_pack(&file);
  nd_add_file(&file,"a.out");
  nd_add_file(&file,"test.txt");
  nd_write_pack(&file,"out.p");
  printf("File has %d files\n",nd_count_headers(&file));
  nd_free_pack(&file);

  nd_file x = {0};
  nd_read_pack(&x,"out.p");

  char **filenames;
  int file_count;

  file_count = nd_get_filenames(&file,&filenames);

  int i; 
  for(i = 0; i < file_count; i++){
    printf("Filename: %s\n",filenames[i]);
    free(filenames[i]);
  }

  free(filenames);


  nd_extract_file(&x,"test.txt","test.xxx");
  nd_extract_file(&x,"a.out","b.out");

  nd_free_pack(&x);

  return 0;
}
