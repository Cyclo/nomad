#include <stdio.h>
#include "nomad.h"

int main()
{
  nd_file file;
  create_pack(&file);
  add_file(&file,"a.out");
  add_file(&file,"test.txt");
  write_pack(&file,"out.p");
  free_pack(&file);

  nd_file x = {0};
  read_pack(&x,"out.p");
  //dump_pack(&x);
  extract_file(&x,"test.txt","test.xxx");
  free_pack(&x);

  return 0;
}
