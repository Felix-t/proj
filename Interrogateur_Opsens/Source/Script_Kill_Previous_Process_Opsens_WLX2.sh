#!/bin/bash

list=$(pidof $1)
# .. now  do something on all pids stored in $list

cpt=0
for p in $list
do
  if [ $cpt -lt 0 ]; then
    echo "Killing $p..."
    sudo kill -9 $p
  else
  cpt=$(($cpt + 1))
  fi
done



