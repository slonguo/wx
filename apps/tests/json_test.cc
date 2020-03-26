#include <iostream>
#include <vector>
#include <string>

#include "apps/3rd/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace ns
{
// a simple struct to model a person
struct person
{
    std::string name;
    std::string address;
    int age;
};

void to_json(json &j, const person &p)
{
    j = json{{"name", p.name}, {"address", p.address}, {"age", p.age}};
}

void from_json(const json &j, person &p)
{
    j.at("name").get_to(p.name);
    j.at("address").get_to(p.address);
    j.at("age").get_to(p.age);
}
} // namespace ns

void testSTL()
{
    // create an array using push_back
    json j;
    j.push_back("foo");
    j.push_back(1);
    j.push_back(true);

    // also use emplace_back
    j.emplace_back(1.78);

    // iterate the array
    for (json::iterator it = j.begin(); it != j.end(); ++it)
    {
        std::cout << *it << '\n';
    }

    // range-based for
    for (auto &element : j)
    {
        std::cout << element << '\n';
    }

    // getter/setter
    const auto tmp = j[0].get<std::string>();
    j[1] = 42;
    bool foo = j.at(2);
    std::cout << foo << std::endl;

    // comparison
    bool cmp = j == "[\"foo\", 42, true]"_json; // true
    std::cout << cmp << std::endl;

    // other stuff
    j.size();  // 3 entries
    j.empty(); // false
    j.type();  // json::value_t::array
    j.clear(); // the array is empty again

    // convenience type checkers
    j.is_null();
    j.is_boolean();
    j.is_number();
    j.is_object();
    j.is_array();
    j.is_string();

    // create an object
    json o;
    o["foo"] = 23;
    o["bar"] = false;
    o["baz"] = 3.141;

    // also use emplace
    o.emplace("weather", "sunny");

    // special iterator member functions for objects
    for (json::iterator it = o.begin(); it != o.end(); ++it)
    {
        std::cout << it.key() << " : " << it.value() << "\n";
    }

    // the same code as range for
    for (auto &el : o.items())
    {
        std::cout << el.key() << " : " << el.value() << "\n";
    }

    // even easier with structured bindings (C++17)
    // for (auto& [key, value] : o.items()) {
    // std::cout << key << " : " << value << "\n";
    // }

    // find an entry
    if (o.find("foo") != o.end())
    {
        // there is an entry with key "foo"
    }

    // or simpler using count()
    int foo_present = o.count("foo"); // 1
    int fob_present = o.count("fob"); // 0
    std::cout << foo_present << std::endl;
    std::cout << fob_present << std::endl;

    // delete an entry
    o.erase("foo");
}

void testMisc()
{
    vector<string> elts = {
        "{\"kick_mode\":1}",
        // "{\"kick_mode\":\"1\"}",
        // "{\"kick_mode\":1",
        "{\"kick_mode1\":1}",

    };
    for (auto s : elts)
    {
        json j = json::parse(s);
        cout << j["kick_mode"].get<int>() << endl;
    }
}

void testObjectify()
{
    // create a person
    ns::person p{"Ned Flanders", "744 Evergreen Terrace", 60};

    // conversion: person -> json
    json j = p;

    cout << j << endl;
    // {"address":"744 Evergreen Terrace","age":60,"name":"Ned Flanders"}

    // conversion: json -> person
    auto p2 = j.get<ns::person>();

    // that's it
    cout << p2.name << endl;
}

int main()
{
    testSTL();
    testMisc();
    testObjectify();
}