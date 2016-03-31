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
#define taille_header_b 4
#define TAILLE_BLOC 4096

void kv_start(KV* kv)
{
  lseek(kv->fd1, taille_header_f, SEEK_SET);
  lseek(kv->fd2, taille_header_f, SEEK_SET);
  lseek(kv->fd3, taille_header_f, SEEK_SET);
  lseek(kv->fd4, taille_header_f, SEEK_SET);
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
    fd1=open(namec, O_RDWR | O_CREAT, 0666);
    fd2=open(nameblk, O_RDWR | O_CREAT, 0666);
    fd3=open(namekv, O_RDWR | O_CREAT, 0666);
    fd4=open(namedkv, O_RDWR | O_CREAT, 0666);
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

//cherche dans la base a partir de la clé et renvoi l'offset correspondant
int offset_cle(KV * kv, const kv_datum * key, len_t * offset)
{
  kv_start(kv);
  int val_hash = hash(key->ptr, kv);

  len_t bloc_courant, bloc_suivant;
  if(lseek(kv->fd1, val_hash*4 , SEEK_CUR) <0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 4) <0){return -1;}

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
      if(lseek(kv->fd3, pos_cle, SEEK_SET) <0){return -1;}
      if(read(kv->fd3, &lg_cle, 4) <0){return -1;}
      if(lg_cle == strlen(key->ptr))
      {
        char * cle_lue=malloc(lg_cle);
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

len_t get_size(const kv_datum *data)
{
  return (sizeof(len_t) + data->len);
}

int write_descripteur(KV *kv, const len_t offset_dkv, const int est_occupe, const len_t longueur_couple, const len_t offset_couple)
{
  if(lseek(kv->fd4, taille_header_f + offset_dkv * (sizeof(int) + 2 * sizeof(len_t)), SEEK_SET) == -1) {return -1;}

  if(write(kv->fd4, &est_occupe, sizeof(int)) == -1) {return -1;}
  if(write(kv->fd4, &longueur_couple, sizeof(len_t)) == -1) {return -1;}
  if(write(kv->fd4, &offset_couple, sizeof(len_t)) == -1) {return -1;}

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
      if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

      if(taille_requise <= taille_courante) // on vérifie maintenant si l'emplacement est assez grand
      {
        if(read(kv->fd4, &offset_courant, 4) < 0) {return -1;} // on récupère l'offset de l'emplacement

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
  int emplacement_libre = 0, flag_while = 42;

  len_t taille_requise = get_size(key) + get_size(val);

  int int_descripteur_max = lseek(kv->fd4, 0, SEEK_END);

  if(int_descripteur_max == -1) {return -1;}

  len_t offset_descripteur_max = int_descripteur_max;

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;} // on se positionne après l'en-tête de fd4

  len_t taille_courante, taille_max = 0, taille_sauvegarde, offset_sauvegarde, offset_descripteur_sauvegarde;

  while(flag_while)
  {
    if(read(kv->fd4, &emplacement_libre, sizeof(int)) < 0) {return -1;}

    if(emplacement_libre == 0) // si l'emplacement est libre
    {
      if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

      if(taille_courante > taille_max) // on vérifie si l'emplacement est plus grand
      {
        taille_sauvegarde = taille_courante;

        offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + sizeof(len_t));

        if(read(kv->fd4, &offset_sauvegarde, 4) < 0) {return -1;} // on récupère l'offset de l'emplacement
      }
      else
      {
        if(lseek(kv->fd4, 4, SEEK_CUR) < 0) {return -1;}
      }
    }
    else if(emplacement_libre == 1) // si l'emplacement est occupé
    {
      if(lseek(kv->fd4, 8, SEEK_CUR) < 0) {return -1;}
    }
    else // si on est à la fin du fichier
    {
      flag_while = 0;
    }
  }

  if(offset_sauvegarde > 0)
  {
    // Modification du fichier dkv
    write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_sauvegarde - taille_requise, offset_sauvegarde);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde + (taille_sauvegarde - taille_requise));

    *offset = offset_sauvegarde + (taille_sauvegarde - taille_requise);

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
  int emplacement_libre = 0, flag_while = 42;

  len_t taille_requise = get_size(key) + get_size(val);

  int int_descripteur_max = lseek(kv->fd4, 0, SEEK_END);

  if(int_descripteur_max == -1) {return -1;}

  len_t offset_descripteur_max = int_descripteur_max;

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;} // on se positionne après l'en-tête de fd4

  len_t taille_courante, taille_min = UINT32_MAX, taille_sauvegarde,  offset_sauvegarde, offset_descripteur_sauvegarde;

  while(flag_while)
  {
      if(read(kv->fd4, &emplacement_libre, sizeof(int)) < 0) {return -1;}

      if(emplacement_libre == 0) // si l'emplacement est libre
      {
        if(read(kv->fd4, &taille_courante, 4) < 0) {return -1;}

        if(taille_requise <= taille_courante && taille_courante < taille_min) // on vérifie maintenant si l'emplacement est assez grand et plus petit
        {
          taille_sauvegarde = taille_courante;

          offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + sizeof(len_t));

          if(read(kv->fd4, &offset_sauvegarde, 4) < 0) {return -1;} // on récupère l'offset de l'emplacement
        }
        else
        {
          if(lseek(kv->fd4, 4, SEEK_CUR) < 0) {return -1;}
        }
      }
      else if(emplacement_libre == 1) // si l'emplacement est occupé
      {
        if(lseek(kv->fd4, 8, SEEK_CUR) < 0) {return -1;}
      }
      else // si on est à la fin du fichier
      {
        flag_while = 0;
      }
  }

  if(offset_sauvegarde > 0)
  {
    // Modification du fichier dkv
    write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_sauvegarde - taille_requise, offset_sauvegarde);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde + (taille_sauvegarde - taille_requise));

    *offset = offset_sauvegarde + (taille_sauvegarde - taille_requise);

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

  len_t bloc_courant, bloc_suivant;
  if(lseek(kv->fd1, val_hash*4 , SEEK_CUR) <0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 4) <0){return -1;}

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
      if(lseek(kv->fd3, pos_cle, SEEK_SET) <0){return -1;}
      if(read(kv->fd3, &lg_cle, 4) <0){return -1;}
      if(lg_cle == strlen(key->ptr))
      {
        char * cle_lue=malloc(lg_cle);
        if(read(kv->fd3, &cle_lue, lg_cle) <0){return -1;}
        if(strcmp(key->ptr, cle_lue))
        {
          free(cle_lue);
          len_t zero=0;
          if(lseek(kv->fd2, -4, SEEK_CUR) == -1){return -1;}
          if(write(kv->fd2, &zero, 4) <0 ){return -1;}

          len_t off_lue;
          while(read(kv->fd4, NULL, sizeof(int)))
          {
            if(read(kv->fd4, NULL, 4) <0){return -1;}
            if(read(kv->fd4, &off_lue, 4) <0){return -1;}
            if(off_lue == pos_cle)
            {
              if(lseek(kv->fd4, -4, SEEK_CUR) == -1){return -1;}
              if(write(kv->fd4, &zero, 4) <0){return -1;}
              return 0;
            }
          }
        }
        free(cle_lue);
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
  errno = ENOENT;
  return -1;
}

/* Écrit dans val_h la valeur à offset_h
*/
int read_h(KV *kv, len_t offset_h, len_t * val_h)
{
  kv_start(kv);

  if(lseek(kv->fd1, offset_h * sizeof(len_t), SEEK_CUR) < 0) {return -1;}

  if(read(kv->fd1, val_h, sizeof(len_t)) == -1) {return -1;}

  return 42;
}

/* Écrit une entrée dans le fichier h à l'index offset_h et de valeur offset_blk
*/
int write_h(KV *kv, len_t offset_h, len_t offset_blk)
{
  kv_start(kv);

  if(lseek(kv->fd1, offset_h * sizeof(len_t), SEEK_CUR) < 0) {return -1;}

  if(write(kv->fd1, &offset_blk, sizeof(len_t)) == -1) {return -1;}

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
  len_t offset_courant, offset_bloc_suivant, offset_sauvegarde = offset_bloc;

  int i;

  while(offset_bloc)
  {
    read_entete_bloc(kv, offset_bloc, &offset_bloc_suivant); // ce qui déplace juste apres l'en tete du bon bloc

    for(i = 0; i < TAILLE_BLOC - (int)sizeof(len_t); i++)
    {
      read(kv->fd2, &offset_courant, sizeof(len_t));

      if(offset_courant == 0)
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
  len_t val_h, offset_h = hash(key->ptr, kv);

  if(read_h(kv, offset_h, &val_h) == -1) {return -1;}

  if(val_h == '\0') // clé pas hachée
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

int kv_put(KV *kv, const kv_datum *key, const kv_datum *val)
{
  if(kv_get(kv,key,NULL) == 1)
  { // la clé existe déjà
    /*
    if((kv_del(kv,key)) == -1) {return -1;}
    if((kv_put(kv,key,val)) == -1) {return -1;}

    return 42;
    */
  }
  else
  { // la clé n'existe pas
    len_t offset;

    if(kv_put_dkv(kv, key, val, &offset) == -1) {return -1;} // on récupère l'offset

    if(kv_put_blk(kv, key, &offset) == -1) {return -1;}

    if(writeData(kv, key, val, offset) == -1) {return -1;}
  }

  return 93270;
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

int main()
{
  return 0;
}
