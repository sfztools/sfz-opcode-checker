#pragma once
#include <ghc/filesystem.hpp>
#include <vector>
#include <unordered_set>

typedef std::vector<std::string> OpcodeNameList;
typedef std::unordered_set<std::string> OpcodeNameSet;

bool scanFileOpcodes(const ghc::filesystem::path &path, OpcodeNameSet &set);
