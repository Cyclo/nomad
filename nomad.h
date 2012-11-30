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

/*

	NDPack File structure

	1 Header: nd_file
	[n] File Headers: nd_header
	[n] File Bodies: nd_body

*/

typedef struct nd_file nd_file;
typedef struct nd_header nd_header;
typedef struct nd_body nd_body;

// Pack core
nd_file *nd_alloc_pack();
void nd_create_pack(nd_file *);
void nd_free_pack(nd_file *);

// Internal
nd_header *nd_get_last_header(nd_file *);
void nd_calculate_offsets(nd_file *);
nd_body *nd_alloc_body(long);
nd_header *nd_find_file(nd_file*,char*);
int nd_count_headers(nd_file *);

// User
int nd_read_file(char*,char *,long);
void nd_add_file(nd_file*,char*);
void nd_write_pack(nd_file*,char*);
void nd_read_pack(nd_file*,char*);

void nd_extract_file(nd_file*,char*,char*);
void nd_dump_pack(nd_file*);