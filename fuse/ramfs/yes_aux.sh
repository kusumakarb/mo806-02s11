#!/bin/bash

rm -f $1

for i in $(seq 100000)
do
    echo -e "y" >> $1
done
