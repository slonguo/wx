#ifndef UTILS_CONN_OPTIONS_JSONIFY_HPP
#define UTILS_CONN_OPTIONS_JSONIFY_HPP

#include "apps/3rd/json.hpp"
#include "apps/3rd/sql/mysqlplus.h"

// for easy serialization/deserialization
// https://github.com/nlohmann/json#basic-usage
namespace daotk
{
namespace mysql
{
using daotk::mysql::connect_options;
using nlohmann::json;

void to_json(json &j, const connect_options &co);
void from_json(const json &j, connect_options &co);
} // namespace mysql
} // namespace daotk

namespace utils
{
using daotk::mysql::connect_options;
using nlohmann::json;

struct DBConfig
{
    connect_options conn;
    size_t pool_size;
};

void to_json(json &j, const DBConfig &dc);
void from_json(const json &j, DBConfig &dc);
} // namespace utils

#endif