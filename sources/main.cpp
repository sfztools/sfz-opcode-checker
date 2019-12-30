#include "main.h"
#include "opcode_collecting_parser.h"
#include "sfzformat_db.h"
#include <getopt.h>
#include <algorithm>
#include <regex>
#include <iostream>

int main(int argc, char *argv[])
{
    bool printOpcodeList = false;
    bool validateOpcodeList = false;
    fs::path opcodeDbPath;

    for (int c; (c = getopt(argc, argv, "pd:")) != -1;) {
        switch (c) {
        case 'p':
            printOpcodeList = true;
            break;
        case 'd':
            validateOpcodeList = true;
            opcodeDbPath = optarg;
            break;
        default:
            return 1;
        }
    }

    if (argc - optind < 1) {
        std::cerr << "Please indicate at least one SFZ file to process.\n";
        return 1;
    }

    ///
    OpcodeNameSet opcodesUnordered;
    for (int i = optind; i < argc; ++i) {
        fs::path sfzFilePath{argv[i]};
        if (!scanFileOpcodes(sfzFilePath, opcodesUnordered))
            return 1;
    }

    ///
    OpcodeNameList opcodeList{opcodesUnordered.begin(), opcodesUnordered.end()};
    std::sort(opcodeList.begin(), opcodeList.end());

    ///
    if (printOpcodeList) {
        for (const std::string &opcode : opcodeList)
            std::cout << opcode << "\n";
        return 0;
    }

    ///
    if (validateOpcodeList) {
        std::unique_ptr<SfzDb> db{SfzDb::loadYAML(opcodeDbPath)};

        if (!db) {
            std::cerr << "Error loading the opcode database from YAML.\n";
            return 1;
        }

        std::string reStr = db->createFatOpcodeRegexp();
        std::regex re("^" + reStr + "$", std::regex::optimize);

        bool success = true;

        for (const std::string &opcode : opcodeList) {
            bool match = std::regex_match(opcode, re);
            if (!match)
                success = false;
            std::cout << (match ? u8"\u2705" : u8"\u274C") << " " << opcode << "\n";
        }

        if (!success)
            return 1;
    }

    return 0;
}

bool scanFileOpcodes(const ghc::filesystem::path &path, OpcodeNameSet &set)
{
    OpcodeCollectingParser<OpcodeNameSet> sfzParser{set};

    if (!sfzParser.loadSfzFile(path)) {
        std::cerr << "The SFZ file has failed to parse.\n";
        return false;
    }

    return true;
}
