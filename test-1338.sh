#!/bin/sh

fail ()
{
    echo "FAIL"
    exit 1
}

generer ()
{
    (echo "$*" ; dd if=/dev/zero count=1 bs=$3) | $V ./put -a $2 toast $1
}

# fichier2 contient les valeurs de l'espace qu'on veut allouer à chaque couple
i=1
for ESPACE in `cat fichier2`
do
  generer $i worst $ESPACE				|| fail "fail"
  i=$((i + 1))
done

exit 0
