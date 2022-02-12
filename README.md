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

You can change the compiler that seabang calls. When seabang is built and installed you can set the compiler by passing in a define into cmake.

e.g.
```cmake -DSEABANG_CXX_COMPILER=gcc```

If you don't do this seabang will use the compiler that cmake chooses.

You can also change the default temporary folder seabang uses with the SEABANG_TEMPORARY_FOLDER define.
e.g.
```cmake -DSEABANG_TEMPORARY_FOLDER=~/temp```
    This option is handy if you have a ram disk created to tmpfs.
    Will make the results of the build vanish after boot and also work on read only disk systems, which is handy.

After seabang has been built and installed you can still change the compiler used with two options. Either set the environment variable SEABANG_CXX_COMPILER or call seabang in the shebang with the option --seabang-compiler=YOU-COMPILER. eg. --seabang-compiler=gcc


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

The environment varible SEABANG_TEMPORARY_FOLDER can be used to change the temporary folder used by default.
    eg. SEABANG_TEMPORARY_FOLDER=~/tmp ./my-code.cpp


Mandatory arguments to long options are mandatory for short options too.
    --seabang-compiler=compiler Allows a specific source file to use a compiler that is not the norm.
              This overides the compiler set with SEABANG_CXX_COMPILER and the default one.
              Example, --seabang-compiler=gcc

    --seabang-temp-path=PATH Allows a specific source file to use a particular temporay folder.
              This overides the compiler set with SEABANG_TEMPORARY_FOLDER and the default one.
              Example, --seabang-temp-path=./bin

    --verbose Enables logging so you can see what seabang is doing.
              Also enables verbose logging for the compiler.

    --rebuild Forces a rebuild of the code. Normally seabang will only
              build the code if the source file, or a dependency, has changed.

    --debug   By default the code is built with optimisations set to 2 and not symbol files created.
              This options turns of all optimisations and generates the symbols needed for debbugging.
              Turn on verbose output to discover the out location of the exec if you need to debug it.

    --compact-path By default the temporay folder used for the intermidiary files includes the path of the source file.
                   This is done to avoid file clashes. If there is a reason that this can not work for you
                   then this option removes this. The intermediary files will use the temporay path
                   plus the sources filename.

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

