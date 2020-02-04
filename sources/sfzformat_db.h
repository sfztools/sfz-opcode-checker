#pragma once
#include <ghc/filesystem.hpp>
#include <string>
#include <vector>
#include <unordered_set>

namespace YAML { class Node; };

class SfzDb {
public:
    static SfzDb *loadYAML(const ghc::filesystem::path &path);

    const std::vector<std::string> &getOpcodes() { return _opcodes; }
    std::string createFatOpcodeRegexp() const;

private:
    void processCategories(YAML::Node categoryList);
    void processTypes(YAML::Node typeList);
    void processOpcodes(YAML::Node opcodeList);
    void processOpcodeAlias(YAML::Node aliasList);
    void processOpcodeModulation(YAML::Node modulationList);
    void processOpcodeName(const std::string &name);

private:
    std::vector<std::string> _opcodes;
    std::unordered_set<std::string> _opcodesUnordered;
};
