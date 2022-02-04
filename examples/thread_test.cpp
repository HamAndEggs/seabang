#!/usr/local/bin/seabang

#include <iostream>
#include <thread>

int main(int argc, char *argv[])
{
// Say hello to the world!
    std::cout << "Hello world!\n";

    std::thread([]()
    {
        int i = 0;
        for( int n = 0 ; n < 10000 ; n++ )
        {
            i += rand();
        }
        
        std::clog << i << "\n";
    }).join();

     std::cout << "And done\n";

// And quit\n";
    return EXIT_SUCCESS;
}
