# seabang
Allows you to run c++ source file as a script


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

# instillation
CMake is now used to configure, build and install the application.

```
mkdir -p ./build
cd ./build
cmake ..
make
sudo make install
```

or call the 'make_and_install.sh' script which does what is listed above.

# usage

Usage: #!/usr/bin/seabang [OPTION]
This command is not to be executed directly but to be placed at the top of a c/c++ source file.
The options specified here are ones that either affect the operation of seabang or are passed to the compiler.
Options from the code file being run are passed in as usual, for example hello_world.cpp --help
seabang does not have any short options so that they do not clash with options for the compiler.

Please note, due to the way shebang options work, any argument for seabang and that takes an value
   must not contain a space. For example '--seabang-compiler=compiler' is ok,
   '--seabang-compiler = compiler' will fail.

The environment varible SEABANG_CXX_COMPILER can be used to change the compiler used by default.
    eg. SEABANG_CXX_COMPILER=gcc ./my-code.cpp

Mandatory arguments to long options are mandatory for short options too.
    --seabang-compiler=compiler Allows a specific source file to use a compiler that is not the norm.
              This overides the compiler set with SEABANG_CXX_COMPILER and the default one.
              Example, --seabang-compiler=gcc

    --verbose Enables logging so you can see what seabang is doing.
              Also enables verbose logging for the compiler.

    --rebuild Forces a rebuild of the code. Normally seabang will only
              build the code if the source file, or a dependency, has changed.

    --debug   By default the code is built with optimisations set to 2 and not symbol files created.
              This options turns of all optimisations and generates the symbols needed for debbugging.
              Turn on verbose output to discover the out location of the exec if you need to debug it.

All single dash options (eg -lncurses) are passed to the compiler. This allows you to have some more
control over the build settings. Such as specifying an optimisation option or a machine option.

Seabang is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
seabang is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with seabang.  If not, see <https://www.gnu.org/licenses/>.
