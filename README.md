# seabang
Allows you to run c++ source file as a script

Use `make` to build it and `make install` to install in /usr/bin.

Example...
```
#!/usr/bin/seabang

#include <iostream>

int main(int argc, char *argv[])
{
// Say hello to the world!
    std::cout << "Hello world!\n";

    std::cout << "Home is " << getenv("HOME") << "\n";
// And quit
    return EXIT_SUCCESS;
}
```
Then make source file executable and your done. You can now run your c/c++ source files.