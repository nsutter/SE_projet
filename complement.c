#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kv.h"


char *usage_string = "ca ne s'utilise pas" ;

char *help_string = "ceci ne sert a rien";

int main()
{
  KV *yolo;
  yolo= kv_open ("test-667", "w", 0, FIRST_FIT);
  if(yolo == NULL){perror(""); exit(1);}
  if(kv_close(yolo) == -1){perror(""); exit(1);}
  yolo= kv_open ("test-667", "w+", 0, FIRST_FIT);
  if(yolo == NULL){perror(""); exit(1);}

  char *cle= malloc(90);
  strcpy(cle, "unprogrammefaitcequonluidemandepasforcementcequonveut");
  char *val=malloc(10);
  val="a";

  kv_datum * kv1=malloc(sizeof(kv_datum));
  kv1->len=53;
  kv1->ptr=cle;

  kv_datum * valeur=malloc(sizeof(kv_datum));
  valeur->len=2;
  valeur->ptr=val;

  int i,j,k;
  for(i=0; i<53; i++)
  {
    for(j=0; j<=i; j++)
    {
      for(k=j;k<=i;k++)
      {
        cle[k]=cle[k]-32;
      }
      if(kv_put(yolo, kv1, valeur)== -1){perror(""); exit(1);}
      for(k=j; k<= i; k++)
        cle[k]=cle[k]+32;
    }
  }
  if(kv_close(yolo) == -1){perror(""); exit(1);}
  return 0;
}
