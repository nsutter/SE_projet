#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NBIT(c,n) (((c) >> (n)) & 1) // macro qui détermine le n-ième bit d'un caractère

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

// 32 caractères max.
int hash2(const char tab[])
{
    int lg_tab = strlen(tab);

    int lg_bit = lg_tab * 8;

    double yolo = (double)lg_bit/(double)32;

    int n_bit = (int)ceil(yolo);

    int i, j, k, hash = 0;

    for(i = 0, j = 0, k = 0; i < lg_tab && k < 32; k++)
    {
      hash += NBIT(tab[i],j) * pow( 2, k);

      j += n_bit;

      if(j > 7)
      {
        i++;
        j = j%8;
      }
    }

    return hash % 999983;
}

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    exit(1);
  }

  printf("%d\n",hash2(argv[1]));

  return 0;
}
