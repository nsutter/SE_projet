#include "kv.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>

#define taille_header_f 1
#define taille_header_b 4
#define TAILLE_BLOC 4096

void kv_start(KV *kv)
{

  lseek(kv->fd1, taille_header_f, SEEK_SET);
  lseek(kv->fd2, taille_header_f, SEEK_SET);
  lseek(kv->fd3, taille_header_f, SEEK_SET);
  lseek(kv->fd4, taille_header_f, SEEK_SET);
}

int write_first_dkv(KV *kv)
{
  int zero = 0;
  len_t lg = 4294967295;
  len_t offset = taille_header_f;
  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1){ return -1;}
  if(write(kv->fd4, &zero, sizeof(int)) == -1) {return -1;}
  if(write(kv->fd4, &lg, sizeof(len_t)) == -1) {return -1;}
  if(write(kv->fd4, &offset, sizeof(len_t)) == -1) {return -1;}

  return 42;
}

// Récupère la clé associé à un index en modifiant kv_datum key par effet de bord
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

// Récupère la valeur associé à un index en modifiant kv_datum val par effet de bord
int readVal(KV *kv, kv_datum *val, len_t offset)
{
  len_t lg_val;

  if(lseek(kv->fd3, offset, SEEK_SET) < 0) {return -1;}

  if(read(kv->fd3, &lg_val, 4) < 0) {return -1;}; // on récupère la longueur de la valeur

  if(val->ptr == NULL)
  {
    val->len = lg_val;
    val->ptr = malloc(lg_val);
  }

  int val_retour;
  if(val->len >= lg_val)
    val_retour = read(kv->fd3, val->ptr, lg_val);
  else
    val_retour = read(kv->fd3, val->ptr, val->len);


  return val_retour;
}

int writeData(KV *kv, const kv_datum *key, const kv_datum *val, len_t offset)
{

  if(lseek(kv->fd3, offset, SEEK_SET) < 0) {return -1;}

  if((write(kv->fd3, &(key->len), 4)) < 0) {return -1;}
  if((write(kv->fd3, key->ptr, key->len)) < 0) {return -1;}
  if((write(kv->fd3, &(val->len), 4)) < 0) {return -1;}
  if((write(kv->fd3, val->ptr, val->len)) < 0) {return -1;}

  return 1337;
}

int hash0(const char tab[])
{
  int i;
  int somme = 0;

  for(i = 0; tab[i] != '\0'; i++)
  {
    somme += tab[i];
  }

  return(somme % 999983);
}

int hash1(const char tab[])
{
  int i;
  int somme = 0;

  for(i = 0; tab[i] != '\0'; i++)
  {
    somme += (i + 1) * tab[i];
  }

  return(somme % 999983);
}

int hash(const char tab[], KV *kv)
{
  if(kv->hidx == 0) // fonction de hachage 0
  {
    return hash0(tab);
  }
  else if(kv->hidx == 1) // fonction de hachage 1
  {
    return hash1(tab);
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

  KV *kv= malloc(sizeof(struct KV));

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

  if(strcmp(mode, "r") == 0)
  {
    fd1=open(namec, O_RDONLY);
    if(fd1 == -1){ perror(""); return NULL;}
    fd2=open(nameblk, O_RDONLY);
    if(fd2 == -1){ perror(""); return NULL;}
    fd3=open(namekv, O_RDONLY);
    if(fd3 == -1){ perror(""); return NULL;}
    fd4=open(namedkv, O_RDONLY);
    if(fd4 == -1){ perror(""); return NULL;}
  }
  else if(strcmp(mode, "r+") == 0)
  {
    fd1=open(namec, O_RDWR | O_CREAT, 0666);
    if(fd1 == -1){ perror(""); return NULL;}
    fd2=open(nameblk, O_RDWR | O_CREAT, 0666);
    if(fd2 == -1){ perror(""); return NULL;}
    fd3=open(namekv, O_RDWR | O_CREAT, 0666);
    if(fd3 == -1){ perror(""); return NULL;}
    fd4=open(namedkv, O_RDWR | O_CREAT, 0666);
    if(fd4 == -1){ perror(""); return NULL;}
  }
  else if(strcmp(mode, "w") == 0)
  {
    fd1=open(namec, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd1 == -1){ perror(""); return NULL;}
    fd2=open(nameblk, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd2 == -1){ perror(""); return NULL;}
    fd3=open(namekv, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd3 == -1){ perror(""); return NULL;}
    fd4=open(namedkv, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd4 == -1){ perror(""); return NULL;}
  }
  else if(strcmp(mode, "w+") == 0)
  {
    fd1=open(namec, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd1 == -1){ perror(""); return NULL;}
    fd2=open(nameblk, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd2 == -1){ perror(""); return NULL;}
    fd3=open(namekv, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd3 == -1){ perror(""); return NULL;}
    fd4=open(namedkv, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd4 == -1){ perror(""); return NULL;}
  }

  kv->fd1 = fd1;
  kv->fd2 = fd2;
  kv->fd3 = fd3;
  kv->fd4 = fd4;
  kv->hidx = hidx;
  kv->alloc = alloc;

  free(namec);
  free(nameblk);
  free(namekv);
  free(namedkv);

  char c_fd1, c_fd2, c_fd3, c_fd4;

  int lg_fd1, lg_fd2, lg_fd3, lg_fd4;

  if(lseek(fd1, 0, SEEK_SET) == -1){return NULL;}
  if(lseek(fd2, 0, SEEK_SET) == -1){return NULL;}
  if(lseek(fd3, 0, SEEK_SET) == -1){return NULL;}
  if(lseek(fd4, 0, SEEK_SET) == -1){return NULL;}

  lg_fd1 = read(fd1, &c_fd1, 1);
  lg_fd2 = read(fd2, &c_fd2, 1);
  lg_fd3 = read(fd3, &c_fd3, 1);
  lg_fd4 = read(fd4, &c_fd4, 1);

  if(lg_fd1 == -1 || lg_fd2 == -1 || lg_fd3 == -1 || lg_fd4 == -1)
  {
    return NULL;
  }

  char c1='h';
  char c2='b';
  char c3='k';
  char c4='d';

  if(lg_fd1 == 0 && (strcmp(mode, "w") == 0 || strcmp(mode, "w+") == 0 || strcmp(mode, "r+") == 0))
  {
    if(write(fd1, &c1, 1) == -1)
    {
      return NULL;
    }
  }

  if(lg_fd2 == 0 && (strcmp(mode, "w") == 0 || strcmp(mode, "w+") == 0 || strcmp(mode, "r+") == 0))
  {
    if(write(fd2, &c2, 1) == -1)
    {
      return NULL;
    }
  }
  if(lg_fd3 == 0 && (strcmp(mode, "w") == 0 || strcmp(mode, "w+") == 0 || strcmp(mode, "r+") == 0))
  {
    if(write(fd3, &c3, 1) == -1)
    {
      return NULL;
    }
  }
  if(lg_fd4 == 0 && (strcmp(mode, "w") == 0 || strcmp(mode, "w+") == 0 || strcmp(mode, "r+") == 0))
  {
    if(write(fd4, &c4, 1) == -1)
    {
      return NULL;
    }
    if(write_first_dkv(kv) == -1) {return NULL;}
  }

  if(lseek(fd1, 0, SEEK_SET) == -1){return NULL;}
  if(lseek(fd2, 0, SEEK_SET) == -1){return NULL;}
  if(lseek(fd3, 0, SEEK_SET) == -1){return NULL;}
  if(lseek(fd4, 0, SEEK_SET) == -1){return NULL;}

  lg_fd1 = read(fd1, &c_fd1, 1);
  lg_fd2 = read(fd2, &c_fd2, 1);
  lg_fd3 = read(fd3, &c_fd3, 1);
  lg_fd4 = read(fd4, &c_fd4, 1);

  if(lg_fd1 == -1 || lg_fd2 == -1 || lg_fd3 == -1 || lg_fd4 == -1)
  {
    return NULL;
  }

  if(c_fd1 != c1 || c_fd2 != c2 || c_fd3 != c3 || c_fd4 != c4 )
  {
    errno = EBADF;
    return NULL;
  }

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

//cherche dans la base a partir de la clé et renvoi l'offset correspondant
int offset_cle(KV * kv, const kv_datum * key, len_t * offset)
{
  kv_start(kv);

  int val_hash = hash(key->ptr, kv);

  len_t bloc_courant, bloc_suivant=0;
  if(lseek(kv->fd1, val_hash * sizeof(len_t) , SEEK_CUR) < 0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 4) < 0){return -1;}

  if(!bloc_courant)
  {
    return 0;
  }

  int boucle = 0;

  while(boucle == 0)
  {
    if(lseek(kv->fd2, bloc_courant, SEEK_SET) <0){return -1;}
    if(read(kv->fd2, &bloc_suivant, 4) <0){return -1;}
    int i;
    for(i=0; i<1023; i++)
    {
      len_t lg_cle, pos_cle;
      if(read(kv->fd2, &pos_cle, 4) <0){return -1;}
      if(pos_cle != 0)
      {
        if(lseek(kv->fd3, pos_cle, SEEK_SET) <0){return -1;}
        if(read(kv->fd3, &lg_cle, 4) <0){return -1;}
        if(lg_cle == key->len)
        {
          char * cle_lue=malloc(lg_cle);
          if(read(kv->fd3, cle_lue, lg_cle) <0){return -1;}
          if(strcmp(key->ptr, cle_lue) == 0)
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
    }
    if(bloc_suivant && bloc_suivant !=0)
    {
      printf("pas bon\n");
      bloc_courant = bloc_suivant;
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
  len_t offset;
  if(offset_cle(kv, key, &offset) == 1)
  {
    return (readVal(kv, val, offset));
  }
  return 0;
}

len_t get_size(const kv_datum *data)
{
  return (sizeof(len_t) + data->len);
}

int write_descripteur(KV *kv, const len_t offset_dkv, const int est_occupe, const len_t longueur_couple, const len_t offset_couple)
{
   printf("\twrite_descripteur : offset_dkv = %" PRIu16 "\n",offset_dkv);
   printf("\twrite_descripteur : est_occupe = %d\n",est_occupe);
   printf("\twrite_descripteur : longueur_couple = %" PRIu16 "\n",longueur_couple);
   printf("\twrite_descripteur : offset_couple = %" PRIu16 "\n",offset_couple);

  if(lseek(kv->fd4, offset_dkv, SEEK_SET) == -1) {return -1;}
   off_t i1 = lseek(kv->fd4, 0, SEEK_CUR);
   printf("\t\tposition write est_occupe %jd\n",(intmax_t)i1);
  write(kv->fd4, &est_occupe, sizeof(int));// == -1) {return -1;}
   off_t i2 = lseek(kv->fd4, 0, SEEK_CUR);
   printf("\t\tposition write longueur_couple %jd\n",(intmax_t)i2);
  write(kv->fd4, &longueur_couple, sizeof(len_t));// == -1) {return -1;}
   off_t i3 = lseek(kv->fd4, 0, SEEK_CUR);
   printf("\t\tposition write offset_couple %jd\n",(intmax_t)i3);
  write(kv->fd4, &offset_couple, sizeof(len_t));// == -1) {return -1;}

  return 1;
}

// Modification de len_t *offset par effet de bord qui doit être déjà alloué
// Écriture dans dkv
int first_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  int emplacement_libre = 0, flag_while = 42;

  len_t taille_requise = get_size(key) + get_size(val);

  int int_descripteur_max = lseek(kv->fd4, 0, SEEK_END);

  if(int_descripteur_max == -1) {return -1;}

  len_t offset_descripteur_max = int_descripteur_max;

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;} // on se positionne après l'en-tête de fd4

  len_t taille_courante, offset_courant, offset_descripteur_courant;

  while(flag_while)
  {
    offset_descripteur_courant = lseek(kv->fd4, 0, SEEK_CUR);

    if(read(kv->fd4, &emplacement_libre, sizeof(int)) < 0) {return -1;}

    if(emplacement_libre == 0) // si l'emplacement est libre
    {
      if(read(kv->fd4, &taille_courante, sizeof(len_t)) < 0) {return -1;}

      if(taille_requise <= taille_courante) // on vérifie maintenant si l'emplacement est assez grand
      {
        if(read(kv->fd4, &offset_courant, sizeof(len_t)) < 0) {return -1;} // on récupère l'offset de l'emplacement

        // Modification du fichier dkv
        write_descripteur(kv, offset_descripteur_courant, 0, taille_courante - taille_requise, offset_courant);
        write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_courant + (taille_courante - taille_requise));

        *offset = offset_courant + (taille_courante - taille_requise);

        return 42;
      }
      else
      {
        if(lseek(kv->fd4, 4, SEEK_CUR) < 0) {return -1;}
      }
    }
    else if(emplacement_libre == 1) // si l'emplacement est occupé
    {
      if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) < 0) {return -1;}
    }
    else // si on est à la fin du fichier
    {
      flag_while = 0;
    }
  }

  offset = NULL; // free(offset)

  return -1;
}

// Modification de len_t *offset par effet de bord qui doit être déjà alloué
// Écriture dans dkv
int worst_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  int emplacement_libre = 0, flag_while = 42, i;

  len_t taille_requise = get_size(key) + get_size(val);

  int int_descripteur_max = lseek(kv->fd4, 0, SEEK_END);

  if(int_descripteur_max == -1) {return -1;}

  len_t offset_descripteur_max = int_descripteur_max;

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;} // on se positionne après l'en-tête de fd4

  len_t taille_courante, taille_max = 0, offset_sauvegarde, offset_descripteur_sauvegarde;

  while(flag_while)
  {
    i = read(kv->fd4, &emplacement_libre, sizeof(int));

    if(i == 0)
    {
      flag_while = 0;
    }
    else
    {
      if(emplacement_libre == 0) // si l'emplacement est libre
      {
        if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

        if(taille_courante > taille_max) // on vérifie si l'emplacement est plus grand
        {
          taille_max = taille_courante;

          offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + sizeof(len_t));

          if(read(kv->fd4, &offset_sauvegarde, sizeof(len_t)) < 0) {return -1;} // on récupère l'offset de l'emplacement
        }
        else
        {
          if(lseek(kv->fd4, sizeof(len_t), SEEK_CUR) < 0) {return -1;}
        }
      }
      else if(emplacement_libre == 1) // si l'emplacement est occupé
      {
        if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) < 0) {return -1;}
      }
    }
  }

  if(offset_sauvegarde > 0)
  {
    // Modification du fichier dkv
    write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_max - taille_requise, offset_sauvegarde);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde + (taille_max - taille_requise));

    *offset = offset_sauvegarde + (taille_max - taille_requise);

    return 42;
  }
  else
  {
    offset = NULL;

    return -1;
  }
}

// Modification de len_t *offset par effet de bord qui doit être déjà alloué
// Écriture dans dkv
int best_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  int emplacement_libre = 0, flag_while = 42, i;

  len_t taille_requise = get_size(key) + get_size(val);

  int int_descripteur_max = lseek(kv->fd4, 0, SEEK_END);

  if(int_descripteur_max == -1) {return -1;}

  len_t offset_descripteur_max = int_descripteur_max;

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;} // on se positionne après l'en-tête de fd4

  len_t taille_courante, taille_min = UINT32_MAX,  offset_sauvegarde, offset_descripteur_sauvegarde;

  while(flag_while)
  {
      i = read(kv->fd4, &emplacement_libre, sizeof(int));

      if(i == 0)
      {
        flag_while = 0;
      }
      {
        if(emplacement_libre == 0) // si l'emplacement est libre
        {
          if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

          if(taille_requise <= taille_courante && taille_courante < taille_min) // on vérifie maintenant si l'emplacement est assez grand et plus petit
          {
            taille_min = taille_courante;

            offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + sizeof(len_t));

            if(read(kv->fd4, &offset_sauvegarde, sizeof(len_t)) < 0) {return -1;} // on récupère l'offset de l'emplacement
          }
          else
          {
            if(lseek(kv->fd4, sizeof(len_t), SEEK_CUR) < 0) {return -1;}
          }
        }
        else if(emplacement_libre == 1) // si l'emplacement est occupé
        {
          if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) < 0) {return -1;}
        }
      }
  }

  if(offset_sauvegarde > 0)
  {
    // Modification du fichier dkv
    write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_min - taille_requise, offset_sauvegarde);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde + (taille_min - taille_requise));

    *offset = offset_sauvegarde + (taille_min - taille_requise);

    return 42;
  }
  else
  {
    offset = NULL;

    return -1;
  }
}

// gestion complète DKV
int kv_put_dkv(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
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

// Écrit l'offset du bloc suivant len_t ancien_offset dans len_t * nouveau_offset
int read_entete_bloc(KV *kv, const len_t offset_bloc, len_t * nouveau_offset)
{
  if(lseek(kv->fd2, offset_bloc, SEEK_SET) < 0) {return -1;}

  if(read(kv->fd2, nouveau_offset, sizeof(len_t)) < 0) {return -1;}

  return 42;
}

// Écrit len_t * nouveau_offset dans l'en-tête du bloc len_t offset_bloc
int write_entete_bloc(KV *kv, const len_t offset_bloc, const len_t * nouveau_offset)
{
  if(lseek(kv->fd2, offset_bloc, SEEK_SET) < 0) {return -1;}

  if(write(kv->fd2, nouveau_offset, sizeof(len_t)) < 0) {return -1;}

  return 42;
}


int kv_del(KV * kv, const kv_datum * key)
{
  kv_start(kv);
  int val_hash = hash(key->ptr, kv);

  len_t bloc_courant, bloc_suivant=0;
  if(lseek(kv->fd1, val_hash*sizeof(len_t) , SEEK_CUR) <0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 4) <0){return -1;}

  if(!bloc_courant)
  {
    return 0;
  }

  int boucle=0;

  while(boucle == 0)
  {
    if(lseek(kv->fd2, bloc_courant, SEEK_SET) <0){return -1;}
    if(read(kv->fd2, &bloc_suivant, 4) <0){return -1;}
    int i;
    for(i=0; i<1023; i++)
    {
      len_t lg_cle, pos_cle;
      if(read(kv->fd2, &pos_cle, 4) <0){return -1;}
      if(pos_cle !=0)
      {
        if(lseek(kv->fd3, pos_cle, SEEK_SET) <0){return -1;}
        if(read(kv->fd3, &lg_cle, 4) <0){return -1;}
        if(lg_cle == key->len)
        {
          char * cle_lue=malloc(lg_cle);
          if(read(kv->fd3, cle_lue, lg_cle) <0){return -1;}

          if(strcmp(key->ptr, cle_lue) == 0)
          {
            free(cle_lue);
            len_t zero=0;
            if(lseek(kv->fd2, -4, SEEK_CUR) == -1){return -1;}
            if(write(kv->fd2, &zero, 4) <0 ){return -1;}

            len_t off_lue;
            int libre;
            lseek(kv->fd4, taille_header_f, SEEK_SET);
            while(read(kv->fd4, &libre, sizeof(int)))
            {
              if(lseek(kv->fd4, 4, SEEK_CUR) <0){return -1;}
              if(read(kv->fd4, &off_lue, 4) <0){return -1;}
              printf("pos_cle :%"PRIu16"\n", pos_cle);
              printf("off_lue :%"PRIu16"\n", off_lue);
              if(off_lue == pos_cle)
              {
                if(lseek(kv->fd4, -4, SEEK_CUR) == -1){return -1;}
                if(write(kv->fd4, &zero, 4) <0){return -1;}
                return 0;
              }
            }
          }
          else
            free(cle_lue);
        }
      }
    }
    if(bloc_suivant && bloc_suivant !=0)
    {
      bloc_courant = bloc_suivant;
    }
    else
    {
      boucle =1;
    }
  }
  errno = ENOENT;
  return -1;
}

/* Écrit dans val_h la valeur à offset_h -> semble OK
*/
int read_h(KV *kv, len_t offset_h, len_t * val_h)
{
  kv_start(kv);

  if(lseek(kv->fd1, offset_h * sizeof(len_t), SEEK_CUR) < 0) {return -1;}

  int n = read(kv->fd1, val_h, sizeof(len_t));

  if(n == -1) {return -1;}

  return n;
}

/* Écrit une entrée dans le fichier h à l'index offset_h et de valeur offset_blk
*/
int write_h(KV *kv, len_t offset_h, len_t offset_blk)
{
  kv_start(kv);

  if(lseek(kv->fd1, offset_h * sizeof(len_t), SEEK_CUR) < 0) {return -1;}

  write(kv->fd1, &offset_blk, sizeof(len_t));// == -1) {return -1;}

  return 42;
}

/* Création d'un nouveau bloc avec que des 0 et écrit l'index dans offset_nouveau_bloc par effet de bord
*/
int new_bloc(KV *kv, len_t * offset_nouveau_bloc)
{
  int int_descripteur_max = lseek(kv->fd2, 0, SEEK_END);

  if(int_descripteur_max == -1) {return -1;}

  len_t offset_descripteur_max = int_descripteur_max;

  int i;

  len_t zero = 0;

  for(i = 0; i < TAILLE_BLOC; i++)
  {
    if(write(kv->fd2, &zero, sizeof(len_t)) == -1) {return -1;}
  }

  *offset_nouveau_bloc = offset_descripteur_max;

  return 42;
}

int write_bloc_entry(KV *kv, len_t offset_entry, len_t offset_data)
{
  if(lseek(kv->fd2, offset_entry, SEEK_SET) < 0) {return -1;}

  if(write(kv->fd2, &offset_data, sizeof(len_t)) == -1) {return -1;}

  return 42;
}

int write_bloc(KV *kv, len_t offset_bloc, len_t * offset_data)
{
  len_t offset_lue_courant, offset_courant, offset_bloc_suivant, offset_sauvegarde = offset_bloc;

  int i;
  off_t off;

  while(offset_bloc)
  {
    read_entete_bloc(kv, offset_bloc, &offset_bloc_suivant); // ce qui déplace juste apres l'en tete du bon bloc

    for(i = 0; i < TAILLE_BLOC - (int)sizeof(len_t); i++)
    {
      off = lseek(kv->fd2, 0, SEEK_CUR);

      if(off == -1)
      {
        return -1;
      }

      offset_courant = off;

      read(kv->fd2, &offset_lue_courant, sizeof(len_t));

      if(offset_lue_courant == 0)
      {
        write_bloc_entry(kv, offset_courant, *offset_data);
        return 42;
      }
    }

    offset_bloc = offset_bloc_suivant;
  }

  // PAS DE BLOC SUIVANT ET PAS ECRIT
  len_t offset_new_bloc;

  if(new_bloc(kv, &offset_new_bloc)) {return -1;} // creation d'un nouveau bloc et rappel

  if(write_bloc(kv, offset_new_bloc, offset_data)) {return -1;};

  if(write_bloc_entry(kv, offset_new_bloc, offset_sauvegarde)) {return -1;}; // lien entre les 2 blocs ocamlus

  return 42;
}

int kv_put_blk(KV *kv, const kv_datum *key, len_t *offset_key)
{
  len_t val_h, offset_h;

  int offset_int = hash(key->ptr, kv);

  if(offset_int == -1)
  {
    return -1;
  }
  else
  {
    offset_h = offset_int;
  }

  int n = read_h(kv, offset_h, &val_h);

  if(n == -1){return -1;}

  if(!n || val_h == 0) // clé pas hachée
  {
    len_t offset_new_bloc;

    if(new_bloc(kv, &offset_new_bloc) == -1) {return -1;}
    if(write_h(kv, offset_h, offset_new_bloc) == -1) {return -1;}
    if(write_bloc(kv, offset_new_bloc, offset_key) == -1) {return -1;}

  }
  else // clé déjà hachée
  {
    if(write_bloc(kv, offset_h, offset_key) == -1) {return -1;}
  }

  return 42;
}

int kv_put (KV *kv, const kv_datum *key, const kv_datum *val)
{
  //printf("start kv_put : %s/%s\n",key->ptr,val->ptr);
  len_t offset_tmp;

  if(offset_cle(kv,key,&offset_tmp) == 1)
  { // la clé existe déjà
    if((kv_del(kv,key)) == -1) {return -1;}
    if((kv_put(kv,key,val)) == -1) {return -1;}

    return 42;
  }
  else
  { // la clé n'existe pas
    len_t offset;

    if(kv_put_dkv(kv, key, val, &offset) == -1) {return -1;} // on récupère l'offset
    printf("end kv_put_dkv\n");
    if(kv_put_blk(kv, key, &offset) == -1) {return -1;}
    printf("end kv_put_blk\n");
    if(writeData(kv, key, val, offset) == -1) {return -1;}
    printf("end writeData\n");
  }

  return 42;
}

int kv_next(KV *kv, kv_datum *key, kv_datum *val)
{
  int est_occupe, flag_while = 666;

  len_t longueur_courante, offset_courant;

  while(flag_while)
  {
    if(read(kv->fd4, &est_occupe, sizeof(int)) == -1) {return -1;}

    if(est_occupe == 0) // vide
    {
      if(read(kv->fd4, &longueur_courante, sizeof(len_t)) == -1) {return -1;}
      if(read(kv->fd4, &offset_courant, sizeof(len_t)) == -1) {return -1;}

      flag_while = 0;
    }
    else if(est_occupe == 1) // occupe
    {
      if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) == -1) {return -1;}
    }
    else // fin
    {
      return 0;
    }
  }

  if(key->len != 0)
  {
    free(key->ptr);
  }

  if(readKey(kv, key, offset_courant) == -1) {return -1;}

  if(val->len != 0)
  {
    free(val->ptr);
  }

  if(readVal(kv, val, offset_courant) == -1) {return -1;}

  return 42;
}

// int main()
// {
//
//   KV * kv = kv_open("toast", "w+", 0, BEST_FIT);
//
//   if(kv == NULL)
//   {
//     printf("kv = NULL\n");
//   }
//
//   kv_datum key, val, val2;
//   val2.ptr = NULL;
//   val.len =0;
//
//   key.ptr = (char *) malloc(5);
//
//   key.ptr = "yolo";
//   key.len = 5;
//
//   val.ptr = (char *) malloc(5);
//
//   val.ptr = "bite";
//   val.len = 5;
//
//   kv_put(kv,&key,&val);
//
//   key.ptr = "yala";
//
//   val.ptr = "teub";
//
//   kv_put(kv,&key,&val);
//
//   int i = kv_get(kv,&key,&val2);
//
//   if(i == 0)
//     printf("pas trouvé\n");
//   else
//   {
//     printf("trouvé\n");
//     printf("end kv_get : %s \n", val2.ptr);
//   }
//
//   printf("\n\nJE DELETE ICI \n\n");
//
//   kv_del(kv,&key);
//
//   i = kv_get(kv,&key,&val2);
//
//   if(i == 0)
//     printf("pas trouvé\n");
//   else
//   {
//     printf("trouvé\n");
//     printf("end kv_get : %s \n", val2.ptr);
//   }
//
//   kv_close(kv);
//
//   return 0;
// }
