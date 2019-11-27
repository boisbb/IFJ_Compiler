#!/usr/bin/env bash

x=0

if [ "$1" == "-h" ]; then
  x=1
  echo
  echo "první argument = soubor, který se bude překládat, optional - druhý = vstupní soubor (.in)"
  echo
  echo "např.: ./test.sh program.ifj vstupni.in"
  echo
  exit
fi

if [ "$#" == 2 ]; then
  cat ./ifj19.py "$1" > testPrg.py
  python3 testPrg.py < "$2" > test.out #výsledek pythonu

  #./ifj_compiler < "$1" >final.out

  ./ic19int final.out <"$2" > result #vstupní souboru asi nějak předat

  echo "start diffu -->"
  #diff result test.out
  echo "<-- konec diffu"

  #rm final.out test.out result

  exit
fi

if [ "$#" == 1 ]; then
  cat ./ifj19.py "$1" > testPrg.py
  python3 testPrg.py > test.out #výsledek pythonu

  #./ifj_compiler < "$1" >final.out

  ./ic19int final.out > result


  echo "start diffu -->"
  #diff result test.out
  echo "<-- konec diffu"

  #rm final.out test.out result

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

exit
