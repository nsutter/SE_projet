#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kv.h"

char *usage_string = "usage_string" ;

char *help_string = "help_string";

int main()
{

  KV *yolo;
  yolo= kv_open ("test-667", "w", 0, FIRST_FIT);
  if(yolo == NULL){perror(""); exit(1);}
  if(kv_close(yolo) == -1){perror(""); exit(1);}


  yolo= kv_open ("test-667", "w+", 0, FIRST_FIT);
  if(yolo == NULL){perror(""); exit(1);}
  KV *blinde1,* blinde2,* blinde3;

  blinde1= kv_open("blinde1", "w+", 0, FIRST_FIT);
  if(blinde1 == NULL){perror(""); exit(1);}
  blinde2= kv_open("blinde2", "w+", 1, BEST_FIT);
  if(blinde2 == NULL){perror(""); exit(1);}
  blinde3= kv_open("blinde3", "w+", 0, WORST_FIT);
  if(blinde3 == NULL){perror(""); exit(1);}


  char *cle= malloc(1030);
  strcpy(cle,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  char *val=malloc(10);
  val="a";

  kv_datum * kv1=malloc(sizeof(kv_datum));
  kv1->len=53;
  kv1->ptr=cle;

  kv_datum * valeur=malloc(sizeof(kv_datum));
  valeur->len=2;
  valeur->ptr=val;

  int i;
  for(i=0; i<1029; i++)
  {
    cle[i]=cle[i]+1;
    if(kv_put(yolo, kv1, valeur)== -1){printf("%d valeures inserés dans la db\n", i); perror(""); exit(1);}
    cle[i]=cle[i]-1;
  }

  kv_datum * kv3=malloc(sizeof(kv_datum));
  kv3->ptr=malloc(1);
  kv3->len=0;
  for(i=0; i<1029; i++)
  {
    cle[i]=cle[i]+1;
    if(kv_get(yolo, kv1, kv3) == -1){printf("erreur"); exit(1);}
    cle[i]=cle[i]-1;
  }


  // test suppression clé inexistante dans un bloc existant
  cle[10]+=2;
  cle[9]-=1;
  kv_del(yolo, kv1);



  if(kv_close(yolo) == -1){perror("");}

  // test remplissage à fond de la db
  int j, k;
  int flag_put1=0, flag_put2=0, flag_put3=0;
  for(i=0; i<26; i++)
  {
    for(j=0; j<1024; j++)
    {
      for(k=0; k<j; k++)
      {
        cle[k]++;
      }
      cle[j]++;
      if(flag_put1 == 0){ if(kv_put(blinde1, kv1, kv3)==-1){flag_put1=1;} }
      if(flag_put2 == 0){ if(kv_put(blinde2, kv1, kv3)==-1){flag_put2=1;} }
      if(flag_put3 == 0){ if(kv_put(blinde3, kv1, kv3)==-1){flag_put3=1;} }
      for(k=0; k<j; k++)
        cle[k]--;
    }
    cle[i]++;
  }

  return 0;
}
