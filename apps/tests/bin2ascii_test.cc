#include <iostream>
#include <string>
#include <vector>

#include "apps/3rd/bin2ascii.hpp"

using namespace std;


int main()
{
    vector<string> elts = {
        "",
        "hello",
        "world",
        "111",
        "15723456734"
    };

    for(auto & e : elts)
    {
        cout << e << endl;
        cout << b64_encode(e) << endl;
        cout << b64_decode(b64_encode(e)) << endl;
        cout << bin2hex(e) << endl;
        cout << hex2bin(bin2hex(e)) << endl;
        cout << endl;
    }
}