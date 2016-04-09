#include <stdio.h>
#include <stdlib.h>
#include "kv.h"

int main()
{
  KV *yolo;
  yolo= kv_open ("yolo", "w", 0, FIRST_FIT);
  if(yolo == NULL){perror(""); return 1;}
  if(kv_close(yolo) == -1){perror(""); return -1;}
  yolo= kv_open ("yolo", "w+", 0, FIRST_FIT);
  if(yolo == NULL){perror(""); return 1;}

  char cle[90]="unprogrammefaitcequonluidemandepasforcementcequonveut";
  char val[2]="a";
  kv_datum * kv1;
  kv1->len=52;
  kv1->ptr=&cle;
  kv_datum * kv2;
  kv2->len=1;
  kv2->ptr=&val;
  int i,j;
  for(i=0; i<53; i++)
  {
    for(j=0; j<i; i++)
    {
      cle[i]=cle[i]+32;
    }
    if(kv_put(yolo, kv1, kv2)== -1){perror(""); return 1;}
    for(j=0; j<i; j++)
      cle[i]=cle[i]-32;
  }
  return 0;
}
