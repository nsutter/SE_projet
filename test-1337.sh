#!/bin/sh

HIDX=0

set -x

fail ()
{
    echo "FAIL"
    exit 1
}

generer ()
{
    (echo "$*" ; dd if=/dev/zero count=1 bs=$3) | $V ./put -a $2 toast $1
}

# Création des trous
# fichier1 contient les valeurs de l'espace qu'on veut allouer à chaque couple
i=1
for ESPACE in `cat fichier1`
do
    $V ./put toast -i $HIDX a$i repere-$i		|| fail "put a$i"
    generer t$i first $ESPACE			|| fail "put t$i"
    i=$((i + 1))
done

for j in $(seq 1 1000)
do
    $V ./del toast t$j
done

exit 0
