#!/usr/bin/env bash

x=0

if [ "$1" == "-h" ]; then
  x=1
  echo
  echo "Jako první argument zadej překládaný kód, jako druhý agrument zadej očekávaný výstup"
  echo
  echo "např.: ./test.sh in1.txt out1.txt"
  echo
  exit
fi

if [ "$#" != 2 ]; then
  if [ "$x" == 0 ]; then
    echo
    echo "chyba argumentů"
    echo
    exit
  fi
fi

echo "start diffu -->"

cat "$1" | ./a.out > vystup.tmp

diff vystup.tmp "$2"

rm vystup.tmp

echo "<-- konec diffu"
exit
