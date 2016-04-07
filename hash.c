#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

#define NBIT(c,n) (((c) >> (n)) & 1) // macro qui détermine le n-ième bit d'un caractère

char **separe(char *chaine, const char *separateurs)
{
	char **tab;
	int i,s,m,size = 10;

	tab = malloc(size * sizeof(char*));
	m = 0;
	i = 0;
	while(chaine[i] != 0)
	{
		for(s = 0; separateurs[s] != 0; s++)
			if(chaine[i] == separateurs[s])
				break;
		if(separateurs[s] != 0)
		{
			chaine[i++] = 0;
			continue;
		}

		if(chaine[i] != 0)
			tab[m++] = chaine + i;
		if(m == size)
		{
			size += 10;
			tab = realloc(tab,size * sizeof(char*));
		}
		for(;chaine[i] != 0; i++)
		{
			for(s = 0; separateurs[s] != 0; s++)
				if(chaine[i] == separateurs[s])
					break;
			if(separateurs[s] != 0)
				break;
		}
	}
	tab[m] = NULL;
	return(tab);
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

int main()
{
  int i, fd = open("/usr/share/dict/words", O_RDONLY);

  char tab_lu[4953680];

  if(read(fd, &tab_lu, 4953680) == -1) {return -1;}

  char **tab = separe(tab_lu, "\n");

	struct timeval tv;
  struct timeval tv2;

  gettimeofday(&tv,NULL);

  printf("%ld-%ld\n",tv.tv_sec,tv.tv_usec);

  for(i = 0; i < 479828; i++)
  {
    hash2(tab[i]);
  }

	gettimeofday(&tv2,NULL);

  printf("%ld-%ld\n",tv2.tv_sec,tv2.tv_usec);

  printf("Temps écoulé : %ld seconde(s) et %ld microseconde(s)",tv2.tv_sec - tv.tv_sec,tv2.tv_usec - tv.tv_usec);

  return 0;
}
