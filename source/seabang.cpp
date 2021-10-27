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

#include <sys/stat.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <memory>

#include "misc.h"
#include "shell.h"

typedef struct stat FileStats;

static bool gVerboseLogging = false;

/**
 * @brief Checks if the file for source is newer than of dest.
 * Also will pretend that it is newer if any of the stats fail.
 * This is a bespoke to my needs here, should not be exported to rest of the code.
 * 
 * @param pSourceFile 
 * @param pDestFile 
 * @return true 
 * @return false 
 */
static bool CompareFileTimes(const std::string& pSourceFile,const std::string& pDestFile)
{
    if( gVerboseLogging )
    {
        std::cout << "CompareFileTimes(" << pSourceFile << "," << pDestFile << ")" << std::endl;
    }

    FileStats Stats;
    if( stat(pDestFile.c_str(), &Stats) == 0 && S_ISREG(Stats.st_mode) )
    {
        timespec dstFileTime = Stats.st_mtim;

        if( stat(pSourceFile.c_str(), &Stats) == 0 && S_ISREG(Stats.st_mode) )
        {
            timespec srcFileTime = Stats.st_mtim;

            if(srcFileTime.tv_sec == dstFileTime.tv_sec)
                return srcFileTime.tv_nsec > dstFileTime.tv_nsec;

            return srcFileTime.tv_sec > dstFileTime.tv_sec;
        }
    }

    return true;
}

static const char* GetSourceFileFromArguments(int argc,char *argv[])
{
    if( argc == 2 )
        return argv[1];

    // If it's a file, assume seabang not passed with any argument.
    if( FileExists(argv[1]) )
        return argv[1];

    // Assume if argv[1] is not a file then they must be arguments and so argv[2] is our file.
    return argv[2];
}

static const StringVec GetArgumentsForSeabang(int argc,char *argv[])
{
    // If not arguments then pass back empty vector
    if( argc > 2 )
    {
        // If it's a file, assume seabang not passed with any argument.
        if( FileExists(argv[1]) == false )
        {
            // Assume if argv[1] is not a file then they must be arguments and so argv[2] is our file.
            return SplitString(argv[1]," ");
        }
    }

    return StringVec();
}

static const StringVec GetArgumentsForApplication(int argc,char *argv[])
{
    StringVec args;
    // If not arguments then pass back empty vector
    if( argc > 2 )
    {
        // If it's a file, assume seabang not passed with any argument.
        if( FileExists(argv[1]) )
        {// Application arguments are as expected, one after another in the argument list, not one string.
            for(int n = 2 ; n < argc ; n++ )
            {
                args.push_back(argv[n]);
            }
        }
        else
        {
            // argv[1] was not a file, assume argv[2] is oyr file. So use argv[3] and one wards, if give.
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

static void LogArguments(const StringVec& pArgs,const std::string& pWho)
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

int main(int argc,char *argv[])
{
    // Got to be at least two args.
    if( argc < 2 )
    {
        std::cerr << "seabang not run from a source file, expects at least two command line arguments.\n";
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
    const StringVec seaBangExtraArguments = GetArgumentsForSeabang(argc,argv);
    const StringVec applicationExtraArguments = GetArgumentsForApplication(argc,argv);

    // Lets see if they want verbose logging.
    gVerboseLogging = Search(seaBangExtraArguments,"-v");
    const bool releaseBuild = true; // Add option for this.

    if( gVerboseLogging )
    {
        std::clog << "originalSourceFile == " << originalSourceFile << "\n";
        LogArguments(seaBangExtraArguments,"seabang");
        LogArguments(applicationExtraArguments,"application");
    }

    const size_t ARG_MAX = 4096;

    const std::string CWD = GetCurrentWorkingDirectory() + "/";
    const std::string sourcePathedFile = CleanPath(CWD + originalSourceFile);

    // Sanity check, is file there?
    if( FileExists(sourcePathedFile) == false )
        return EXIT_FAILURE;

    // Get the path to the source file, need this as we need to insert the project configuration name so we can find it.
    const std::string sourceFilePath = GetPath(sourcePathedFile);

    // This is the temp folder path we use to cache build results.
    const std::string tempFolderPath("/tmp/seabang/");

    // Use the source file as the 'project name' for error reports.
    // This is the name of the project that the user will expect to see in errors.
    const std::string usersProjectName = sourcePathedFile;

    // Now we need to create the path to the compiled exec.
    // This is done so we only have to build when something changes.
    // To ensure no clashes I take the fully pathed source file name
    // and add /tmp to the start for the temp folder. I also add .exe at the end.
    const std::string exename = GetFileName(sourcePathedFile) + ".exe";
    const std::string pathedExeName = CleanPath(tempFolderPath + sourceFilePath + exename);

    // We need the source file without the shebang too.
    const std::string tempSourcefile = CleanPath(tempFolderPath + sourcePathedFile);

    // The temp folder that it's all done in.
    const std::string projectTempFolder = GetPath(tempSourcefile);

#ifdef DEBUG_BUILD
    std::cout << "CWD " << CWD << std::endl;
    std::cout << "sourcePathedFile " << sourcePathedFile << std::endl;
    std::cout << "exenameFileExists " << pathedExeName << std::endl;
#endif

    // Make sure our temp folder is there.
    MakeDir(projectTempFolder);

    StringVec libraryFiles;

    // First see if the source file that does not have the shebang in it is there.
    bool rebuildNeeded = CompareFileTimes(sourcePathedFile,tempSourcefile);

    // Now, it may have built but failed to link, so we then need to change is the output is there.
    if( rebuildNeeded == false )
    {
        rebuildNeeded = CompareFileTimes(sourcePathedFile,pathedExeName);
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
    if( rebuildNeeded )
    {
        // First compile the new source file that is in the temp folder, this has the she bang removed, so it'll compile.
        StringVec args;

        args.push_back(tempSourcefile);

        // For now, we'll build a release build. Later I'll add an option for a debug or release to be selected in the comand line options to seabang.
        if( releaseBuild )
        {
            args.push_back("-o2");
            args.push_back("-g0");
            args.push_back("-DRELEASE_BUILD");
            args.push_back("-DNDEBUG");
        }
        else
        {
            args.push_back("-o0");
            args.push_back("-g2");
            args.push_back("-DDEBUG_BUILD");
        }

        // For now we'll assume c++17, later add option to allow them to define this. Will always default to c++17
        args.push_back("-std=c++17");
        args.push_back("-Wall"); // Lots of warnings please.
        
        args.push_back("-lm");  // Maths libs
        args.push_back("-lstdc++");  // C++ stuff
        args.push_back("-lpthread");  // For threading

        args.push_back("-o");
        args.push_back(pathedExeName);

        std::string compileOutput;
        const bool comiledOK = ExecuteShellCommand("g++",args,compileOutput);
        if( compileOutput.size() > 0 && (comiledOK == false || gVerboseLogging ) )
        {
            std::clog << compileOutput << "\n";
        }
    }


    // See if we have the output file, if so run it!
    if( FileExists(pathedExeName) )
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

        char** TheArgs = new char*[(argc - 3) + 2];// -3 for the args sent into shebang, then +1 for the NULL and +1 for the file name as per convention, see https://linux.die.net/man/3/execlp.
        int c = 0;
        TheArgs[c++] = CopyString(pathedExeName);
        for( int n = 3 ; n < argc ; n++ )
        {
            char* str = CopyString(argv[n],ARG_MAX);
            if(str)
            {
                //Trim leading white space.
                while(isspace(str[0]) && str[0])
                    str++;
                if(str[0])
                    TheArgs[c++] = str;
            }
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
