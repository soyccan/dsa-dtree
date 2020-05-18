#!/bin/sh

exec > test/out.csv

for epsilon in $(seq 0 0.01 0.5); do
    for i in $(seq 0 1000); do
        depth="$(./tree data/heart_scale "$epsilon" 2>&1 >/dev/null \
            | sed -nE 's/MaxDepth=(.*)$/\1/p')"
        echo "$epsilon,$depth"
    done
done
