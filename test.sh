#!/usr/bin/env bash

x=0

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

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
  if [ "$1" != "-e" ]; then
    echo "${RED}INCORRECT PARAMETER FORMAT!${NC}"
  fi

  printf "${YELLOW}EXIT CODES COMPARISON:${NC} \n"
  for file in $2
  do
    ./main < "$file"
    retval=$?
    file_to_check=${file::-2}
    file_to_check="${file_to_check}txt"
    needed_exit="$(cat "$file_to_check")"
    if [[ "$needed_exit" == "$retval" ]]; then
      echo -e "$(realpath "$file") ${GREEN}PASSED${NC}"
    else
      echo -e "$(realpath "$file") ${RED}FAILED${NC} | ${GREEN}EXPECTED:${NC} "$needed_exit" | ${RED}ACTUAL: "$retval""
    fi
  done

  exit
fi

if [ "$#" == 1 ]; then
  printf "${BLUE}DIFF RESULTS:${NC} \n"
  for file in $1
  do
    cat ./ifj19.py "$file" > testPrg.py
    python3 testPrg.py > test.out #výsledek pythonu

    ./main < "$file" >final.out
    #echo "interpret -->"

    ./ic19int final.out > result


    DIFF=$(diff result test.out)
    if [ "$DIFF" == "" ]
    then
      echo -e "$(realpath "$file") ${GREEN}PASSED${NC}"
    else
      echo -e "$(realpath "$file") ${RED}FAILED${NC}"
    fi

    rm  test.out result final.out testPrg.py
  done

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
