#include "main.h"
#include "opcode_collecting_parser.h"
#include "sfzformat_db.h"
#include <getopt.h>
#include <unistd.h>
#include <algorithm>
#include <regex>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        displayHelp();
        return 1;
    }

    int ret;
    absl::string_view verb{argv[1]};

    if (verb == "validate")
        ret = runValidate(argc - 1, argv + 1);
    else if (verb == "print")
        ret = runPrint(argc - 1, argv + 1);
    else {
        displayHelp();
        ret = 1;
    }

    return ret;
}

void displayHelp()
{
    std::cerr <<
        "An opcode checker for SFZ instrument files\n"
        "Usage: sfz-opcode-checker <verb> [option]...\n"
        "\n"
        "# print - scan opcodes used by SFZ files and display them.\n"
        "\n"
        "  sfz-opcode-checker print <sfz-file> [other-sfz-file]...\n"
        "\n"
        "# validate - scan opcodes used by SFZ files, and check their\n"
        "             presence in the sfzformat.github.io database.\n"
        "\n"
        "  sfz-opcode-checker validate <-d syntax.yml> <sfz-file> [other-sfz-file]...\n"
        "     -d <syntax.yml> : database file syntax.yml from sfzformat.github.io\n";
}

int runValidate(int argc, char *argv[])
{
    fs::path opcodeDbPath;

    for (int c; (c = getopt(argc, argv, "d:")) != -1;) {
        switch (c) {
        case 'd':
            opcodeDbPath = optarg;
            break;
        default:
            return 1;
        }
    }

    ///
    if (argc - optind < 1) {
        std::cerr << "You didn't indicate any SFZ files.\n";
        return 1;
    }

    if (opcodeDbPath.empty()) {
        std::cerr << "Please indicate a YAML database file with option -d.\n";
        return 1;
    }

    ///
    std::unique_ptr<SfzDb> db{SfzDb::loadYAML(opcodeDbPath)};

    if (!db) {
        std::cerr << "Error loading the opcode database from YAML.\n";
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

    std::string reStr = db->createFatOpcodeRegexp();
    std::regex re("^" + reStr + "$", std::regex::optimize);

    bool success = true;

    for (const std::string &opcode : opcodeList) {
        bool match = std::regex_match(opcode, re);
        if (!match)
            success = false;

#if 0 // it was complained that not all terminals will show these
        absl::string_view txtGood = u8"\u2705";
        absl::string_view txtFail = u8"\u274C";
#else
        absl::string_view txtGood = "[ OK ]";
        absl::string_view txtFail = "[FAIL]";
        if (isatty(1)) {
            txtGood = "\033[32m" "[ OK ]" "\033[0m";
            txtFail = "\033[31m" "[FAIL]" "\033[0m";
        }
#endif

        std::cout << (match ? txtGood : txtFail) << " " << opcode << "\n";
    }

    if (!success)
        return 1;

    return 0;
}

int runPrint(int argc, char *argv[])
{
    for (int c; (c = getopt(argc, argv, "")) != -1;) {
        switch (c) {
        default:
            return 1;
        }
    }

    ///
    if (argc - optind < 1) {
        std::cerr << "You didn't indicate any SFZ files.\n";
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
    for (const std::string &opcode : opcodeList)
        std::cout << opcode << "\n";

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
