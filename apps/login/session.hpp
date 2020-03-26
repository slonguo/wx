#ifndef LOGIN_SESSION_HPP
#define LOGIN_SESSION_HPP

#include <iostream>
#include <string>

#include "apps/utils/strings.hpp"
#include "apps/utils/time.hpp"

namespace login
{
using std::string;
using std::to_string;

class ISession
{
public:
    ISession(){};
    virtual ~ISession(){};

    virtual string genSession(string str, string key) = 0;
    virtual bool decodeSession(const string &str, string key, string &result) = 0;
};

class BasicSession : public ISession
{
public:
    static BasicSession &Instance()
    {
        static BasicSession bs;
        return bs;
    }
    BasicSession(const BasicSession &) = delete;
    BasicSession &operator=(BasicSession &) = delete;

private:
    BasicSession() {}

public:
    string genToken(string name, int sysType, uint64_t stampMillisec = 0)
    {
        if (stampMillisec == 0)
        {
            stampMillisec = utils::NowMilliSec();
        }
        string src = to_string(sysType) + to_string(stampMillisec);
        return genSession(src, name);
    }

    string genSession(string str, string key)
    {
        return utils::encode(str, key);
    }

    bool decodeSession(const string &str, string key, string &result)
    {
        return utils::decode(str, key, result);
    }
};
} // namespace login

#endif