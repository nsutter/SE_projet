#include <stdlib.h>
#include <stdio.h>

int hashage(char tab[])
{
  int i;
  int somme=0;
  for(i=0; tab[i]!= '\0'; i++)
  {
    somme= somme + tab[i];
  }
  return(somme%999983);
}
