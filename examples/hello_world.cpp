#!/usr/local/bin/seabang --verbose

#include <iostream>

int main(int argc, char *argv[])
{
// Say hello to the world!
    std::cout << "Hello world!\n";

    std::cout << "Home is " << getenv("HOME") << "\n";

// And quit
    return EXIT_SUCCESS;
}

