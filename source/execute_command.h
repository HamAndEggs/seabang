#ifndef EXECUTE_COMMAND_H__
#define EXECUTE_COMMAND_H__

#include <string>
#include <vector>
#include <map>
#include <filesystem>

bool ExecuteShellCommand(const std::filesystem::path& pCommand,const std::vector<std::string>& pArgs,const std::map<std::string,std::string>& pEnv, std::string& rOutput);
inline bool ExecuteShellCommand(const std::filesystem::path& pCommand,const std::vector<std::string>& pArgs, std::string& rOutput)
{
    const std::map<std::string,std::string> empty;
    return ExecuteShellCommand(pCommand,pArgs,empty,rOutput);
}
void ExecuteCommand(const std::filesystem::path& pCommand,const std::vector<std::string>& pArgs,const std::map<std::string,std::string>& pEnv);

#endif //#ifndef EXECUTE_COMMAND_H__
