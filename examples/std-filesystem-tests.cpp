#!/usr/local/bin/seabang

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <filesystem>

int main(int argc, char *argv[])
{
    std::filesystem::path random1 = "/dev/random";
    std::filesystem::path random2 = "../../../../../dev/random";
    std::filesystem::path random3 = "../.././../../../dev/random";

    std::cout << random1 << "\n";
    std::cout << random2 << "\n";
    std::cout << random3 << "\n";
    std::cout << random1.compare(random2) << "\n";
    std::cout << random1.compare(random3) << "\n";
    std::cout << random3.compare(random1) << "\n";


    return EXIT_SUCCESS;
}
