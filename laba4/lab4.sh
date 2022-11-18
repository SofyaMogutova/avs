#!/bin/bash
echo "Function;MatrixSize;Time" > lab4.csv
for((i=1;i<=10;i++))
do
./lab4 -s $((i*100)) -o 0
done

for((i=1;i<=10;i++))
do
./lab4 -s $((i*100)) -o 1
done