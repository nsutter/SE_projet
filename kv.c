#include "kv.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define taille_header_f 5
#define taille_header_b 4

#define size_lent 10


int reset_lecture(KV* kv)
{
  //initialisation des pointeurs de lecture
  if(lseek(kv->fd1, taille_header_f, SEEK_SET) <0) {return -1;}
  if(lseek(kv->fd2, taille_header_f, SEEK_SET) <0) {return -1;}
  if(lseek(kv->fd3, taille_header_f, SEEK_SET) <0) {return -1;}
  if(lseek(kv->fd4, taille_header_f, SEEK_SET) <0) {return -1;}
  return 0;
}

/* Récupère la clé associé à un index en modifiant kv_datum key par effet de bord
*/
int readKey(KV *kv, kv_datum *key, len_t offset)
{
  len_t lg_cle;

  if(lseek(kv->fd3, offset, SEEK_SET) < 0) {return -1;}

  if(read(kv->fd3, &lg_cle, 4) < 0) {return -1;}; // on récupère la longueur de la clé

  key->len = lg_cle;

  key->ptr = malloc(lg_cle);

  if(read(kv->fd3, &key->ptr, lg_cle) < 0) {return -1;}; // on récupère la clé

  return lg_cle;
}

/* Récupère la valeur associé à un index en modifiant kv_datum val par effet de bord
*/
int readVal(KV *kv, kv_datum *val, len_t offset)
{
  len_t lg_cle, lg_val;

  if(lseek(kv->fd3, offset, SEEK_SET) < 0) {return -1;}

  if(read(kv->fd3, &lg_cle, 4) < 0) {return -1;}; // on récupère la longueur de la clé

  if(lseek(kv->fd3, lg_cle, SEEK_CUR) < 0) {return -1;}

  if(read(kv->fd3, &lg_val, 4) < 0) {return -1;}; // on récupère la longueur de la valeur

  val->len = lg_val;

  val->ptr = malloc(lg_val);

  if(read(kv->fd3, &val->ptr, lg_val) < 0) {return -1;} // on récupère la valeur

  return lg_val;
}

// http://www.jeuxvideo.com/forums/42-47-46456702-1-0-1-0-question-langage-c.htm
int writeData(KV *kv, kv_datum *key, kv_datum *val, len_t offset)
{

  if(lseek(kv->fd3, offset, SEEK_SET) < 0) {return -1;}

  if((write(kv->fd3, &(key->len), 4)) < 0) {return -1;}
  if((write(kv->fd3, key->ptr, key->len)) < 0) {return -1;}
  if((write(kv->fd3, &(val->len), 4)) < 0) {return -1;}
  if((write(kv->fd3, val->ptr, val->len)) < 0) {return -1;}

  return 1337;
}

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
  if(tab[0] == '0'){}
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
    errno = EINVAL;
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

int offset_cle(KV * kv, const kv_datum * key, len_t * offset)
{
  reset_lecture(kv);
  int val_hash = hash(key->ptr, kv);

  len_t bloc_courant, bloc_suivant;
  if(lseek(kv->fd1, val_hash*4 , SEEK_CUR) <0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 4) <0){return -1;}

  int boucle=0;
  char * cle_lue;

  while(boucle == 0)
  {
    if(lseek(kv->fd2, bloc_courant, SEEK_SET) <0){return -1;}
    if(read(kv->fd2, &bloc_suivant, 4) <0){return -1;}
    int i;
    for(i=0; i<1023; i++)
    {
      len_t lg_cle, pos_cle;
      if(read(kv->fd2, &pos_cle, 4) <0){return -1;}
      if(lseek(kv->fd3, pos_cle, SEEK_SET) <0){return -1;}
      if(read(kv->fd3, &lg_cle, 4) <0){return -1;}
      if(lg_cle == strlen(key->ptr))
      {
        cle_lue=malloc(lg_cle);
        if(read(kv->fd3, &cle_lue, lg_cle) <0){return -1;}
        if(strcmp(key->ptr, cle_lue))
        {
          free(cle_lue);
          *offset= lseek(kv->fd3, 0, SEEK_CUR);
          return 1;
        }
        else
        {
          free(cle_lue);
        }
      }
    }
    if(bloc_suivant != 0 && bloc_suivant != '\0')
    {
      bloc_courant=bloc_suivant;
    }
    else
    {
      boucle =1;
    }
  }
  return 0;
}

int kv_get (KV *kv, const kv_datum *key, kv_datum *val)
{
  len_t offset, offset_dkv;
  if(offset_cle(kv, key, &offset) == 1)
  {
    int existe=0;
    if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1){return -1;}
    while(read(kv->fd4, &existe, sizeof(int)) != EOF)
    {
      read(kv->fd4, NULL, 4);
      read(kv->fd4, &offset_dkv, 4);
      if(offset == offset_dkv && existe == 1)
      {
        if(val->len !=0)
          free(val->ptr);
        readVal(kv, val, offset);
        return 1;
      }
    }
    return 0;
  }
  return 0;
}

// Modification de len_t *offset par effet de bord qui doit être déjà alloué
int first_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  // On part du principe qu'on est après l'en tête de fd4
  int emplacement_libre = 0, i, nb_descripteur;

  len_t taille_requise = get_size(key) + get_size(val);

  len_t taille_courante = 0, offset_courant = 0;

  for(i = 0; i < nb_descripteur; i++)
  {
      if(read(kv->fd4, &emplacement_libre, 1) < 0) {return -1;}

      if(emplacement_libre == 0) // on vérifie si l'emplacement est libre
      {
        if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

        if(taille_requise <= taille_courante) // on vérifie maintenant si l'emplacement est assez grand
        {
          if(read(kv->fd4, &offset_courant, 4) < 0) {return -1;} // on récupère l'offset de l'emplacement

          *offset = offset_courant;

          return 42;
        }
        else
        {
          if(lseek(kv->fd4, 4, SEEK_CUR) < 0) {return -1;}
        }
      }
      else
      {
        if(lseek(kv->fd4, 8, SEEK_CUR) < 0) {return -1;}
      }
  }

  offset = NULL; // free(offset) ?

  return -1;
}

// Modification de len_t *offset par effet de bord qui doit être déjà alloué
int worst_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  // On part du principe qu'on est après l'en tête de fd4
  int emplacement_libre = 0, i, nb_descripteur;

  len_t taille_requise = get_size(key) + get_size(val);

  len_t taille_courante = 0, taille_max = 0, offset_courant = 0;

  for(i = 0; i < nb_descripteur; i++)
  {
      if(read(kv->fd4, &emplacement_libre, 1) < 0) {return -1;}

      if(emplacement_libre == 0) // on vérifie maintenant si l'emplacement est libre
      {
        if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

        if(taille_courante > taille_max) // on vérifie si l'emplacement est plus grand
        {
          if(read(kv->fd4, &offset_courant, 4) < 0) {return -1;} // on récupère l'offset de l'emplacement
        }
        else
        {
          if(lseek(kv->fd4, 4, SEEK_CUR) < 0) {return -1;}
        }
      }
      else
      {
        if(lseek(kv->fd4, 8, SEEK_CUR) < 0) {return -1;}
      }
  }

  if(offset_courant > 0)
  {
    *offset = offset_courant;

    return 42;
  }
  else
  {
    offset = NULL;

    return -1;
  }
}

// Modification de len_t *offset par effet de bord qui doit être déjà alloué
int best_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  // On part du principe qu'on est après l'en tête de fd4
  int emplacement_libre = 0, i, nb_descripteur;

  len_t taille_requise = get_size(key) + get_size(val);

  len_t taille_courante = 0, taille_min = 10000, offset_courant = 0; // taille max du fichier kv

  for(i = 0; i < nb_descripteur; i++)
  {
      if(read(kv->fd4, &emplacement_libre, 1) < 0) {return -1;}

      if(emplacement_libre == 0) // on vérifie si l'emplacement est libre
      {
        if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

        if(taille_requise <= taille_courante && taille_courante < taille_min) // on vérifie maintenant si l'emplacement est assez grand et plus petit
        {
          if(read(kv->fd4, &offset_courant, 4) < 0) {return -1;} // on récupère l'offset de l'emplacement

          *offset = offset_courant;

          return 42;
        }
        else
        {
          if(lseek(kv->fd4, 4, SEEK_CUR) < 0) {return -1;}
        }
      }
      else
      {
        if(lseek(kv->fd4, 8, SEEK_CUR) < 0) {return -1;}
      }
  }

  offset = NULL;

  return -1;
}

int get_offset(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  if(kv->alloc == 0)
  {
    return first_fit(kv, key, val, offset);
  }
  else if(kv->alloc == 1)
  {
    return worst_fit(kv, key, val, offset);
  }
  else if(kv->alloc == 2)
  {
    return best_fit(kv, key, val, offset);
  }
  else
  {
    errno = EINVAL;
    return -1;
  }
}

// positionne après l'entete du bloc
int read_entete_bloc(KV *kv, int nb_bloc)
{
  if(lseek(kv->fd2, taille_header_f + 4096 * nb_bloc, SEEK_SET) < 0) {return -1;}

  len_t nb_bloc_suivant;
  if(read(kv->fd2, &nb_bloc_suivant, 4) < 0) {return -1;}

  return nb_bloc_suivant;
}

int write_entete_bloc(KV *kv, int nb_bloc)
{
  if(lseek(kv->fd2, taille_header_f + 4096 * nb_bloc, SEEK_SET) < 0) {return -1;}

  //lecture max bloc
  max_bloc++;
  //write
}

int kv_put (KV *kv, const kv_datum *key, const kv_datum *val)
{
  reset_lecture(kv);

  // hachage de la clé
  int nb_bloc = hash(key->ptr, kv);

  int taille_bloc = (4096 - taille_header_b) / 4;

  len_t offset_bloc;

  if(kv_get(kv,key,NULL) == 1)
  { // la clé existe déjà => remplacement
    /* UTILISE DES FONCTIONS NON IMPLEMENTEES COMPLETEMENT
    if((kv_del(kv,key)) == -1) {return -1;};
    if((kv_put(kv,key,val)) == -1) {return -1;};

    return 42;
    */
  }
  else
  { // la clé n'existe pas => ajout
    len_t offset;

    if(get_offset(kv,key,val,&offset) == -1) {return -1;} // on récupère l'offset

    // trouver le bon emplacement dans le bloc pour stocker l'index

    int i, j, total_blocs, nb_bloc_suivant; // total_blocs à définir et récupérer dans l'entête

    for(i = 0; i < total_blocs; i++)
    {
      nb_bloc_suivant = read_entete_bloc(kv, nb_bloc);

      for(j = 0; j < taille_bloc; j++)
      {
        if(read(kv->fd2, &offset_bloc, 4) < 0) {return -1;}

        if(offset_bloc == '\0' || offset_bloc == 0) // emplacement OK
        {
          if(lseek(kv->fd2, -4, SEEK_CUR) < 0) {return -1;}

          if(write(kv->fd2, &offset_bloc, 4) < 0) {return -1;}

          writeData(kv, key, val, offset); // on écrit les données dans le fichier kv
          return 1337;
        }
      }

      if(nb_bloc_suivant != '\0')
      {
        nb_bloc = nb_bloc_suivant;
      }
      else // pas trouvé et pas de bloc suivant -> création + modifier en tête en tête
      {

      }
    }

    // est-ce qu'on peut arriver ici ?
  }

}


int kv_del(KV * kv, const kv_datum * key, kv_datum * val)
{
  if(reset_lecture(kv) == -1){return -1;}

  return 0;
}

int main()
{
  return 0;
}
