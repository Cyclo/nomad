#include "ndutils.h"

struct nd_body{
  char *data;
};

struct nd_header{
  char filename[255];
  long offset;             // offset of file, relative to 0
  long length;             // length of file

  struct nd_header *next;  // next files header
  struct nd_body *body;
};

struct nd_file{
  char magic[2];     
  int count;               // how many files in pack

  struct nd_header *headers;
};


typedef struct nd_file nd_file;
typedef struct nd_header nd_header;
typedef struct nd_body nd_body;


void create_pack(nd_file *);
void free_pack(nd_file *);
nd_header *get_last_header(nd_file *);
void calculate_offsets(nd_file *);
nd_body *alloc_body(long);
int read_file(char*,char *,long);
void add_file(nd_file*,char*);
void write_pack(nd_file*,char*);
void read_pack(nd_file*,char*);
nd_header *find_file(nd_file*,char*);
void extract_file(nd_file*,char*,char*);
void dump_pack(nd_file*);