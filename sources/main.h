#pragma once
#ifndef _WIN32
#include <curl/curl.h>
#else
#include <windows.h>
#include <wininet.h>
#endif
#include <ghc/filesystem.hpp>
#include <vector>
#include <memory>
#include <unordered_set>

typedef std::vector<std::string> OpcodeNameList;
typedef std::unordered_set<std::string> OpcodeNameSet;

void displayHelp();
int runValidate(int argc, char *argv[]);
int runPrint(int argc, char *argv[]);
int runUpdate(int argc, char *argv[]);
int runList(int argc, char *argv[]);
bool performUpdate();
bool scanFileOpcodes(const ghc::filesystem::path &path, OpcodeNameSet &set);
const ghc::filesystem::path &getDbCacheDir();
bool downloadNewDb(const ghc::filesystem::path &path);

#ifndef _WIN32
struct CURL_delete { void operator()(CURL *x) const noexcept { curl_easy_cleanup(x); } };
typedef std::unique_ptr<CURL, CURL_delete> CURL_u;
#else
struct HINTERNET_delete { void operator()(HINTERNET x) const noexcept { InternetCloseHandle(x); } };
typedef std::unique_ptr<std::remove_pointer<HINTERNET>::type, HINTERNET_delete> HINTERNET_u;
#endif

#define SFZ_DB_HOST "raw.githubusercontent.com"
#define SFZ_DB_PATH "/sfzformat/sfzformat.github.io/source/_data/sfz/syntax.yml"
#define SFZ_DB_URL "https://" SFZ_DB_HOST SFZ_DB_PATH
