#!/bin/bash

FILE=./$1

if [[ $FILE == "" ]]; then
    echo "mk_code.sh <filename>"
    exit 1
fi

EXT=.cpp
NAME=${FILE%$EXT}
OUT=$NAME.o$EXT

function get_files {
    OLDFILES=$1
    while :; do
        NEWFILES="$OLDFILES"
        for f in $OLDFILES; do
            TMP=$(grep -hE '^#include "(.*)"' $f | sed 's/#include "\(.*\)"/\1/')
            for n in $TMP; do
                NEWFILES="$NEWFILES $(dirname $f)/$n"
            done
        done
        NEWFILES=$(echo $NEWFILES | tr ' ' '\n' | sort | uniq)
        if [[ $OLDFILES == $NEWFILES ]]; then
            break
        fi
        OLDFILES=$NEWFILES
    done
    echo $NEWFILES
}

FILES=$(get_files $FILE)

for f in $FILES; do
    F=$(grep -hE '^#include "(.*)"' $f | sed 's/#include "\(.*\)"/\1/')
    for n in $F; do
        echo $(dirname $f)/$n $f
    done
done | tsort | xargs awk '{print}' | grep -vE '#include "(.*)"' > $OUT.tmp

(grep -E '#include <(.*)>' $OUT.tmp | sort | uniq;
grep -vE '#(ifndef|define) .*_H' $OUT.tmp | grep -vE '#include <(.*)>' | grep -vE '#endif')> $OUT

rm $OUT.tmp
