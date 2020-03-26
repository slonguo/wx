#ifndef UTILS_CRYPTO_HPP
#define UTILS_CRYPTO_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <set>
#include <map>



namespace utils
{

using std::map;
using std::set;
using std::string;
using std::vector;

string encode(string &str, string &key);
bool decode(const string &str, string &key, string &dest);
void pad2right(string &str, const size_t num, const char c = ' ');
void pad2left(string &str, const size_t num, const char c = ' ');
void rtrim(string &str, const char c = ' ');
void ltrim(string &str, const char c = ' ');
void trim(string &str, const char c = ' ');

vector<string> splitByWhitespace(const string &input);

struct SubCmdItem
{
    string subCmd;
    string desc;

    SubCmdItem(string sub = "", string desc = "") : subCmd(sub), desc(desc) {}
    bool operator<(const SubCmdItem &rhs) const
    {
        return subCmd < rhs.subCmd || (subCmd == rhs.subCmd && desc < rhs.desc);
    }
};

using CmdMap = map<string, set<SubCmdItem>>;

void help(map<string, set<SubCmdItem>> &cmds, string command = "");
auto isWord = [](string &cmd) { return all_of(cmd.begin(), cmd.end(), [](char c) { return isalpha(c); }); };
auto isAlphaNum = [](string &str) { return all_of(str.begin(), str.end(), [](char c) { return isalnum(c); }); };
auto toLower = [](string &cmd) {for_each(cmd.begin(), cmd.end(), [](char &c) { c = tolower(c); });return cmd; };
auto toUpper = [](string &cmd) {for_each(cmd.begin(), cmd.end(), [](char &c) { c = toupper(c); });return cmd; };
auto in = [](const string &s, vector<string> &vec) -> bool { return std::any_of(vec.begin(), vec.end(), [&s](string &e) { return s == e; }); };

string randGenAlpha(size_t len);
string randGenAlphaNum(size_t len);
string randGenNumStr(size_t len);
string randGenAlphaLowerCase(size_t len);
string randGenAlphaUpperCase(size_t len);

string randGen(const string &src, size_t len);
int randInt(int start, int stop);

} // namespace utils

#endif
