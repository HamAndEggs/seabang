#!../build/seabang --verbose
//#!../build/seabang --seabang-compiler=gcc --verbose
//#!/usr/local/bin/seabang --seabang-compiler=gcc

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Compiler overwritten test\n";
    return EXIT_SUCCESS;
}
