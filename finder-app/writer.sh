#!/bin/sh
filespath=""
contentstr=""

if [ $# -lt 3 ]
then
	if [ $# -lt 1 ]
	then
        echo "the parameters were not specified"
		exit 1
	else
		filespath=$1
        contentstr=$2
	fi	
else
    echo "the parameters were not specified"
    exit 1
fi

directory_path=$(dirname "$filespath")
if [ ! -d "$directory_path" ]; then
    mkdir -p "$directory_path"
fi

echo "${contentstr}" > ${filespath}

# Check if file creation was successful
if [ $? -ne 0 ]; then
    echo "Error: Could not create the file."
    exit 1
fi