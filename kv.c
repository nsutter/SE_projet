#include "kv.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

KV *kv_open (const char *dbnamec, const char *mode, int hidx, alloc_t alloc)
{
  int fd1, fd2, fd3, fd4;
  int longueur=0;
  longueur= strlen(dbnamec);
  char *namec= malloc((longueur+2)*sizeof(char));
  char *nameblk= malloc((longueur+4)*sizeof(char));
  char *namekv= malloc((longueur+3)*sizeof(char));
  char *namedkv= malloc((longueur+4)*sizeof(char));
  strcpy(namec, dbnamec);
  strcpy(nameblk, dbnamec);
  strcpy(namekv, dbnamec);
  strcpy(namedkv, dbnamec);
  strcat(namec, ".c");
  strcat(nameblk, ".blk");
  strcat(namekv, ".kv");
  strcat(namedkv, ".dkv");
  if(strcmp(mode, "r"))
  {
    fd1=open(namec, O_RDONLY);
    fd2=open(nameblk, O_RDONLY);
    fd3=open(namekv, O_RDONLY);
    fd4=open(namedkv, O_RDONLY);
  }
  else if(strcmp(mode, "r+"))
  {
    fd1=open(namec, O_RDWR);
    fd2=open(nameblk, O_RDWR);
    fd3=open(namekv, O_RDWR);
    fd4=open(namedkv, O_RDWR);
  }
  else if(strcmp(mode, "w"))
  {
    fd1=open(namec, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    fd2=open(nameblk, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    fd3=open(namekv, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    fd4=open(namedkv, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  }
  else if(strcmp(mode, "w+"))
  {
    fd1=open(namec, O_RDWR | O_CREAT | O_TRUNC, 0666);
    fd2=open(nameblk, O_RDWR | O_CREAT | O_TRUNC, 0666);
    fd3=open(namekv, O_RDWR | O_CREAT | O_TRUNC, 0666);
    fd4=open(namedkv, O_RDWR | O_CREAT | O_TRUNC, 0666);
  }
  free(namec);
  free(nameblk);
  free(namekv);
  free(namedkv);
}
