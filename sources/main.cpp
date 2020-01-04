#include "main.h"
#include "opcode_collecting_parser.h"
#include "sfzformat_db.h"
#include <getopt.h>
#include <unistd.h>
#include <algorithm>
#include <regex>
#include <iostream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

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
    else if (verb == "update")
        ret = runUpdate(argc - 1, argv + 1);
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
        "Usage: " "\033[1m" "sfz-opcode-checker" "\033[0m" " <verb> [option]...\n"
        "\n"
        "# " "\033[1m" "print" "\033[0m" " - scan opcodes used by SFZ files and display them.\n"
        "\n"
        "  sfz-opcode-checker print <sfz-file> [other-sfz-file]...\n"
        "\n"
        "# " "\033[1m" "validate" "\033[0m" " - scan opcodes used by SFZ files, and check their\n"
        "             presence in the sfzformat.github.io database.\n"
        "\n"
        "  sfz-opcode-checker validate [-d syntax.yml] <sfz-file> [other-sfz-file]...\n"
        "     -d <syntax.yml> : database file syntax.yml from sfzformat.github.io\n"
        "\n"
        "# " "\033[1m" "update" "\033[0m" " - update the database file syntax.yml from sfzformat.github.io\n"
        "\n"
        "  sfz-opcode-checker update\n"
        ;
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

    std::unique_ptr<SfzDb> db;

    if (!opcodeDbPath.empty())
        db.reset(SfzDb::loadYAML(opcodeDbPath));
    else {
        fs::path dbPath = getDbCacheDir() / "syntax.yml";
        db.reset(SfzDb::loadYAML(dbPath));
        if (!db) {
            if (!performUpdate())
                return 1;
            db.reset(SfzDb::loadYAML(dbPath));
        }
    }

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

int runUpdate(int argc, char *argv[])
{
    for (int c; (c = getopt(argc, argv, "")) != -1;) {
        switch (c) {
        default:
            return 1;
        }
    }

    ///
    if (argc - optind > 0) {
        std::cerr << "This command does not accept positional arguments.\n";
        return 1;
    }

    if (!performUpdate())
        return 1;

    return 0;
}

bool performUpdate()
{
    fs::path dbPath = getDbCacheDir() / "syntax.yml";

    std::cerr << "\033[33;1m" "=== Downloading new sfz format database" "\033[0m" "\n";

    if (!downloadNewDb(dbPath)) {
        std::cerr << "Failed to download the sfz format database\n";
        return false;
    }
    else {
        std::cerr << "\033[32;1m" "=== Download OK : " << dbPath.string().c_str() << "\033[0m" "\n";
    }

    return true;
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

const fs::path &getDbCacheDir()
{
    static const fs::path path = []() {
#ifdef _WIN32
        std::wstring path;

        WCHAR buffer[MAX_PATH + 1] = {};
        if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, buffer) != S_OK)
            throw std::runtime_error("Cannot get LocalAppData directory.");
        path.append(buffer);

        path.append(L"\\SFZ Tools");
        CreateDirectoryW(path.c_str(), nullptr);
        path.append(L"\\Opcode Checker");
        CreateDirectoryW(path.c_str(), nullptr);
        path.push_back(L'\\');

        return fs::path{path};
#else
        std::string path;

        if (const char *env = getenv("XDG_CACHE_HOME")) {
            path.assign(env);
            path.push_back('/');
        }
        else if (const char *env = getenv("HOME")) {
            path.assign(env);
            path.append("/.cache/");
        }
        else
            throw std::runtime_error("Cannot get XDG cache directory.");

        mkdir(path.c_str(), 0755);
        path.append("SFZ Tools/");
        mkdir(path.c_str(), 0755);
        path.append("Opcode Checker/");
        mkdir(path.c_str(), 0755);

        return fs::path{path};
#endif
    }();
    return path;
}

bool downloadNewDb(const fs::path &path)
{
    fs::path tempPath = path;
    tempPath.concat(".part");

    fs::ofstream os(tempPath, std::ios::binary);
    if (os.bad())
        return false;

#ifndef _WIN32
    CURL_u curl{curl_easy_init()};
    if (!curl)
        return false;

    curl_easy_setopt(curl.get(), CURLOPT_URL, SFZ_DB_URL);
    curl_easy_setopt(
        curl.get(), CURLOPT_WRITEFUNCTION,
        +[](void *ptr, size_t size, size_t nmemb, void *stream)
        {
            ((fs::ofstream *)stream)->write((const char *)ptr, size * nmemb);
            return nmemb;
        });
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &os);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "Opcode Checker");

    long status = 0;
    if (curl_easy_perform(curl.get()) != CURLE_OK ||
        curl_easy_getinfo(curl.get(), CURLINFO_HTTP_CODE, &status) != CURLE_OK ||
        status != 200)
    {
        return false;
    }
#else
    HINTERNET_u hNet{InternetOpenA("Opcode Checker", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0)};
    if (!hNet)
        return false;
    HINTERNET_u hConn{InternetConnectA(hNet.get(), SFZ_DB_HOST, INTERNET_DEFAULT_HTTPS_PORT, "", "", INTERNET_SERVICE_HTTP, 0, 0)};
    if (!hConn)
        return false;
    HINTERNET_u hReq{HttpOpenRequestA(hConn.get(), "GET", SFZ_DB_PATH, HTTP_VERSION, "", nullptr, INTERNET_FLAG_SECURE, 0)};
    if (!hReq)
        return false;
    if (!HttpSendRequestA(hReq.get(), nullptr, 0, nullptr, 0))
        return false;

    char buffer[8192];
    DWORD count = 0;
    do {
        if (!InternetReadFile(hReq.get(), buffer, sizeof(buffer), &count))
            return false;
        os.write(buffer, count);
    } while (count > 0);
#endif

    os.flush();
    if (os.bad())
        return false;
    os.close();

    fs::remove(path);
    fs::rename(tempPath, path);

    return true;
}
