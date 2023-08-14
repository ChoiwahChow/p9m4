#!/bin/bash

outfile=$1



fs0=$(wc -l $outfile | awk '{print $1}')
if [ $fs0 -gt 0 ]; then
    ../utils/cubing/isonaut.py -i ${outfile} -o non_iso_models.out -k 1
    wc -l ${outfile} >> models_count.out
    rm ${outfile}
fi

if [ ! -f "non_iso_models.out" ]; then
  >> non_iso_models.out
fi

fs=$(wc -l "non_iso_models.out" | awk '{print $1}')

if [ $fs -gt 10000000 ]
then
  ../utils/cubing/isonaut.py -i non_iso_models.out -o level1_non_iso_models.out  -k 1
  rm non_iso_models.out

  fs1=$(wc -l "level1_non_iso_models.out" | awk '{print $1}')

  if [ $fs1 -gt 30000000 ]
  then
    ../utils/cubing/isonaut.py -i level1_non_iso_models.out -o level2_non_iso_models.out -k 1
    rm level1_non_iso_models.out
  fi
fi
