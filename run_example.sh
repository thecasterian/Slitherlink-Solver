#!/bin/bash
for file in example/*.txt; do
    echo $file
    ./slitherlink < $file
done
