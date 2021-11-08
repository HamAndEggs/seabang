/**
 * @file dependencies.cpp
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

#ifndef __DEPENDENCIES_H__
#define __DEPENDENCIES_H__

#include <inttypes.h>
#include <stdint.h>
#include <functional>
#include <unordered_map>

#include "TinyTools.h"

class Dependencies
{
public:
	Dependencies() = default;

	// Returns true if the object file date is older than the source file or any of it's dependencies.
	bool RequiresRebuild(const std::string& pSourceFile,const std::string& pObjectFile,const tinytools::StringVec& pIncludePaths);

private:
	bool CheckSourceDependencies(const std::string& pSourceFile,const timespec& pObjFileTime,const tinytools::StringVec& pIncludePaths);
	bool GetFileTime(const std::string& pFilename,timespec& rFileTime);
	bool FileYoungerThanObjectFile(const std::string& pFilename,const timespec& pObjFileTime);
	bool FileYoungerThanObjectFile(const timespec& pOtherTime,const timespec& pObjFileTime)const;
	bool GetIncludesFromFile(const std::string& pFilename,const tinytools::StringVec& pIncludePaths,tinytools::StringSet& rIncludes);

	typedef struct stat FileStats;
	typedef std::unordered_map<std::string,timespec> FileTimeMap;
	typedef std::unordered_map<std::string,tinytools::StringSet> DependencyMap;
	typedef std::unordered_map<std::string,bool> FileState;	// True if it is out of date and thus the source file needs building, false it is not. If not found we have not checked it yet.


	DependencyMap mDependencies;
	FileTimeMap mFileTimes;
	FileState mFileDependencyState;
	FileState mFileCheckedState;
};


#endif //#ifndef __DEPENDENCIES_H__
