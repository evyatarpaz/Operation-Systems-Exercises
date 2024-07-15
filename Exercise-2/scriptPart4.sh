#!/bin/bash

compile_program() {
    gcc -c copytree.c -o copytree.o
    ar rcs libcopytree.a copytree.o
    gcc part4.c -L. -lcopytree -o part4_program
}

setup_directories() {
    # Remove existing directories and recreate them
    mkdir -p source_directory/subdir1 source_directory/subdir2 destination_directory

    # Create files in the source_directory and its subdirectories
    echo "Content of file1.txt" > source_directory/file1.txt
    echo "Content of file2.txt" > source_directory/file2.txt
    echo "Content of file3.txt" > source_directory/subdir1/file3.txt
    echo "Content of file4.txt" > source_directory/subdir1/file4.txt
    echo "Content of file5.txt" > source_directory/subdir2/file5.txt

    # Create a symbolic link in subdir2 pointing to file1.txt in the parent directory
    ln -s ../file1.txt source_directory/subdir2/link_to_file1
}

run_and_compare() {
    local flags=$1
    echo "Running with flags: $flags"
    ./part4_program $flags "./source_directory" "./destination_directory"

    # echo the tree structure of the source and destination directories
    echo "Source directory:"
    tree source_directory
    echo "Destination directory:"
    tree destination_directory
}

# Compile the library and the main program
compile_program

setup_directories

# Test with different flags
run_and_compare "-l -p"