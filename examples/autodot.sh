#! /usr/bin/env bash


# From https://gist.github.com/JpGallegos/7825596

for file in `ls *.dot`
do
    dot -Kneato -Tpdf -o ${file/.dot/}.pdf $file
done
