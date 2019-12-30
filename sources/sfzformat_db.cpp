#include "sfzformat_db.h"
#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <iostream>
#include <fstream>

SfzDb *SfzDb::loadYAML(const ghc::filesystem::path &path)
{
    SfzDb db;

    db._opcodes.reserve(1024);

    YAML::Node doc;
    try {
        doc = YAML::LoadFile(path);
    }
    catch (std::exception &ex) {
        std::cerr << ex.what() << "\n";
        return nullptr;
    }

    db.processCategories(doc["categories"]);

    std::sort(db._opcodes.begin(), db._opcodes.end());

    return new SfzDb{std::move(db)};
}

std::string SfzDb::createFatOpcodeRegexp() const
{
    std::string re;
    re.reserve(32 * 1024);

    re.append("(?:");

    bool firstOpcode = true;

    for (const std::string &opcode : _opcodes) {
        if (!firstOpcode)
            re.push_back('|');

        for (char c : opcode) {
            bool isLower = c >= 'a' && c <= 'z';
            bool isUpper = c >= 'A' && c <= 'Z';
            bool isDigit = c >= '0' && c <= '9';

            if (c == 'N' || c == 'X' || c == 'Y' || c == 'Z')
                re.append("[0-9]+");
            else if (c == '*')
                re.append("[a-zA-Z0-9_]+");
            else if (isLower || isUpper || isDigit || c == '_' || c == '#')
                re.push_back(c);
            else {
                std::cerr << "Do not know how to handle character in regexp: " << (int)(unsigned char)c << "\n";
                return std::string{};
            }
        }

        firstOpcode = false;
    }

    re.append(")");

    return re;
}

void SfzDb::processCategories(YAML::Node categoryList)
{
    if (!categoryList.IsSequence())
        return;

    size_t numCat = categoryList.size();
    for (size_t idxCat = 0; idxCat < numCat; ++idxCat) {
        YAML::Node category = categoryList[idxCat];
        processOpcodes(category["opcodes"]);
        processTypes(category["types"]);
    }
}

void SfzDb::processTypes(YAML::Node typeList)
{
    if (!typeList.IsSequence())
        return;

    size_t numTypes = typeList.size();
    for (size_t idxType = 0; idxType < numTypes; ++idxType) {
        YAML::Node type = typeList[idxType];
        processOpcodes(type["opcodes"]);
    }
}

void SfzDb::processOpcodes(YAML::Node opcodeList)
{
    if (!opcodeList.IsSequence())
        return;

    size_t numOpcodes = opcodeList.size();
    for (size_t idxOpcode = 0; idxOpcode < numOpcodes; ++idxOpcode) {
        YAML::Node opcode = opcodeList[idxOpcode];
        processOpcodeName(opcode["name"].as<std::string>());
        processOpcodeAlias(opcode["alias"]);
        processOpcodeModulation(opcode["modulation"]);
    }
}

void SfzDb::processOpcodeAlias(YAML::Node aliasList)
{
    if (!aliasList.IsSequence())
        return;

    size_t numAliases = aliasList.size();
    for (size_t idxAlias = 0; idxAlias < numAliases; ++idxAlias) {
        YAML::Node alias = aliasList[idxAlias];
        processOpcodeName(alias["name"].as<std::string>());
    }
}

void SfzDb::processOpcodeModulation(YAML::Node modulationList)
{
    if (!modulationList.IsMap())
        return;

    for (YAML::const_iterator it = modulationList.begin(), end = modulationList.end();
         it != end; ++it)
    {
        // key: midi_cc, ... (other?)
        processOpcodes(it->second);
    }
}

void SfzDb::processOpcodeName(const std::string &name)
{
    if (_opcodesUnordered.insert(name).second)
        _opcodes.emplace_back(name);
}
