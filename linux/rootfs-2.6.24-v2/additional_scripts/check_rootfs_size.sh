#!/bin/sh

blk=`df | tail -1 | sed 's/[a-z/0-9]* *\([0-9]*\).*/\1/'`
tot=`expr $1 \- 200`

if [ $# -ne 1 ]
then
    echo Wrong number of arguments
    exit 1
fi

if [ $blk -gt $tot ]
then
    echo True
else
    echo Available=$blk
    echo Expected=$tot
    echo False
fi

