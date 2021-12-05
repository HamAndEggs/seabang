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

Usage: #!/usr/bin/seabang [OPTION]
This command is not to be executed directly but to be placed at the top of a c/c++ source file.
The options specified here are ones that either affect the operation of seabang or are passed to the compiler.
Options from the code file being run are passed in as usual, for example hello_world.cpp --help
seabang does not have any short options so that they do not clash with options for the compiler.
Compiler currently used is fixed as g++.

Mandatory arguments to long options are mandatory for short options too.
    --verbose Enables logging so you can see what seabang is doing.
              Also enables verbose logging for the compiler.

    --rebuild Forces a rebuild of the code. Normally seabang will only
              build the code if the source file, or a dependency, has changed.

    --debug   By default the code is built with optimisations set to 2 and not symbol files created.
              This options turns of all optimisations and generates the symbols needed for debbugging.
              Turn on verbose output to discover the out location of the exec if you need to debug it.

    -D name
            Predefine name as a macro, with definition 1.

    -D name=definition
           The contents of definition are tokenized and processed as if
           they appeared during translation phase three in a #define
           directive.  In particular, the definition is truncated by
           embedded newline characters.

           If you are invoking the preprocessor from a shell or shell-
           like program you may need to use the shell's quoting syntax
           to protect characters such as spaces that have a meaning in
           the shell syntax.

           If you wish to define a function-like macro on the command
           line, write its argument list with surrounding parentheses
           before the equals sign (if any).  Parentheses are meaningful
           to most shells, so you should quote the option.  With sh and
           csh, -D'name(args...)=definition' works.

           -D and -U options are processed in the order they are given
           on the command line.  All -imacros file and -include file
           options are processed after all -D and -U options.

    -llibrary
    -l library
           Search the library named library when linking.  (The second
           alternative with the library as a separate argument is only
           for POSIX compliance and is not recommended.)

           The -l option is passed directly to the linker by GCC.  Refer
           to your linker documentation for exact details.  The general
           description below applies to the GNU linker.

           The linker searches a standard list of directories for the
           library.  The directories searched include several standard
           system directories plus any that you specify with -L.

           Static libraries are archives of object files, and have file
           names like liblibrary.a.  Some targets also support shared
           libraries, which typically have names like liblibrary.so.  If
           both static and shared libraries are found, the linker gives
           preference to linking with the shared library unless the
           -static option is used.

           It makes a difference where in the command you write this
           option; the linker searches and processes libraries and
           object files in the order they are specified.  Thus, foo.o
           -lz bar.o searches library z after file foo.o but before
           bar.o.  If bar.o refers to functions in z, those functions
           may not be loaded.
