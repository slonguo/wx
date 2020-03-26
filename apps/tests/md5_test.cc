#include <iostream>
#include <iomanip>
#include <string>
#include <utility>
#include <vector>

#include "apps/3rd/md5.hpp"

using namespace std;
using namespace joyee;

int main()
{
    vector<pair<string, string>> elts = {
        make_pair("", "d41d8cd98f00b204e9800998ecf8427e"),
        make_pair("123456", "e10adc3949ba59abbe56e057f20f883e"),
        make_pair("1", "c4ca4238a0b923820dcc509a6f75849b"),
        make_pair("ffffffff", "3028879ab8d5c87dc023049fa5bb5c1a"),
        make_pair("qwertyuiop", "6eea9b7ef19179a06954edd0f6c05ceb"),
    };

    for (auto & e : elts) {
        string str = joyee::md5(e.first);
        cout << setw(32) << e.first << "\t" << str << "\t" << e.second << "\t" << (str == e.second) << endl;
    }
}
