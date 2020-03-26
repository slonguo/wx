#include <iostream>
#include <string>
#include <vector>
#include "apps/utils/strings.hpp"

using namespace std;

struct Item
{
    // Item(string name, string type, string stamp) : name(name), type(type), stamp(stamp) {}
    string name;
    string type;

    string stamp;
};

int main()
{
    vector<Item> elts = {
        {"alex", "14", "//1589932232432"},
        {"felix", "43", "+1589932232323"},
        {"bob", "12", "1589932232121"},
        {"minkov", "72", "1589932232129"},
        {"liapunov", "2", "1589932232333"},
    };

    for (auto &e : elts)
    {
        string src = e.type + e.stamp;
        auto enc = utils::encode(src, e.name);
        cout << enc << endl;
        string dest;
        cout << utils::decode(enc, e.name, dest) << endl;
        cout << src << endl;
        cout << dest << endl;
    }
    for (auto i = 0; i < 3; ++i)
    {
        cout << utils::randGenAlpha(16) << endl;
        cout << utils::randGenAlphaNum(16) << endl;
        cout << utils::randGenNumStr(16) << endl;
        cout << utils::randGenAlphaLowerCase(16) << endl;
        cout << utils::randGenAlphaUpperCase(16) << endl;
    }
}
