#!/bin/bash


if [ $# -eq 0 ]; then
    echo "нет аргументов"
    exit 1
fi

sum=0
count=$#

for num in "$@"; do
    sum=$((sum + num))
done


average=$((sum / count))

echo "колво аргументов: $count"
echo "среднее ариф: $average"
