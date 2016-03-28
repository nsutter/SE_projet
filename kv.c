#include "kv.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define taille_header_f 1
#define taille_header_b 32

int hash0(char tab[])
{
  int i;
  int somme = 0;

  for(i = 0; tab[i] != '\0'; i++)
  {
    somme += tab[i];
  }

  return(somme % 999983);
}

int hash1(char tab[])
{
  int i;
  int somme = 0;

  for(i = 0; tab[i] != '\0'; i++)
  {
    somme += (i + 1) * tab[i];
  }

  return(somme % 999983);
}

int hash2(char tab[])
{
  // non implanté
  return 0;
}

int hash(char tab[], KV *kv)
{
  if(kv->hidx == 0) // fonction de hachage 0
  {
    return hash0(tab);
  }
  else if(kv->hidx == 1) // fonction de hachage 1
  {
    return hash1(tab);
  }
  else if(kv->hidx == 2) // fonction de hachage 2
  {
    return hash2(tab);
  }
  else
  {
    errno = EINVAL ;
    return -1;
  }
}

KV *kv_open (const char *dbnamec, const char *mode, int hidx, alloc_t alloc)
{
  int fd1, fd2, fd3, fd4;
  int longueur = 0;

  KV *kv = malloc(sizeof(kv));

  longueur = strlen(dbnamec);

  char *namec= malloc((longueur+2)*sizeof(char));
  char *nameblk= malloc((longueur+4)*sizeof(char));
  char *namekv= malloc((longueur+3)*sizeof(char));
  char *namedkv= malloc((longueur+4)*sizeof(char));

  strcpy(namec, dbnamec);
  strcpy(nameblk, dbnamec);
  strcpy(namekv, dbnamec);
  strcpy(namedkv, dbnamec);
  strcat(namec, ".h");
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

  char c_fd1, c_fd2, c_fd3, c_fd4;
  int lg_fd1, lg_fd2, lg_fd3, lg_fd4;
  lg_fd1=read(fd1, &c_fd1, 1);
  lg_fd2=read(fd2, &c_fd2, 1);
  lg_fd3=read(fd3, &c_fd3, 1);
  lg_fd4=read(fd4, &c_fd4, 1);

  if(lg_fd1 == -1 || lg_fd2 == -1 || lg_fd3 == -1 || lg_fd4 == -1)
  {
    return NULL;
  }
  char c1='h';
  char c2='b';
  char c3='k';
  char c4='d';

  if(lg_fd1 == 0 && (strcmp(mode, "w") || strcmp(mode, "w+")))
  {
    if(write(fd1, &c1, 1) == -1)
    {
      return NULL;
    }
  }
  if(lg_fd2 == 0 && (strcmp(mode, "w") || strcmp(mode, "w+")))
  {
    if(write(fd2, &c2, 1) == -1)
    {
      return NULL;
    }
  }
  if(lg_fd3 == 0 && (strcmp(mode, "w") || strcmp(mode, "w+")))
  {
    if(write(fd3, &c1, 1) == -1)
    {
      return NULL;
    }
  }
  if(lg_fd4 == 0 && (strcmp(mode, "w") || strcmp(mode, "w+")))
  {
    if(write(fd4, &c1, 1) == -1)
    {
      return NULL;
    }
  }
  if(c_fd1 != c1 || c_fd2 != c2 || c_fd3 != c3 || c_fd4 != c4 )
  {
    errno= EBADF;
    return NULL;
  }

  kv->fd1 = fd1;
  kv->fd2 = fd2;
  kv->fd3 = fd3;
  kv->fd4 = fd4;
  kv->hidx = hidx;
  kv->alloc = alloc;

  return kv;
}

int kv_close(KV *kv)
{
  close(kv->fd1);
  close(kv->fd2);
  close(kv->fd3);
  close(kv->fd4);
  free(kv);
  return 0;
}

int kv_get (KV *kv, const kv_datum *key, kv_datum *val)
{
  //initialisation des pointeurs de lecture
  if(lseek(kv->fd1, taille_header_f, SEEK_SET) <0) {return -1;}
  if(lseek(kv->fd2, taille_header_f, SEEK_SET) <0) {return -1;}
  if(lseek(kv->fd3, taille_header_f, SEEK_SET) <0) {return -1;}
  if(lseek(kv->fd4, taille_header_f, SEEK_SET) <0) {return -1;}

  // hasher la clé avec la bonne fonction
  int val_hash= hash(key->ptr, kv);

  // trouver le bloc de la clé
  len_t bloc_courant;
  if(lseek(kv->fd1, val_hash*32 , SEEK_CUR) <0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 32) != 32){return -1;}

  // lire l'en tête du bloc et trouver le bloc suivant
  len_t bloc_suiv= 0;
  if(lseek(kv->fd2, bloc_courant, SEEK_CUR) <0) {return -1;}
  read(kv->fd2, &bloc_suiv, 32);
  int i;
  // parcourir le bloc si pas trouver passer au bloc suivant
  // si trouver verifier que dans blk que ça n'a pas été suppr -> pas sur
  // recuperer la valeur
  if(val == NULL)
  {
    // allouer ici de la place pour val en fct de la longueur
  }
  return 0;
}

int main()
{
  return 0;
}
