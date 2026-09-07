#!/usr/bin/env bash

if (( $# == 0)); then 
    echo "Usage: $0 number [number ...]" >&2
    exit 1
fi

sum=0
for number in "$@"; do 
    ((sum += number))
done

average=$(awk -v sum="$sum" -v count="$#" 'BEGIN { printf "%0.3f", sum / count}')
echo "Count: $#"
echo "Average: $average"
