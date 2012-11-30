#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nomad.h"


nd_file *
nd_alloc_pack()
{
  nd_file *f;

  f = malloc(sizeof(nd_file));

  if(!f){
    die("nd_alloc_pack: failed to allocate space");
  }

  nd_create_pack(f);

  return f;
}

void 
nd_create_pack(nd_file *f)
{
  if(!f){
    die("No nd_file allocated");
  }

  memset(f,0,sizeof(nd_file));

  f->count = 0;
  f->headers = NULL;
  f->magic[0] = 'N';
  f->magic[1] = 'D';
}

void 
nd_free_pack(nd_file *f)
{
  if(!f->headers){
    return;
  }

  nd_header *h = f->headers;
  nd_header *t = NULL;

  while(h != NULL){
    t = h->next;

    if(h->body){
      if(h->body->data){
        free(h->body->data);
      }
      free(h->body);
    }
    free(h);
    h = t;
  }
}

nd_header *
nd_get_last_header(nd_file *f)
{
  if(!f){
    die("no nd_file");
  }

  nd_header *t = f->headers;

  while(t && (t->next != NULL)){
    t = t->next;
  }

  return t;
}

void 
nd_calculate_offsets(nd_file *f)
{
  nd_header *h = f->headers;

  int base,i;
  i = 0;
  base = sizeof(nd_file) + (f->count*sizeof(struct nd_header));
  
  while(h != NULL){
    
    h->offset = base;
    base += h->length;

    h = h->next;
  }
}

nd_body *
nd_alloc_body(long l)
{
  nd_body *b = malloc(sizeof(struct nd_body));
  
  if(!b){
    die("nd_alloc_body: out of memory");
  }

  b->data = malloc(l);

  if(!b->data){
    die("nd_alloc_body: out of memory");
  }

  return b;
}

// Path, Buffer, Length

int 
nd_read_file(char *p,char *b,long l)
{
  if(!p ||  strlen(p) <= 0){
    return -1;
  }

  FILE *fp;
  fp = fopen(p,"rb");

  if(!fp){
    return -1;
  }

  int status;
  status = fread(b,l,1,fp);
  
  fclose(fp);
  return status;
}


void 
nd_add_file(nd_file *f, char *path)
{
  if(!f){
    die("No nd_file");
  }

  if(!nd_file_exists(path)){
    fprintf(stderr,"File: %s doesn't exist, skipping.\n",path);
    return;
  }

  nd_header *h = malloc(sizeof(nd_header));
  nd_header *last = nd_get_last_header(f);

  if(!h){
    die("out of memory");
  }

  if(last){
    last->next = h;
  }else{
    f->headers = h;
  }

  f->count++;

  char *fname = basename(path);
  strcpy(h->filename,fname);

  h->offset = -1;
  h->length = get_file_size(path);
  h->next = NULL;
  h->body = nd_alloc_body(h->length);

  if(nd_read_file(path,h->body->data,h->length) < 0){
    die("couldn't read file body");
  }
}

void 
nd_write_pack(nd_file *f, char *path)
{
  FILE *fp;
  fp = fopen(path,"wb");

  if(!fp){
    die("couldn't open file for writing");
  }

  nd_calculate_offsets(f);

  // write the header
  // TODO: Write part of structure
  fwrite(f,sizeof(nd_file),1,fp);

  // Loop headers and write
  nd_header *h = f->headers;

  while(h != NULL){

    fwrite(h,1,sizeof(nd_header),fp);
    h = h->next;
  }

  h = f->headers;

  while(h != NULL){

    fwrite(h->body->data,1,h->length,fp);
    h = h->next;
  }

  fclose(fp);
}

void 
nd_read_pack(nd_file *f, char *path)
{

  FILE *fp;
  fp = fopen(path,"rb");
  if(!fp){
    die("couldn't open file for reading");
  }

  fread(f,1,sizeof(struct nd_file),fp);
  f->headers = NULL;

  if(strcmp(f->magic,"ND") == 0){

    printf("nd_read_pack: magic confirmed: ND\n");
  }else{
    fprintf(stderr,"magic didn't match ND!=%s\n",f->magic);
    goto cleanup;
  }

  printf("nd_read_pack: pack contains %d files\n",f->count);

  // Read the first header
  nd_header *h,*t;
  int tcount,cur,read;
  
  tcount = cur = read = 0;
  h = t = NULL;

  while(tcount < f->count){

    if(h) t = h;
    h = malloc(sizeof(nd_header));

    if(!h){
      fclose(fp);
      die("out of memory");
    } 

    if(!f->headers){
      f->headers = h;
    }else{
      t->next = h;
    }

    fread(h,1,sizeof(struct nd_header),fp);
    cur = ftell(fp);
    
    printf("nd_read_pack: header[%d] %s offset:%d length: %d\n",tcount,h->filename,h->offset,h->length);

    h->body = nd_alloc_body(h->length);
    fseek(fp,h->offset,SEEK_SET);
    
    read = fread(h->body->data,1,h->length,fp);

    printf("nd_read_pack: read %d bytes of body data for header[%d]\n",read,tcount);
    if(read != h->length){
      die("nd_read_pack: didn't read correct amount of data");
    }

    // Seek back to next header
    fseek(fp,cur,SEEK_SET);

    tcount++;
  }


cleanup:
  fclose(fp);
}

nd_header *
nd_find_file(nd_file *f, char *filename)
{
  if(!f){
    die("no nd_file");
  }

  if(!filename || strlen(filename) <= 0){
    die("find_file: no filename");
  }

  nd_header *h = f->headers;

  while(h != NULL){

    if(strcmp(h->filename,filename) == 0){
      return h;
    }

    h = h->next;
  }

  return NULL;
}


void 
nd_extract_file(nd_file *f, char *filename, char *path)
{
  if(!f){
    die("no nd_file");
  }

  if(!filename || strlen(filename) <= 0){
    die("nd_extract_file: no filename");
  }

  if(!path || strlen(path) <= 0){
    die("nd_extract_file: no path specified");
  }
  
  nd_header *h = nd_find_file(f,filename);
  if(!h){
    printf("nd_extract_file: no file found\n");
    return;
  }

  FILE *fp;
  fp = fopen(path,"wb");
  
  if(!fp){
    die("nd_extract_file: couldnt open file for writing");
  }  
  
  if(!h->body->data){
    die("nd_extract_file: no body data");
  }

  int read = fwrite(h->body->data,1,h->length,fp);
  printf("nd_extract_file: write %d bytes of data to %s\n",read,path);
  fclose(fp);
}

int
nd_count_headers(nd_file *f)
{

  if(!f){
    die("nd_count_headers: no nd_file");
  }

  if(!f->headers){
    return 0;
  }

  int i = 1;
  nd_header *h = f->headers;

  while((h = h->next) != NULL && i++)
    ;

  return i;
}

int 
nd_get_filenames(nd_file *f, char **list)
{

  if(!f){
    die("nd_get_filenames: no nd_file");
  }

  if(!f->headers){
    die("nd_get_filesnames: pack has no files");
  }

  int h_count;
  nd_header *h = f->headers;

  h_count = nd_count_headers(f);

  while(h != NULL){

    h = h->next;
  }
}

void 
nd_dump_pack(nd_file *f)
{
  if(!f && !f->headers){
    die("nd_dump_pack: no headers or nd_file");
  }

  nd_header *h = f->headers;

  printf("----HEAD DUMP----\n");
  while(h != NULL){
    printf("Filename:%s\nOffset:%d\nSize:%d\n----------\n",h->filename,h->offset,h->length);
    printf("Body: %s\n",h->body->data);
    h = h->next;
  }
}

