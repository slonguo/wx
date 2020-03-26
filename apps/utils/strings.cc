#include <iostream>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <random>

#include "strings.hpp"
#include "apps/3rd/bin2ascii.hpp"
#include "apps/3rd/md5.hpp"

namespace utils
{

using std::back_inserter;
using std::copy;
using std::cout;
using std::endl;
using std::istream_iterator;
using std::istringstream;

using joyee::md5;

static const size_t kPadSize = 16;
static const size_t kMd5Size = 32;

string encode(string &str, string &key)
{
    pad2left(str, kPadSize, ' ');
    pad2right(key, kPadSize, ' ');
    return md5(str + key) + bin2hex(string(str.rbegin(), str.rend()));
}

bool decode(const string &str, string &key, string &dest)
{
    if (str.size() < kMd5Size || str.size() % 2)
    {
        return false;
    }
    try
    {
        string md5Str = str.substr(0, kMd5Size);
        dest = hex2bin(str.substr(kMd5Size, str.size() - kMd5Size));
        std::reverse(dest.begin(), dest.end());
        pad2right(key, kPadSize, ' ');
        if (md5(dest + key) != md5Str)
            return false;
        rtrim(dest, ' ');
        return true;
    }
    catch (...)
    {
        return false;
    }

    return false;
}

void pad2right(string &str, const size_t num, const char paddingChar)
{
    if (num <= str.size())
        return;
    str.insert(str.begin(), num - str.size(), paddingChar);
}

void pad2left(string &str, const size_t num, const char paddingChar)
{
    if (num <= str.size())
        return;
    str.insert(str.end(), num - str.size(), paddingChar);
}

void rtrim(string &str, const char c)
{
    str.erase(str.find_last_not_of(c) + 1);
}

void ltrim(string &str, const char c)
{
    str.erase(0, str.find_first_not_of(c));
}

void trim(string &str, const char c)
{
    ltrim(str, c);
    rtrim(str, c);
}

vector<string> splitByWhitespace(const string &input)
{
    istringstream buffer(input);
    vector<string> ret;

    copy(istream_iterator<string>(buffer), istream_iterator<string>(), back_inserter(ret));
    return ret;
}

void help(map<string, set<SubCmdItem>> &cmds, string command)
{
    // width:80
    // add `_` to `caption` to force alignment with splitter.(talking about paranoid)
    string splitter = "--------------------------------------------------------------------------------\n";
    string caption_ = "| command |   sub command/data  |                   descriptions               |\n";

    cout << splitter << caption_ << splitter;

    auto outputCmd = [](string cmd, set<SubCmdItem> &v) {
        // string cmd(k);

        if (v.size() > 1)
        {
            pad2left(cmd, 78);
            cout << "|" << cmd << "|" << endl;
            for (auto &e : v)
            {
                auto subCmd = e.subCmd, desc = e.desc;
                pad2left(subCmd, 20);
                pad2left(desc, 46);
                cout << "|          " << subCmd << "  " << desc << "|" << endl;
            }
        }
        else
        {
            for (auto &e : v)
            {
                pad2left(cmd, 10);
                auto subCmd = e.subCmd, desc = e.desc;
                pad2left(subCmd, 20);
                pad2left(desc, 46);
                cout << "|" << cmd << subCmd << "  " << desc << "|" << endl;
            }
        }
    };

    if (cmds.find(command) == cmds.end())
    {
        for (auto &[k, v] : cmds)
        {
            outputCmd(k, v);
            cout << splitter;
        }
    }
    else
    {
        outputCmd(command, cmds[command]);
        cout << splitter;
    }
}

static const string kAlphaNums = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const string kAlphas = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const string kAalphaUppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const string kAlphaLowers = "abcdefghijklmnopqrstuvwxyz";
static const string kNums = "0123456789";

// https://stackoverflow.com/questions/47977829/generate-a-random-string-in-c11
string randGen(const string &src, size_t len)
{
    if (src.empty())
        return "";
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> dist(0, src.size() - 1);
    string ret = "";
    for (int i = 0; i < len; i++)
    {
        int random_index = dist(engine);
        ret += src[random_index];
    }
    return ret;
}

int randInt(int start, int stop)
{
    if (start >= stop)
        return 0;
    std::random_device rd;                              // obtain a random number from hardware
    std::mt19937 engine(rd());                          // seed the generator
    std::uniform_int_distribution<> distr(start, stop); // define the range
    return distr(engine);
}

string randGenAlpha(size_t len) { return randGen(kAlphas, len); }
string randGenAlphaNum(size_t len) { return randGen(kAlphaNums, len); }
string randGenNumStr(size_t len) { return randGen(kNums, len); }
string randGenAlphaLowerCase(size_t len) { return randGen(kAlphaLowers, len); }
string randGenAlphaUpperCase(size_t len) { return randGen(kAalphaUppers, len); }

} // namespace utils