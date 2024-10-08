#!/bin/bash
# -----------------------------------------------
# Build file and extract libmagic.so and magic.h
#
# Author: 	Josselin DULONGCOURTY
# Date: 	2024-10-08
# Github:   josselinonduty
# -----------------------------------------------

# -----------------------------------------------
# Usage:
# 	./magic.sh build <input-path>
# 	./magic.sh extract <input-path> <output-path>
# 	./magic.sh clean <input-path> <output-path>
# -----------------------------------------------


case $1 in
    build)
        # check if the input path is provided
        if [ -z $2 ]; then
            echo "Input directory is not provided"
            exit 1
        fi

        # check if the input path exists
        if [ ! -d $2 ]; then
            echo "Input directory does not exist"
            exit 1
        fi

        # build the file
        cd $2
        autoreconf -f -i
        make distclean
        ./configure --enable-silent-rules
        make -j4
        ;;

    extract)
        # check if the input path is provided
        if [ -z $2 ]; then
            echo "Input directory is not provided"
            exit 1
        fi

        # check if the input path exists
        if [ ! -d $2 ]; then
            echo "Input directory does not exist"
            exit 1
        fi

        # check if the output path is provided
        if [ -z $3 ]; then
            echo "Output directory is not provided"
            exit 1
        fi

        # check if the output path exists
        if [ ! -d $3 ]; then
            echo "Output directory does not exist"
            exit 1
        fi

        # ensure files to extract exist
        if [ ! -f $2/src/.libs/libmagic.so ] || [ ! -f $2/src/magic.h ]; then
            echo "Error: No files to extract. Try to run build first"
            exit 1
        fi

        # extract files
        cp $2/src/.libs/libmagic.so $3
        cp $2/src/magic.h $3
        ;;

    clean)
        # check if the input path is provided
        if [ -z $2 ]; then
            echo "Input directory is not provided"
            exit 1
        fi

        # check if the input path exists
        if [ ! -d $2 ]; then
            echo "Input directory does not exist"
            exit 1
        fi

        # clean the file
        pwd_init=$(pwd)
        cd $2
        make distclean
        cd "$pwd_init"

        # check if the output path is provided
        if [ -z $3 ]; then
            exit 0
        fi
    
        # check if the output path exists
        if [ ! -d $3 ]; then
            echo "Warning: Output directory does not exist"
            exit 0
        fi

        # remove the output path
        rm -rf $3
        ;;
    *)
        echo "Invalid command"
        exit 1
        ;;
esac

exit 0