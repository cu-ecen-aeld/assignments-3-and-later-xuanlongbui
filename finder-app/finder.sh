#!/bin/sh
filesdir=""
searchstr=""

if [ $# -lt 3 ]
then
	if [ $# -lt 1 ]
	then
        echo "the parameters were not specified"
		exit 1
	else
		filesdir=$1
        searchstr=$2
	fi	
else
    echo "the parameters were not specified"
    exit 1
fi


if [ ! -d "$filesdir" ]; then
    echo "filesdir does not represent a directory on the filesystem"
    exit 1
fi

X=$(grep -rl ${searchstr} ${filesdir} |wc -l)
Y=$(grep  ${searchstr} ${filesdir}/* |wc -l)

echo "The number of files are ${X} and the number of matching lines are ${Y}"