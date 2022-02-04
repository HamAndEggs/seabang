/**
 * @file seabang.cpp
 * @author Richard e Collins
 * @version 0.1
 * @date 2021-10-24
 * 
 *  seabang is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  seabang is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with seabang.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include <limits.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <memory>
#include <assert.h>

#include "TinyTools.h"
#include "dependencies.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

static bool gVerboseLogging = false;

/**
 * @brief Get the source file from the arguments
 */
static const char* GetSourceFileFromArguments(int argc,char *argv[])
{
    if( argc == 2 )
        return argv[1];

    // If it's a file, assume seabang not passed with any argument.
    if( tinytools::file::FileExists(argv[1]) )
        return argv[1];

    // Assume if argv[1] is not a file then they must be arguments and so argv[2] is our file.
    return argv[2];
}

/**
 * @brief Get the Arguments that are for use in seabang.
 */
static const tinytools::StringVec GetArgumentsForSeabang(int argc,char *argv[])
{
    // If no arguments then pass back empty vector
    if( argc > 2 )
    {
        // If it's a file, assume seabang not passed with any argument.
        if( tinytools::file::FileExists(argv[1]) == false )
        {
            // Assume if argv[1] is not a file then they must be arguments and so argv[2] is our file.
            return tinytools::string::SplitString(argv[1]," ");
        }
    }

    return tinytools::StringVec();
}

/**
 * @brief Get the value that an argument is set to.
 * E.G Will return FRED for --name=FRED, also deals with spaces.
 * Has to assume if = is found after argument that the next arg is the value.
 */
static const std::string GetArgumentValue(const tinytools::StringVec& args, const std::string theArg)
{
    for( auto s : args )
    {
        if( tinytools::string::CompareNoCase(s,theArg) )
        {
            // Does the arg contain an = sign?
            const size_t equality = s.find('=');
            if( equality != std::string::npos )
            {
                // Grab the rest of the string as the value.
                return s.substr(equality+1);
            }
        }
    }
    return "";
}

/**
 * @brief Get the Arguments passed to the file that is being executed
 * These arguments are then passed to the compiled exec.
 * So you can do things like ./wait.cpp -n=3
 */
static const tinytools::StringVec GetArgumentsForApplication(int argc,char *argv[])
{
    tinytools::StringVec args;
    // If no arguments then pass back empty vector
    if( argc > 2 )
    {
        // If it's a file, assume seabang not passed with any argument.
        if( tinytools::file::FileExists(argv[1]) )
        {// Application arguments are as expected, one after another in the argument list, not one string.
            for(int n = 2 ; n < argc ; n++ )
            {
                args.push_back(argv[n]);
            }
        }
        else
        {
            // argv[1] was not a file, assume argv[2] is our file. So use argv[3] and one wards, if give.
            if( argc > 3 )
            {
                for(int n = 3 ; n < argc ; n++ )
                {
                    args.push_back(argv[n]);
                }
            }
        }

    }

    return args;
}

/**
 * @brief Get the single dashed arguments, passed onto the compiler
 */
static const tinytools::StringVec GetArgumentsForCompiler(const tinytools::StringVec& seaBangExtraArguments)
{
    // Scan the arguments for seabang and select which are know to be needed for the compiler.
    tinytools::StringVec compilerArgs;

    for( auto arg : seaBangExtraArguments )
    {
        if( arg.size() > 1 && arg[0] == '-' && std::isalnum(arg[1]) )
        {
            compilerArgs.push_back(arg);
        }
    }

    return compilerArgs;
}

/**
 * @brief When --verbose is used, this function is used to output the args to the log.
 */
static void LogArguments(const tinytools::StringVec& pArgs,const std::string& pWho)
{
    if( pArgs.size() == 0 )
    {
        std::clog << "No arguments for " << pWho << "\n";
    }
    else
    {
        std::clog << "arguments for " << pWho << " are:\n";
        for( auto a : pArgs )
        {
            std::clog << "    " << a << "\n";
        }
    }
}

/**
 * @brief Looks to see if there is a tmp folder in the users home directory, if not defaults to /tmp
 */
static std::string FindTemporayFolder()
{
    const char* home = getenv("HOME");
    if( home )
    {
        const std::string homeTmp = std::string(home) + "/tmp";
        if( tinytools::file::DirectoryExists(homeTmp) )
        {
            return homeTmp;
        }
    }
    // No reasonble alternative found so use the one that most distros use.
    return "/tmp";
}

/**
 * @brief Selects the compiler that we should used.
 */
const std::string SelectComplier(const tinytools::StringVec& seaBangExtraArguments)
{
    // The order is important and defined in the documentation.
    // First see if seabang was invoked with the complier overide.
    const std::string cmdCompiler = GetArgumentValue(seaBangExtraArguments,"--seabang-compiler");
    if( cmdCompiler.size() > 0 )
    {
        if( gVerboseLogging )
        {
            std::clog << "Compiler overwritten to use " << cmdCompiler << "\n";
        }
        return cmdCompiler;
    }

    // No argument passed, so see if an environment varible was set.
    const char* envCompiler = getenv("SEABANG_CXX_COMPILER");
    if( envCompiler != nullptr )
    {
        const std::string compiler = std::string(envCompiler);
        if( gVerboseLogging )
        {
            std::clog << "Compiler overwritten with environment varible, using " << compiler << "\n";
        }
        return compiler;
    }

    if( gVerboseLogging )
    {
        std::clog << "Using default compiler " << SEABANG_CXX_COMPILER << "\n";
    }

    return SEABANG_CXX_COMPILER;
}

/**
 * @brief Displays the help text.
 */
static void DisplayHelp()
{
const std::string_view helpText =
R"(
Usage: #!/usr/bin/seabang [OPTION]
This command is not to be executed directly but to be placed at the top of a c/c++ source file.
The options specified here are ones that either affect the operation of seabang or are passed to the compiler.
Options from the code file being run are passed in as usual, for example hello_world.cpp --help
seabang does not have any short options so that they do not clash with options for the compiler.
Compiler built to use )" TOSTRING(SEABANG_CXX_COMPILER) R"(
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

)";

    std::cout << helpText << "\n";
}

/**
 * @brief Our entrypoint called by the OS
 */
int main(int argc,char *argv[])
{
    assert(tinytools::string::CompareNoCase("onetwo","one",3) == true);
    assert(tinytools::string::CompareNoCase("onetwo","ONE",3) == true);
    assert(tinytools::string::CompareNoCase("OneTwo","one",3) == true);
    assert(tinytools::string::CompareNoCase("onetwo","oneX",3) == true);
    assert(tinytools::string::CompareNoCase("OnE","oNe") == true);
    assert(tinytools::string::CompareNoCase("onetwo","one") == true);	// Does it start with 'one'
    assert(tinytools::string::CompareNoCase("onetwo","onetwothree",6) == true);
    assert(tinytools::string::CompareNoCase("onetwo","onetwothreeX",6) == true);
    assert(tinytools::string::CompareNoCase("onetwo","onetwothree") == false); // sorry, but we're searching for more than there is... false...
    assert(tinytools::string::CompareNoCase("onetwo","onetwo") == true);

    // See if they are looking for seabang help.
    if( argc == 2 )
    {
        if( tinytools::string::CompareNoCase(argv[1],"--help") )
        {
            DisplayHelp();
            return EXIT_SUCCESS;
        }
    }

    // Got to be at least two args.
    if( argc < 2 )
    {
        std::cerr << "Seabang not run from a source file, expects at least two command line arguments.\n";
        DisplayHelp();
        return EXIT_FAILURE;
    }

    // The way the commandline works with a shebang is...
    // argv[0] is the shebang exec name, so in our case will be seabang
    // then comes the arguments passed to the seabang, in the source file, all as one argument, [1]
    // If no arguments given then argv[1] will be the source file.
    // If arguments were given then the source file will be in argv[2]
    // Then the rest of the arguments are as we expect, one argv[n] per argument.
    // And so we need to look if argv[1] is a file or not. If it is assume no args passed to the shebang, if it is not assume it's args for the seabang exec.
    const std::string originalSourceFile = GetSourceFileFromArguments(argc,argv);
    const tinytools::StringVec seaBangExtraArguments = GetArgumentsForSeabang(argc,argv);
    const tinytools::StringVec applicationArguments = GetArgumentsForApplication(argc,argv);
    const tinytools::StringVec compilerExtraArguments = GetArgumentsForCompiler(seaBangExtraArguments);

    // Lets see if they want verbose logging.
    // All seabang arguments are in long form so not to get mixed up with arguments for the compiler.
    gVerboseLogging = tinytools::string::Search(seaBangExtraArguments,"--verbose");
    bool rebuildNeeded = tinytools::string::Search(seaBangExtraArguments,"--rebuild");
    const bool debugBuild = tinytools::string::Search(seaBangExtraArguments,"--debug");

    if( gVerboseLogging )
    {
        std::clog << "originalSourceFile == " << originalSourceFile << "\n";
        LogArguments(seaBangExtraArguments,"seabang");
        LogArguments(applicationArguments,"application");
        LogArguments(compilerExtraArguments,"compiler");
    }

    const std::string CWD = tinytools::file::GetCurrentWorkingDirectory() + "/";
    const std::string sourcePathedFile = tinytools::file::CleanPath(CWD + originalSourceFile);

    // Sanity check, is file there?
    if( tinytools::file::FileExists(sourcePathedFile) == false )
        return EXIT_FAILURE;

    // Get the path to the source file, need this as we need to insert the project configuration name so we can find it.
    const std::string sourceFilePath = tinytools::file::GetPath(sourcePathedFile);

    // This is the temp folder path we use to cache build results.
    const std::string tempFolderPath(FindTemporayFolder() + "/seabang/");

    // Use the source file as the 'project name' for error reports.
    // This is the name of the project that the user will expect to see in errors.
    const std::string usersProjectName = sourcePathedFile;

    // Now we need to create the path to the compiled exec.
    // This is done so we only have to build when something changes.
    // To ensure no clashes I take the fully pathed source file name
    // and add /tmp to the start for the temp folder. I also add .exe at the end.
    const std::string exename = tinytools::file::GetFileName(sourcePathedFile) + ".exe";
    const std::string pathedExeName = tinytools::file::CleanPath(tempFolderPath + sourceFilePath + exename);

    // We need the source file without the shebang too.
    const std::string tempSourcefile = tinytools::file::CleanPath(tempFolderPath + sourcePathedFile);

    // The temp folder that it's all done in.
    const std::string projectTempFolder = tinytools::file::GetPath(tempSourcefile);

    // Pick the compiler that the user wants or was selected when the tool was built.
    const std::string CompilerToUse = SelectComplier(seaBangExtraArguments);

#ifdef DEBUG_BUILD
    std::cout << "CWD " << CWD << std::endl;
    std::cout << "sourcePathedFile " << sourcePathedFile << std::endl;
    std::cout << "exenameFileExists " << pathedExeName << std::endl;
#endif

    // Make sure our temp folder is there.
    tinytools::file::MakeDir(projectTempFolder);

    tinytools::StringVec libraryFiles;

    // First see if the source file that does not have the shebang in it is there.
    // May have been forced on.
    if( rebuildNeeded == false )
    {
        rebuildNeeded = tinytools::file::CompareFileTimes(sourcePathedFile,tempSourcefile);
    }

    // Ok, so the source file may not have changed but has any of it's dependencies?
    // This will also check the age of the source file against the age of the executable file.
    // Don't need to do this if we're building anyway.
    if( rebuildNeeded == false )
    {
        tinytools::StringVec includePaths;
        includePaths.push_back("/usr/include");
        includePaths.push_back("/usr/local/include");
        includePaths.push_back(CWD);
        Dependencies sourceFileDependencies;
        rebuildNeeded = sourceFileDependencies.RequiresRebuild(sourcePathedFile,pathedExeName,includePaths);
    }

    if( rebuildNeeded )
    {
#ifdef DEBUG_BUILD
        std::cout << "Source file rebuild needed!" << std::endl;
#endif
        // Ok, we better build it.
        // Copy all lines of the file over excluding the first line that has the shebang.
        std::string line;
        std::ifstream oldSource(sourcePathedFile);
        std::ofstream newSource(tempSourcefile);
        if( newSource )
        {
            bool foundShebang = false;

            while( std::getline(oldSource,line) )
            {
                if( gVerboseLogging )
                {
                    std::cout << line << std::endl;
                }

                if( line[0] == '#' && line[1] == '!' ) // Is it the shebang? If so remove it.
                {
                    foundShebang = true;
                }
                else
                {
                    newSource << line << std::endl;
                }
            }

            if( !foundShebang )
            {
                std::cerr << "Failed to parse the source file..." << std::endl;
                return EXIT_FAILURE;
            }
        }
        else
        {
            std::cerr << "Failed to parse the source file into new temp file..." << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if( gVerboseLogging )
    {
        std::cout << "Skipping rebuild, executable is not out of date." << std::endl;
    }

    if( chdir(projectTempFolder.c_str()) != 0 )
    {
        std::cerr << "Failed to " << projectTempFolder << std::endl;
        return EXIT_FAILURE;
    }

    // No point doing the link stage is the source file has not changed!
    bool compliedOK = true;
    if( rebuildNeeded )
    {
        // Make source output is deleted so can run if there was a build error.
        std::remove(pathedExeName.c_str());

        // First compile the new source file that is in the temp folder, this has the she bang removed, so it'll compile.
        tinytools::StringVec args;

        args.push_back(tempSourcefile);

        // For now, we'll build a release build. Later I'll add an option for a debug or release to be selected in the comand line options to seabang.
        if( debugBuild )
        {
            args.push_back("-g2");
            args.push_back("-DDEBUG_BUILD");
        }
        else
        {
            args.push_back("-o2");
            args.push_back("-g0");
            args.push_back("-DRELEASE_BUILD");
            args.push_back("-DNDEBUG");
        }

        // Need to add the current working dir as a search path.
        // This is because the file maybe including a a file from a local path and not the system include folder.
        // E.g #include "../somecode.cpp"
        args.push_back("-I" + CWD);

        // For now we'll assume c++17, later add option to allow them to define this. Will always default to c++17
        args.push_back("-std=c++17");
        args.push_back("-Wall"); // Lots of warnings please.
        
        args.push_back("-lm");  // Maths libs
        args.push_back("-lstdc++");  // C++ stuff
        args.push_back("-lpthread");  // For threading

        if( gVerboseLogging )
        {
            args.push_back("-v");
        }

        // Add stuff passed in for the compiler
        for( auto arg : compilerExtraArguments )
        {
            args.push_back(arg);
        }

        // And set the output file.
        args.push_back("-o");
        args.push_back(pathedExeName);

        std::string compileOutput;
        compliedOK = tinytools::system::ExecuteShellCommand(CompilerToUse,args,compileOutput);
        if( compileOutput.size() > 0 && (compliedOK == false || gVerboseLogging ) )
        {
            std::clog << compileOutput << "\n";
        }
    }

    // See if we have the output file, if so run it!
    if( compliedOK && tinytools::file::FileExists(pathedExeName) )
    {// I will not be using ExecuteShellCommand as I need to replace this exec to allow the input and output to be taken over.

        if( chdir(CWD.c_str()) != 0 )
        {
            std::cerr << "Failed to return to the original run folder " << CWD << std::endl;
            return EXIT_FAILURE;
        }

        if( gVerboseLogging )
        {
            std::clog << "Running exec: " << pathedExeName << "\n";
        }

        // The args sent into shebang for the app, then +1 for the NULL and +1 for the file name as per convention, see https://linux.die.net/man/3/execlp.
        char** TheArgs = new char*[applicationArguments.size() + 2];
        int c = 0;
        TheArgs[c++] = tinytools::string::CopyString(pathedExeName);
        for( auto arg : applicationArguments )
        {
            TheArgs[c++] = tinytools::string::CopyString(arg);
        }
        TheArgs[c++] = NULL;

        // This replaces the current process so no need to clean up the memory leaks before here. ;)
        execvp(TheArgs[0], TheArgs);

        std::cerr << "ExecuteShellCommand execl() failure!" << std::endl << "This print is after execl() and should not have been executed if execl were successful!" << std::endl;
        _exit(1);

    }
    else
    {
        std::cerr << "Failed to find executable " << pathedExeName << std::endl;
        return EXIT_FAILURE;
    }

    if( chdir(CWD.c_str()) != 0 )
    {
        std::cerr << "Failed to return to the original run folder " << CWD << std::endl;
    }

    return EXIT_FAILURE;
}
