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
#include "execute_command.h"
#include "dependencies.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

static bool gVerboseLogging = false;

#define VLOG(__THING_TO_LOG) {if( gVerboseLogging ){std::clog << __THING_TO_LOG << "\n";}}

static std::vector<std::string> SplitString(const std::string& pString, const char* pSeperator)
{
    std::vector<std::string> res;
    for (size_t p = 0, q = 0; p != pString.npos; p = q)
	{
		const std::string part(pString.substr(p + (p != 0), (q = pString.find(pSeperator, p + 1)) - p - (p != 0)));
		if( part.size() > 0 )
		{
	        res.push_back(part);
		}
	}
    return res;
}

inline bool SearchString(const std::vector<std::string>& pVec,const std::string& pLost)
{
   return std::find(pVec.begin(),pVec.end(),pLost) != pVec.end();
}

/**
 * @brief Does an ascii case insensitive test within the full string or a limited start of the string.
 * 
 * @param pA 
 * @param pB 
 * @param pLength If == 0 then length of second string is used. If first is shorter, will always return false.
 * @return true 
 * @return false 
 */
bool CompareNoCase(const char* pA,const char* pB,size_t pLength)
{
    assert( pA != nullptr || pB != nullptr );// Note only goes pop if both are null.
// If either or both NULL, then say no. A bit like a divide by zero as null strings are not strings.
    if( pA == nullptr || pB == nullptr )
        return false;

// If same memory then yes they match, doh!
    if( pA == pB )
        return true;

    if( pLength == 0 )
        pLength = strlen(pB);

    while( (*pA != 0 || *pB != 0) && pLength > 0 )
    {
        // Get here are one of the strings has hit a null then not the same.
        // The while loop condition would not allow us to get here if both are null.
        if( *pA == 0 || *pB == 0 )
        {// Check my assertion above that should not get here if both are null. Note only goes pop if both are null.
            assert( pA != NULL || pB != NULL );
            return false;
        }

        if( tolower(*pA) != tolower(*pB) )
            return false;

        pA++;
        pB++;
        pLength--;
    };

    // Get here, they are the same.
    return true;
}
inline bool CompareNoCase(const std::string& pA,const std::string& pB,size_t pLength = 0)
{
	return CompareNoCase(pA.c_str(),pB.c_str(),pLength);
}

/**
 * @brief Get the source file from the arguments
 */
static const char* GetSourceFileFromArguments(int argc,char *argv[])
{
    if( argc == 2 )
        return argv[1];

    // If it's a file, assume seabang not passed with any argument.
    if( std::filesystem::exists(argv[1]) )
        return argv[1];

    // Assume if argv[1] is not a file then they must be arguments and so argv[2] is our file.
    return argv[2];
}

/**
 * @brief Get the Arguments that are for use in seabang.
 */
static const std::vector<std::string> GetArgumentsForSeabang(int argc,char *argv[])
{
    // If no arguments then pass back empty vector
    if( argc > 2 )
    {
        // If it's a file, assume seabang not passed with any argument.
        if( std::filesystem::exists(argv[1]) == false )
        {
            // Assume if argv[1] is not a file then they must be arguments and so argv[2] is our file.
            return SplitString(argv[1]," ");
        }
    }

    return std::vector<std::string>();
}

/**
 * @brief Get the value that an argument is set to.
 * E.G Will return FRED for --name=FRED, also deals with spaces.
 * Has to assume if = is found after argument that the next arg is the value.
 */
static const std::string GetArgumentValue(const std::vector<std::string>& args, const std::string theArg)
{
    for( auto s : args )
    {
        if( CompareNoCase(s,theArg) )
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
static const std::vector<std::string> GetArgumentsForApplication(int argc,char *argv[])
{
    std::vector<std::string> args;
    // If no arguments then pass back empty vector
    if( argc > 2 )
    {
        // If it's a file, assume seabang not passed with any argument.
        if( std::filesystem::exists(argv[1]) )
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
static const std::vector<std::string> GetArgumentsForCompiler(const std::vector<std::string>& seaBangExtraArguments)
{
    // Scan the arguments for seabang and select which are know to be needed for the compiler.
    std::vector<std::string> compilerArgs;

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
static void LogArguments(const std::vector<std::string>& pArgs,const std::string& pWho)
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

static std::string CorrectTemparyFolderString(std::string pFolder)
{
    VLOG("CorrectTemparyFolderString(" << pFolder << ")")
    if( pFolder.back() != '/' )
        pFolder += '/';

    // Check for ~ home char. Need to replace with true HOME. Seen issues if I do not, makes a folder in CWD called '~'
    if( pFolder.front() == '~' )
    {
        if( getenv("HOME") )
        {
            pFolder = std::string(getenv("HOME")) + '/' + pFolder.substr(1);
        }
    }
    return pFolder;
}

// std::filesystem::path::compare just checks the string, does not check the file the path points too.
static bool AreFilesTheSame(const std::filesystem::path pFileA,const std::filesystem::path pFileB)
{
    VLOG("Comparing fileA " << pFileA << "with file B " << pFileB);
    struct stat A;
    struct stat B;
    if( stat(pFileA.string().c_str(),&A) != -1 && stat(pFileB.string().c_str(),&B) != -1 )
    {
        return A.st_ino == B.st_ino && A.st_dev == B.st_dev;
    }

    // Ok to return false here as if one or both can't be opened then safe to assume can't be the same file.
    return false;
}

/**
 * @brief Allows the user to alter the temp folder used.
 * The order is important and defined in the documentation.
 */
static std::filesystem::path FindTemporayFolder(const std::vector<std::string>& seaBangExtraArguments)
{
    // First see if seabang was invoked with the temporay path overide.
    const std::filesystem::path cmdTempFolder = GetArgumentValue(seaBangExtraArguments,"--seabang-temp-path");
    if( cmdTempFolder.empty() )
    {
        // Next see if the temp folder is defined in an environment varible.
        if( getenv("SEABANG_TEMPORARY_FOLDER") != nullptr )
        {
            const std::filesystem::path tempFolder = CorrectTemparyFolderString(getenv("SEABANG_TEMPORARY_FOLDER"));
            if( tempFolder.empty() == false && std::filesystem::is_directory(tempFolder) && std::filesystem::exists(tempFolder) )
            {
                return tempFolder;
            }
        }

        VLOG("No reasonble tempory folder alternative found so use the one that setup in the make file. (which can be configured with cmake)");
        return CorrectTemparyFolderString(SEABANG_TEMPORARY_FOLDER);
    }

    VLOG("Compiler overwritten to use " << cmdTempFolder);
    return CorrectTemparyFolderString(cmdTempFolder);
}

/**
 * @brief Selects the compiler that we should used.
 */
const std::string SelectComplier(const std::vector<std::string>& seaBangExtraArguments)
{
    // The order is important and defined in the documentation.
    // First see if seabang was invoked with the complier overide.
    const std::string cmdCompiler = GetArgumentValue(seaBangExtraArguments,"--seabang-compiler");
    if( cmdCompiler.size() > 0 )
    {
        VLOG("Compiler overwritten to use " << cmdCompiler);
        return cmdCompiler;
    }

    // No argument passed, so see if an environment varible was set.
    const char* envCompiler = getenv("SEABANG_CXX_COMPILER");
    if( envCompiler != nullptr )
    {
        const std::string compiler = std::string(envCompiler);
        VLOG("Compiler overwritten with environment varible, using " << compiler);
        return compiler;
    }

    VLOG("Using default compiler " << SEABANG_CXX_COMPILER);

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
seabang was built to use the Compiler )" TOSTRING(SEABANG_CXX_COMPILER) R"(
seaband was built to use the temporay folder )" TOSTRING(SEABANG_TEMPORARY_FOLDER) R"(
Please note, due to the way shebang options work, any argument for seabang and that takes an value
   must not contain a space. For example '--seabang-compiler=compiler' is ok,
   '--seabang-compiler = compiler' will fail.

The environment varible SEABANG_CXX_COMPILER can be used to change the compiler used by default.
    eg. SEABANG_CXX_COMPILER=gcc ./my-code.cpp

The environment varible SEABANG_TEMPORARY_FOLDER can be used to change the tempoary folder used by default.
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

)";

    std::cout << helpText << "\n";
}

static std::filesystem::path ChooseTempSourceFilename(const std::filesystem::path &tempFolderPath,bool compactTempPath,const std::filesystem::path &pathedSourceFile)
{
    std::filesystem::path pathedFilename = tempFolderPath;
    if( compactTempPath )
    {
        pathedFilename /= ".temp";
        pathedFilename /= pathedSourceFile.filename();
    }
    else
    {
        pathedFilename += pathedSourceFile;
    }

    // If the file does not have an extension, assume cpp file so add one.
    // This allows source files to look like applications.
    if( pathedFilename.has_extension() == false )
    {
        pathedFilename += ".cpp";
    }

    return pathedFilename;
}

/**
 * @brief Our entrypoint called by the OS
 */
int main(int argc,char *argv[])
{
    assert(CompareNoCase("onetwo","one",3) == true);
    assert(CompareNoCase("onetwo","ONE",3) == true);
    assert(CompareNoCase("OneTwo","one",3) == true);
    assert(CompareNoCase("onetwo","oneX",3) == true);
    assert(CompareNoCase("OnE","oNe") == true);
    assert(CompareNoCase("onetwo","one") == true);	// Does it start with 'one'
    assert(CompareNoCase("onetwo","onetwothree",6) == true);
    assert(CompareNoCase("onetwo","onetwothreeX",6) == true);
    assert(CompareNoCase("onetwo","onetwothree") == false); // sorry, but we're searching for more than there is... false...
    assert(CompareNoCase("onetwo","onetwo") == true);

    // See if they are looking for seabang help.
    if( argc == 2 )
    {
        if( CompareNoCase(argv[1],"--help") )
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
    const std::vector<std::string> seaBangExtraArguments = GetArgumentsForSeabang(argc,argv);
    const std::vector<std::string> applicationArguments = GetArgumentsForApplication(argc,argv);
    const std::vector<std::string> compilerExtraArguments = GetArgumentsForCompiler(seaBangExtraArguments);

    // Lets see if they want verbose logging.
    // All seabang arguments are in long form so not to get mixed up with arguments for the compiler.
    gVerboseLogging = SearchString(seaBangExtraArguments,"--verbose");
    bool rebuildNeeded = SearchString(seaBangExtraArguments,"--rebuild");
    const bool debugBuild = SearchString(seaBangExtraArguments,"--debug");
    const bool compactTempPath = SearchString(seaBangExtraArguments,"--compact-path");

    if( gVerboseLogging )
    {
        LogArguments(seaBangExtraArguments,"seabang");
        LogArguments(applicationArguments,"application");
        LogArguments(compilerExtraArguments,"compiler");
    }

    const std::filesystem::path CWD = std::filesystem::current_path();
    const std::filesystem::path pathedSourceFile = CWD / originalSourceFile;

    // Sanity check, is file there? This is done to check for errors in the logic of the code above. 
    if( std::filesystem::exists(pathedSourceFile) == false )
    {
        VLOG("The source file we are trying to run is not found at: " << pathedSourceFile);
        return EXIT_FAILURE;
    }

    // Get the path to the source file, need this as we need to insert the project configuration name so we can find it.
    const std::filesystem::path sourceFilePath = std::filesystem::path(pathedSourceFile).remove_filename();

    // This is the temp folder path we use to cache build results.
    const std::filesystem::path tempFolderPath(FindTemporayFolder(seaBangExtraArguments));

    // We need the source file without the shebang too.
    const std::filesystem::path tempSourcefile = ChooseTempSourceFilename(tempFolderPath,compactTempPath,pathedSourceFile);

    // Now we need to create the path to the compiled exec.
    // This is done so we only have to build when something changes.
    // To ensure no clashes I take the fully pathed temporay source file name and add .exe at the end.
    const std::filesystem::path pathedExeName = (std::filesystem::path(tempSourcefile) += ".exe");


    // The temp folder that it's all done in.
    const std::filesystem::path projectTempFolder = std::filesystem::path(tempSourcefile).remove_filename();

    // Pick the compiler that the user wants or was selected when the tool was built.
    const std::string CompilerToUse = SelectComplier(seaBangExtraArguments);

    // Make sure the new temp source file is not pointing to original source file.
    if( AreFilesTheSame(pathedSourceFile,tempSourcefile) )
    {
        VLOG("Error in temporay path. Original source file is same as temporay source file.\n    " << originalSourceFile << "\n    " << tempSourcefile);
        return EXIT_FAILURE;
    }

    VLOG("Source file " << pathedSourceFile);
    VLOG("Temp Source file " << tempSourcefile);
    VLOG("exe file name " << pathedExeName);

    // Make sure our temp folder is there.
    std::filesystem::create_directories(projectTempFolder);

    std::vector<std::string> libraryFiles;

    // Check the temp source that is compiled is there and that it's date is not older than the one we're executing.
    // Will also 
    // May have been forced on.
    if( rebuildNeeded == false )
    {
        if( std::filesystem::exists(tempSourcefile) == false || std::filesystem::last_write_time(tempSourcefile) < std::filesystem::last_write_time(pathedSourceFile) )
        {
            rebuildNeeded = true;
            VLOG("File times differ, need to rebuild");
        }
    }
    else
    {
        VLOG("Comandline forcing build");
    }

    // Ok, so the source file may not have changed but has any of it's dependencies?
    // We do this as the exec maybe including a header in the same folder or from else where that maybe changing.
    // This will also check the age of the source file against the age of the executable file.
    // Don't need to do this if we're building anyway.
    if( rebuildNeeded == false )
    {   // We don't need to add the paths for the c/c++ includes as we don't care if we can't find the file to check.
        // They should not be changing. We only care about the files that the source file refers to in it's own folder.
        // And also the exec that it ultimately builds.
        Dependencies::PathVec includePaths;
        includePaths.push_back(CWD);
        Dependencies sourceFileDependencies;
        rebuildNeeded = sourceFileDependencies.RequiresRebuild(pathedSourceFile,pathedExeName,includePaths);
        if( rebuildNeeded )
            VLOG("Dependency check says we need a rebuild")
        else
            VLOG("Dependency check says, NO rebuild needed")
    }
    else
    {
        VLOG("We already know we need a rebuild, skipping dependency check");
    }

    if( rebuildNeeded )
    {
        VLOG("Source file rebuild needed!");

        // Ok, we better build it.
        // Copy all lines of the file over excluding the first line that has the shebang.
        std::string line;
        std::ifstream oldSource(pathedSourceFile);
        std::ofstream newSource(tempSourcefile);
        if( newSource )
        {
            bool foundShebang = false;

            while( std::getline(oldSource,line) )
            {
                VLOG(line);

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
    else
    {
        VLOG("Skipping rebuild, executable is not out of date.");
    }

    // No point doing the link stage is the source file has not changed!
    bool compliedOK = true;
    if( rebuildNeeded )
    {
        // Make source output is deleted so can run if there was a build error.
        std::filesystem::remove(pathedExeName);

        // First compile the new source file that is in the temp folder, this has the she bang removed, so it'll compile.
        std::vector<std::string> args;

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
        args.push_back("-I" + CWD.string());

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

        if( gVerboseLogging )
        {
            std::cout << CompilerToUse << " ";
            for(auto s : args )
            {
                std::cout << s << " ";
            }
            std::cout << "\n\n";
        }

        std::string compileOutput;
        compliedOK = ExecuteShellCommand(CompilerToUse,args,compileOutput);
        if( compileOutput.size() > 0 && (compliedOK == false || gVerboseLogging ) )
        {
            std::clog << compileOutput << "\n";
        }
    }

    // See if we have the output file, if so run it!
    if( compliedOK && std::filesystem::exists(pathedExeName) )
    {// I will not be using ExecuteShellCommand as I need to replace this exec to allow the input and output to be taken over.

        if( chdir(CWD.c_str()) != 0 )
        {
            std::cerr << "Failed to return to the original run folder " << CWD << std::endl;
            return EXIT_FAILURE;
        }
        VLOG("Running exec: " << pathedExeName);

        std::string cmd = pathedExeName.string();
        for( auto arg : applicationArguments )
        {
            cmd += " ";
            cmd += arg;
        }
        return std::system(cmd.c_str());

    }
    else
    {
        std::cerr << "Failed to find executable " << pathedExeName << std::endl;
        return EXIT_FAILURE;
    }

//    if( chdir(CWD.c_str()) != 0 )
//    {
//        std::cerr << "Failed to return to the original run folder " << CWD << std::endl;
//    }

    return EXIT_SUCCESS;
}
