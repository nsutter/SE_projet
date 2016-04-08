#include "kv.h"
#include "common.h"
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
#include <math.h> // compiler avec gcc -lm

#define taille_header_f 1
#define taille_header_b 4
#define TAILLE_BLOC 4096

#define NBIT(c,n) (((c) >> (n)) & 1) // macro qui détermine le n-ième bit d'un caractère

/*
 * @brief Initialisation des têtes de lecture après les en-têtes
 *
 * @param kv descripteur d'accès à la base
 */
void kv_start(KV *kv)
{

  lseek(kv->fd1, taille_header_f, SEEK_SET);
  lseek(kv->fd2, taille_header_f, SEEK_SET);
  lseek(kv->fd3, taille_header_f, SEEK_SET);
  lseek(kv->fd4, taille_header_f, SEEK_SET);
}

/*
 * @brief Initialisation du .dkv
 *
 * @param kv descripteur d'accès à la base
 */
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

/*
 * @brief Récupère la valeur associé à un index dans le .kv
 *
 * @param kv descripteur d'accès à la base
 * @param val valeur modifiée par effet de bord
 * @param offset index dans le .kv
 */
int readVal(KV *kv, kv_datum *val, len_t offset)
{
  len_t lg_val;

  if(lseek(kv->fd3, offset, SEEK_SET) < 0) {return -1;}

  if(read(kv->fd3, &lg_val, 4) < 0) {return -1;}

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

/*
 * @brief Écrit un couple key/val dans le .kv
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param val valeur
 * @param offset index dans le .kv
 */
int writeData(KV *kv, const kv_datum *key, const kv_datum *val, len_t offset)
{
  if(lseek(kv->fd3, offset, SEEK_SET) < 0) {return -1;}

  if((write(kv->fd3, &(key->len), sizeof(len_t))) < 0) {return -1;}
  if((write(kv->fd3, key->ptr, key->len)) < 0) {return -1;}
  if((write(kv->fd3, &(val->len), sizeof(len_t))) < 0) {return -1;}
  if((write(kv->fd3, val->ptr, val->len)) < 0) {return -1;}

  return 1337;
}

/*
 * @brief 1ère fonction de hachage
 *
 * @param tab[]
 */
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

/*
 * @brief 2ème fonction de hachage
 *
 * @param tab[]
 */
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

/*
 * @brief 3ème fonction de hachage limitée pour le moment à des clés de 32 caractères
 *
 * @param tab[]
 */
// int hash2(const char tab[])
// {
//     int lg_tab = strlen(tab);
//
//     int lg_bit = lg_tab * 8;
//
//     double yolo = (double)lg_bit/(double)32;
//
//     int n_bit = (int)ceil(yolo);
//
//     int i, j, k, hash = 0;
//
//     for(i = 0, j = 0, k = 0; i < lg_tab && k < 32; k++)
//     {
//       hash += NBIT(tab[i],j) * pow( 2, k);
//
//       j += n_bit;
//
//       if(j > 7)
//       {
//         i++;
//         j = j%8;
//       }
//     }
//
//     return hash % 999983;
// }

/*
 * @brief Détermine la fonction de hachage
 *
 * @param kv descripteur d'accès à la base
 * @param tab[]
 */
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

/*
 * @brief Ouvre une base
 *
 * @param dbnamec nom de la base
 * @param mode mode d'ouverture
 * @param hidx fonction de hachage
 * @param alloc mode d'allocation
 */
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

/*
 * @brief Termine l'accès à la base
 *
 * @param kv descripteur d'accès à la base
 */
int kv_close(KV *kv)
{
  close(kv->fd1);
  close(kv->fd2);
  close(kv->fd3);
  close(kv->fd4);
  free(kv);
  return 0;
}

/*
 * @brief Recherche de la clé dans la base et renvoi de l'offset correspondant
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param offset index modifié par effet de bord
 */
int offset_cle(KV * kv, const kv_datum * key, len_t * offset)
{
  kv_start(kv);

  int val_hash = hash(key->ptr, kv);

  len_t bloc_courant, bloc_suivant=0;
  if(lseek(kv->fd1, val_hash * sizeof(len_t) , SEEK_CUR) < 0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 4) < 0){return -1;}

  if(!bloc_courant)
  {
    errno = ENOENT;
    return -1;
  }

  int boucle = 0;

  while(boucle == 0)
  {
    if(lseek(kv->fd2, bloc_courant, SEEK_SET) < 0) {return -1;}
    if(read(kv->fd2, &bloc_suivant, 4) < 0) {return -1;}
    int i;
    for(i=0; i<1023; i++)
    {
      len_t lg_cle, pos_cle;
      if(read(kv->fd2, &pos_cle, 4) < 0) {return -1;}
      if(pos_cle != 0)
      {

        if(lseek(kv->fd3, pos_cle, SEEK_SET) < 0) {return -1;}
        if(read(kv->fd3, &lg_cle, 4) < 0) {return -1;}
        if(lg_cle == key->len)
        {
          char * cle_lue = malloc(lg_cle + 1);

          if(read(kv->fd3, cle_lue, lg_cle) < 0) {return -1;}

          cle_lue[lg_cle]= '\0';

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
      bloc_courant = bloc_suivant;
    }
    else
    {
      boucle =1;
    }
  }
  return -1;
}

/*
 * @brief Recherche de la clé dans la base
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param val valeur modifiée par effet de bord
 */
int kv_get(KV *kv, const kv_datum *key, kv_datum *val)
{
  len_t offset;
  if(offset_cle(kv, key, &offset) == 1)
  {
    return (readVal(kv, val, offset));
  }

  return 0;
}

/*
 * @brief Renvoie la longueur totale de data
 *
 * @param data
 */
len_t get_size(const kv_datum *data)
{
  return (sizeof(len_t) + data->len);
}

/*
 * @brief Écrit un descripteur dans le .dkv
 *
 * @param kv descripteur d'accès à la base
 * @param offset_dkv index dans le .dkv
 * @param est_occupe entier (emplacement libre ou non libre)
 * @param longueur_couple len_t (longueur du couple dans le .kv)
 * @param offset_couple len_t (offset du couple dans le .kv)
 */
int write_descripteur(KV *kv, const len_t offset_dkv, const int est_occupe, const len_t longueur_couple, const len_t offset_couple)
{
  if(lseek(kv->fd4, offset_dkv, SEEK_SET) == -1) {return -1;}

  if(write(kv->fd4, &est_occupe, sizeof(int)) == -1) {return -1;}

  if(write(kv->fd4, &longueur_couple, sizeof(len_t)) == -1) {return -1;}

  if(write(kv->fd4, &offset_couple, sizeof(len_t)) == -1) {return -1;}

  return 1;
}

// len_t * offset_dkv modifié par effet de bord
int search_pos_dkv(KV *kv, len_t *offset_dkv)
{
  kv_start(kv);

  len_t emplacement_libre;

  off_t offset;

  while(read(kv->fd4, &emplacement_libre, sizeof(int)))
  {
    if(emplacement_libre == 2)
    {
      offset = lseek(kv->fd4, 0, SEEK_CUR);

      if(offset == -1)
      {
        return -1;
      }
      else
      {
        *offset_dkv = offset - sizeof(int);

        return 42;
      }
    }
    else
    {
      if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) < 0) {return -1;}
    }
  }

  off_t int_descripteur_max = lseek(kv->fd4, 0, SEEK_END);

  if(int_descripteur_max == -1) {return -1;}

  *offset_dkv = (len_t)int_descripteur_max;

  return 42;
}

/*
 * @brief Méthode d'allocation first_fit
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param val valeur
 * @param offset index dans le .kv modifié par effet de bord (doit être déjà alloué)
 */
int first_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  int emplacement_libre = 0;

  len_t taille_requise = get_size(key) + get_size(val), offset_descripteur_max;

  if(search_pos_dkv(kv, &offset_descripteur_max) == -1) {return -1;}

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;}

  len_t offset_descripteur_sauvegarde, taille_courante, offset_courant, offset_min = UINT32_MAX, taille_sauvegarde = 0;


  while(read(kv->fd4, &emplacement_libre, sizeof(int)))
  {
    if(emplacement_libre == 0) // si l'emplacement est libre
    {
      if(read(kv->fd4, &taille_courante, sizeof(len_t)) < 0) {return -1;}

      if(read(kv->fd4, &offset_courant, sizeof(len_t)) < 0) {return -1;}

      if(offset_courant < offset_min && taille_courante >= taille_requise)
      {
        offset_min = offset_courant;

        offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + 2 * sizeof(len_t));

        taille_sauvegarde = taille_courante;
      }
    }
    else // si l'emplacement est occupé
    {
      if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) < 0) {return -1;}
    }
  }

  if(taille_sauvegarde > 0)
  {
    // modification du .dkv
    write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_sauvegarde - taille_requise, offset_min + taille_requise);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_min);

    *offset = offset_min;

    return 42;
  }
  else
  {
    offset = NULL;

    return -1;
  }
}

/*
 * @brief Méthode d'allocation worst_fit
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param val valeur
 * @param offset index dans le .kv modifié par effet de bord (doit être déjà alloué)
 */
int worst_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  int emplacement_libre = 0;

  len_t taille_requise = get_size(key) + get_size(val), offset_descripteur_max;

  if(search_pos_dkv(kv, &offset_descripteur_max) == -1) {return -1;}

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;}

  len_t taille_courante, offset_courant, taille_max = 0, offset_sauvegarde, offset_descripteur_sauvegarde, taille_max2 = 0, offset_sauvegarde2, offset_descripteur_sauvegarde2;

  while(read(kv->fd4, &emplacement_libre, sizeof(int)))
  {
    if(emplacement_libre == 0) // si l'emplacement est libre
    {
      if(read(kv->fd4, &taille_courante, sizeof(len_t)) < 0) {return -1;}

      if(read(kv->fd4, &offset_courant, sizeof(len_t)) < 0) {return -1;}

      if(taille_courante == UINT32_MAX)
      {
        offset_sauvegarde = offset_courant;

        offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + 2 * sizeof(len_t));

        taille_max = taille_courante;

        // modification du .dkv
        write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_max - taille_requise, offset_sauvegarde + taille_requise);
        write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde);

        *offset = offset_sauvegarde;

        return 42;
      }

      if((taille_courante > taille_max) && (taille_courante >= taille_requise) && ((taille_courante + offset_courant - 1) == UINT32_MAX))
      {
        offset_sauvegarde2 = offset_courant;

        offset_descripteur_sauvegarde2 = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + 2 * sizeof(len_t));

        taille_max2 = taille_courante;
      }

      if((taille_courante > taille_max) && (taille_courante >= taille_requise) && ((taille_courante + offset_courant - 1) != UINT32_MAX)) // on vérifie si l'emplacement est plus grand
      {
        offset_sauvegarde = offset_courant;

        offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + 2 * sizeof(len_t));

        taille_max = taille_courante;
      }
    }
    else // si l'emplacement est occupé
    {
      if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) < 0) {return -1;}
    }
  }

  if(taille_max >= taille_requise)
  {
    // modification du .dkv
    write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_max - taille_requise, offset_sauvegarde + taille_requise);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde);

    *offset = offset_sauvegarde;

    return 42;
  }
  else if(taille_max2 >= taille_requise)
  {
    write_descripteur(kv, offset_descripteur_sauvegarde2, 0, taille_max2 - taille_requise, offset_sauvegarde2 + taille_requise);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde2);

    *offset = offset_sauvegarde2;

    return 42;
  }
  else
  {
    offset = NULL;

    return -1;
  }
}

/*
 * @brief Méthode d'allocation best_fit
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param val valeur
 * @param offset index dans le .kv modifié par effet de bord (doit être déjà alloué)
 */
int best_fit(KV *kv, const kv_datum *key, const kv_datum *val, len_t *offset)
{
  int emplacement_libre = 0;

  len_t taille_requise = get_size(key) + get_size(val), offset_descripteur_max;

  if(search_pos_dkv(kv, &offset_descripteur_max) == -1) {return -1;}

  if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1) {return -1;}

  len_t taille_courante, taille_min = UINT32_MAX,  offset_sauvegarde, offset_descripteur_sauvegarde;

  while(read(kv->fd4, &emplacement_libre, sizeof(int)))
  {
    if(emplacement_libre == 0) // si l'emplacement est libre
    {
      if(read(kv->fd4, &taille_courante, sizeof(len_t)) < 0) {return -1;}

      if(taille_courante == UINT32_MAX)
      {
        if(read(kv->fd4, &offset_sauvegarde, sizeof(len_t)) < 0) {return -1;}

        offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + 2 * sizeof(len_t));

        taille_min = taille_courante;

        // modification du .dkv
        write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_min - taille_requise, offset_sauvegarde + taille_requise);
        write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde);

        *offset = offset_sauvegarde;

        return 42;
      }

      if(taille_requise <= taille_courante && taille_courante < taille_min) // on vérifie si l'emplacement est assez grand et plus petit
      {
        if(read(kv->fd4, &offset_sauvegarde, sizeof(len_t)) < 0) {return -1;}

        offset_descripteur_sauvegarde = lseek(kv->fd4, 0, SEEK_CUR) - (sizeof(int) + 2 * sizeof(len_t));

        taille_min = taille_courante;
      }
      else
      {
        if(lseek(kv->fd4, sizeof(len_t), SEEK_CUR) < 0) {return -1;}
      }
    }
    else // si l'emplacement est occupé
    {
      if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) < 0) {return -1;}
    }
  }

  if(taille_min < UINT32_MAX)
  {
    // modification du .dkv
    write_descripteur(kv, offset_descripteur_sauvegarde, 0, taille_min - taille_requise, offset_sauvegarde + taille_requise);
    write_descripteur(kv, offset_descripteur_max, 1, taille_requise, offset_sauvegarde);

    *offset = offset_sauvegarde;

    return 42;
  }
  else
  {
    offset = NULL;

    return -1;
  }
}

/*
 * @brief Détermine la bonne méthode d'allocation
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param val valeur
 * @param offset index dans le .kv modifié par effet de bord
 */
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

/*
 * @brief Lit une en-tête dans .blk
 *
 * @param kv descripteur d'accès à la base
 * @param offset_bloc index dans le .blk
 * @param nouveau_offset index du prochain bloc dans le .blk
 */
int read_entete_bloc(KV *kv, const len_t offset_bloc, len_t * nouveau_offset)
{
  if(lseek(kv->fd2, offset_bloc, SEEK_SET) < 0) {return -1;}

  if(read(kv->fd2, nouveau_offset, sizeof(len_t)) < 0) {return -1;}

  return 42;
}

/*
 * @brief Écrit une en-tête dans .blk
 *
 * @param kv descripteur d'accès à la base
 * @param offset_bloc index dans le .blk
 * @param nouveau_offset index du prochain bloc dans le .blk (à écrire)
 */
int write_entete_bloc(KV *kv, const len_t offset_bloc, const len_t * nouveau_offset)
{
  if(lseek(kv->fd2, offset_bloc, SEEK_SET) < 0) {return -1;}

  if(write(kv->fd2, nouveau_offset, sizeof(len_t)) < 0) {return -1;}

  return 42;
}

/*
 * @brief Suppression de la clé dans la base
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 */
int kv_del(KV * kv, const kv_datum * key)
{
  kv_start(kv);
  int val_hash = hash(key->ptr, kv);
  len_t bloc_courant, bloc_suivant=0;
  if(lseek(kv->fd1, val_hash*sizeof(len_t) , SEEK_CUR) <0) {return -1;}
  if(read(kv->fd1, &bloc_courant, 4) <0){return -1;}

  if(!bloc_courant)
  {
    errno= ENOENT;
    return -1;
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
        if(lseek(kv->fd3, pos_cle, SEEK_SET) < 0) {return -1;}
        if(read(kv->fd3, &lg_cle, 4) < 0) {return -1;}
        if(lg_cle == key->len)
        {
          char * cle_lue = malloc(lg_cle + 1);

          if(read(kv->fd3, cle_lue, lg_cle) < 0) {return -1;}

          cle_lue[lg_cle] = '\0';

          if(strcmp(key->ptr, cle_lue) == 0)
          {
            free(cle_lue);
            len_t zero = 0;
            if(lseek(kv->fd2, -4, SEEK_CUR) == -1) {return -1;}
            if(write(kv->fd2, &zero, 4) < 0) {return -1;}

            len_t off_lue;
            int libre;
            len_t lg_atruncate;
            lseek(kv->fd4, taille_header_f, SEEK_SET);
            while(read(kv->fd4, &libre, sizeof(int)))
            {
              if(lseek(kv->fd4, 4, SEEK_CUR) < 0) {return -1;}
              if(read(kv->fd4, off_lue, 4) < 0) {return -1;}

              if(off_lue == pos_cle)
              {
                int zero_int =0;
                if(lseek(kv->fd4, -12, SEEK_CUR) == -1) {return -1;}
                if(write(kv->fd4, &zero_int, 4) < 0) {return -1;}
                if(read(kv->fd4, &lg_atruncate, 4) != 4){return -1;}
                len_t atruncate=lseek(kv->fd4, -8, SEEK_CUR);

                if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1){return -1;}
                int existe;
                int flag_while=0;
                while((read(kv->fd4, &existe, sizeof(int))) && (flag_while != 2))
                {
                  if(existe==0)
                  {
                    len_t lg, off;
                    if(read(kv->fd4, &lg, 4) < 4){return -1;}
                    if(read(kv->fd4, &off, 4) < 4){return -1;}
                    if(lg+off == off_lue)
                    {
                      //modifier lg clé il faut ajouter la longueur total et pas celle de la clé
                      lg_atruncate= lg+lg_atruncate;
                      len_t pos_tmp;
                      if(lseek(kv->fd4, -8, SEEK_CUR) == -1){return -1;}
                      if(write(kv->fd4, &lg_atruncate, 4) != 4){return -1;}
                      pos_tmp=lseek(kv->fd4, 4, SEEK_CUR);
                      if(lseek(kv->fd4, atruncate, SEEK_SET) == -1){return -1;}
                      int deux = 2;
                      if(write(kv->fd4, &deux, 4) != 4){return -1;}
                      atruncate=pos_tmp-12;
                      off_lue= off;
                      flag_while++;
                      if(lseek(kv->fd4, pos_tmp, SEEK_SET) == -1){return -1;}
                    }
                    else if(off == off_lue + lg_atruncate)
                    {
                      //modifier lg clé il faut ajouter la longueur total et pas celle de la clé
                      lg_cle= lg+lg_atruncate;
                      len_t pos_tmp;
                      if(lseek(kv->fd4, -8, SEEK_CUR) == -1){return -1;}
                      if(write(kv->fd4, &lg_atruncate, 4) != 4){return -1;}
                      if(write(kv->fd4, &off_lue, 4) != 4){return -1;}
                      pos_tmp=lseek(kv->fd4, 0, SEEK_CUR);
                      if(lseek(kv->fd4, atruncate, SEEK_SET) == -1){return -1;}
                      int deux = 2;
                      if(write(kv->fd4, &deux, 4) != 4){return -1;}
                      atruncate=pos_tmp-12;
                      flag_while++;
                      if(lseek(kv->fd4, pos_tmp, SEEK_SET) == -1){return -1;}
                    }
                  }
                  else
                  {
                    if(lseek(kv->fd4, 8, SEEK_CUR) == -1){return -1;}
                  }
                }
                return 0;
              }
            }
          }
          else
            free(cle_lue);
        }
      }
    }
    if(bloc_suivant && bloc_suivant != 0)
    {
      bloc_courant = bloc_suivant;
    }
    else
    {
      boucle =-1;
    }
  }
  errno = ENOENT;
  return -1;
}

/*
 * @brief Lecture d'une entrée dans le .h
 *
 * @param kv descripteur d'accès à la base
 * @param offset_h index dans le .h
 * @param offset_blk index dans le .blk modifié par effet de bord
 */
int read_h(KV *kv, len_t offset_h, len_t * val_h)
{
  kv_start(kv);

  if(lseek(kv->fd1, offset_h * sizeof(len_t), SEEK_CUR) < 0) {return -1;}

  int n = read(kv->fd1, val_h, sizeof(len_t));

  if(n == -1) {return -1;}

  return n;
}

/*
 * @brief Écrit une entrée dans le .h
 *
 * @param kv descripteur d'accès à la base
 * @param offset_h index dans le .h
 * @param offset_blk index dans le .blk (à écrire)
 */
int write_h(KV *kv, len_t offset_h, len_t offset_blk)
{
  kv_start(kv);

  if(lseek(kv->fd1, offset_h * sizeof(len_t), SEEK_CUR) < 0) {return -1;}

  if(write(kv->fd1, &offset_blk, sizeof(len_t)) == -1) {return -1;}

  return 42;
}

/*
 * @brief Création d'un nouveau bloc en fin de fichier .blk
 *
 * Création d'un nouveau bloc en fin de fichier .blk et remplissage de 0
 *
 * @param kv descripteur d'accès à la base
 * @param offset_nouveau_bloc index du bloc créé modifié par effet de bord
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

/*
 * @brief Écrit une entrée dans un bloc
 *
 * @param kv descripteur d'accès à la base
 * @param offset_entry index dans le .blk
 * @param offset_data index dans le .kv (à écrire)
 */
int write_bloc_entry(KV *kv, len_t offset_entry, len_t offset_data)
{
  if(lseek(kv->fd2, offset_entry, SEEK_SET) < 0) {return -1;}

  if(write(kv->fd2, &offset_data, sizeof(len_t)) == -1) {return -1;}

  return 42;
}

/*
 * @brief Détermine le bon bloc
 *
 * @param kv descripteur d'accès à la base
 * @param offset_bloc index dans le .blk
 * @param offset_data index dans le .kv (à écrire)
 */
int write_bloc(KV *kv, len_t offset_bloc, len_t * offset_data)
{
  len_t offset_lu_courant, offset_courant, offset_bloc_suivant, offset_sauvegarde = offset_bloc;

  int i;
  off_t off;

  while(offset_bloc)
  {
    read_entete_bloc(kv, offset_bloc, &offset_bloc_suivant); // ce qui déplace juste apres l'en-tête du bon bloc

    for(i = 0; i < TAILLE_BLOC - (int)sizeof(len_t); i++)
    {
      off = lseek(kv->fd2, 0, SEEK_CUR);

      if(off == -1)
      {
        return -1;
      }

      offset_courant = off;

      read(kv->fd2, &offset_lu_courant, sizeof(len_t));

      if(offset_lu_courant == 0)
      {
        write_bloc_entry(kv, offset_courant, *offset_data);
        return 42;
      }
    }

    offset_bloc = offset_bloc_suivant;
  }

  // pas de bloc suivant et plus de place dans le bloc courant

  len_t offset_new_bloc;

  if(new_bloc(kv, &offset_new_bloc)) {return -1;}

  if(write_bloc(kv, offset_new_bloc, offset_data)) {return -1;};

  if(write_bloc_entry(kv, offset_new_bloc, offset_sauvegarde)) {return -1;}; // lien entre les 2 blocs

  return 42;
}

/*
 * @brief Gestion du .blk lors de l'ajout d'un couple key/val
 *
 * Si un index de bloc existe déjà :
 *  -> écriture de offset_key dans ce bloc (avec gestion de bloc plein)
 * Sinon :
 * -> création d'un nouveau bloc et écriture de l'index de ce bloc dans le .h
 * -> écriture de offset_key dans ce bloc (avec gestion de bloc plein)
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param offset_key index du couple dans le .kv
 */
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
    if(write_bloc(kv, val_h, offset_key) == -1) {return -1;}
  }

  return 42;
}

/*
 * @brief Insère un couple key/val dans la base
 *
 * Insère un couple key/val dans la base en fonction de la méthode d'allocation
 * et de la fonction de hachage en argument (kv)
 *
 * @param kv descripteur d'accès à la base
 * @param key clé
 * @param val valeur
 */
int kv_put (KV *kv, const kv_datum *key, const kv_datum *val)
{
  len_t offset_tmp;

  if(offset_cle(kv,key,&offset_tmp) == 1) // la clé existe déjà
  {
    if((kv_del(kv,key)) == -1) {return -1;}
    if((kv_put(kv,key,val)) == -1) {return -1;}

    return 42;
  }
  else // la clé n'existe pas
  {
    len_t offset;

    if(kv_put_dkv(kv, key, val, &offset) == -1) {return -1;} // modification dans le .dkv
    if(kv_put_blk(kv, key, &offset) == -1) {return -1;} // modification dans le .blk
    if(writeData(kv, key, val, offset) == -1) {return -1;} // écriture du couple key/val
  }
  return 42;
}

/*
 * @brief Renvoie le couple key/val suivant
 *
 * @param kv descripteur d'accès à la base
 * @param key clé modifiée par effet de bord
 * @param val valeur modifiée par effet de bord
 */
int kv_next(KV *kv, kv_datum *key, kv_datum *val)
{
  int pos = lseek(kv->fd4, -1, SEEK_CUR);

  int existe;

  if(pos == -1)
  {
    return -1;
  }
  else if(pos == 0)
  {
    if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1){return -1;}

    while(read(kv->fd4, &existe, sizeof(int)))
    {
      if(existe == 1)
      {
        len_t lgtmp, off;
        if(read(kv->fd4, &lgtmp, 4) < 4){return -1;}
        if(read(kv->fd4, &off, 4) < 4){return -1;}

        if(off == 1)
        {
          len_t cle_saut;
          if(readVal(kv, key, off) == -1) {return -1;}
          if(lseek(kv->fd3, off, SEEK_SET) < 0) {return -1;}
          if(read(kv->fd3, &cle_saut, sizeof(len_t)) == -1) {return -1;}
          off = off + sizeof(len_t) + cle_saut;
          if(readVal(kv, val, off) == -1) {return -1;}

          return 1;
        }
      }
      else
      {
        if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) == -1){return -1;}
      }
    }
  }
    else
  {
    if(lseek(kv->fd4, -7, SEEK_CUR) == -1) {return -1;}

    len_t slg1, slg2, lg;

    if(read(kv->fd4, &slg1, sizeof(len_t)) == -1) {return -1;}
    if(read(kv->fd4, &slg2, sizeof(len_t)) == -1) {return -1;}

    lg = slg1 + slg2;

    if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1){return -1;}

    while(read(kv->fd4, &existe, sizeof(int)))
    {
      len_t lgtmp, off;
      if(read(kv->fd4, &lgtmp, 4) < 4){return -1;}
      if(read(kv->fd4, &off, 4) < 4){return -1;}
      if(existe == 1)
      {
        if(off == lg)
        {
          len_t cle_saut;
          if(readVal(kv, key, off) == -1) {return -1;}
          if(lseek(kv->fd3, off, SEEK_SET) < 0) {return -1;}
          if(read(kv->fd3, &cle_saut, sizeof(len_t)) == -1) {return -1;}
          off = off + sizeof(len_t) + cle_saut;
          if(readVal(kv, val, off) == -1) {return -1;}
          return 1;
        }
      }
      else if(existe == 0)
      {
        if(off == lg)
        {
          lg= off + lgtmp;
          if(lseek(kv->fd4, taille_header_f, SEEK_SET) == -1){return -1;}
        }
      }
      else
      {
        if(lseek(kv->fd4, 2 * sizeof(len_t), SEEK_CUR) == -1){return -1;}
      }

    }
  }

  return 0;
}
