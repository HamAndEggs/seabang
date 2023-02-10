#ifndef EXECUTE_COMMAND_H__
#define EXECUTE_COMMAND_H__

#include <string>
#include <vector>
#include <map>
#include <filesystem>

bool ExecuteShellCommand(const std::filesystem::path& pCommand,const std::vector<std::string>& pArgs, std::string& rOutput);

#endif //#ifndef EXECUTE_COMMAND_H__
