#!/bin/bash

list=$(pidof $1)

echo $list 
# .. now  do something on all pids stored in $list

i=0
for p in $list
do
    echo "Killing $p..."
    sudo kill -9 $p
done



