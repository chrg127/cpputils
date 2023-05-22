/*
 * This is a library for configuration files.
 * The configuration file defined by this library is a list of key-value pairs,
 * in which the key is a string and the value can be either a number (integer
 * or float), a string or a boolean.
 * The library implements function for reading and write these kind of files.
 *
 * This library depends on io.hpp and string.hpp.
 */

#pragma once

#include <string>
#include <string_view>
#include <map>
#include <variant>
#include <filesystem>
#include <system_error>
#include <vector>
#include <expected.hpp>

namespace conf {

enum class Type { Int, Float, Bool, String, List };

inline std::string type_to_string(conf::Type t)
{
    switch (t) {
    case conf::Type::Int:    return "int";
    case conf::Type::Float:  return "float";
    case conf::Type::Bool:   return "bool";
    case conf::Type::String: return "string";
    case conf::Type::List:   return "list";
    default: return "";
    }
}

/*
 * A Value in a configuration file. Can be one of: number (integer), number
 * (floating point), boolean or string.
 */
struct Value;
using ValueList = std::vector<Value>;

struct Value {
    std::variant<int, float, bool, std::string, ValueList> value;

    Value() = default;
    explicit Value(bool v)               : value(v) {}
    explicit Value(int v)                : value(v) {}
    explicit Value(float v)              : value(v) {}
    explicit Value(const std::string &v) : value(v) {}
    explicit Value(const char *v)        : value(std::string(v)) {}
    explicit Value(char *v)              : value(std::string(v)) {}
    explicit Value(ValueList &&vs) : value(std::move(vs)) {}

    template <typename T> T as() const { return std::get<T>(value); }
    Type type() const { return static_cast<Type>(value.index()); }

    std::string to_string() const
    {
        switch (value.index()) {
        case 0: return std::to_string(as<int>());
        case 1: return std::to_string(as<float>());
        case 2: return as<bool>() ? "true" : "false";
        case 3: return "\"" + as<std::string>() + "\"";
        case 4: {
            auto l = as<ValueList>();
            std::string s = "[";
            if (l.size() > 0)
                s += l[0].to_string();
            for (auto i = 1u; i < l.size(); i++)
                s += ", " + l[i].to_string();
            s += "]";
            return s;
        }
        default: return "";
        }
    }
};

template <typename T>
inline bool operator==(const Value &v, const T &t)
    requires std::is_same_v<T, int>  || std::is_same_v<T, float>
          || std::is_same_v<T, bool> || std::is_same_v<T, std::string>
{
    const auto *p = std::get_if<T>(&v.value);
    return p != nullptr && *p == t;
}

namespace literals {

inline Value operator"" _v(unsigned long long x)  { return conf::Value{int(x)}; }
inline Value operator"" _v(long double x)         { return conf::Value{float(x)}; }
inline Value operator"" _v(const char *x, std::size_t n) { return conf::Value{std::string(x, n)}; }

} // namespace literals

/*
 * An error found while parsing.
 * @type: what kind of error.
 * @prev, @cur: tokens affected
 * @line, @col: line and column into the file
 * @key, @value, @def: affected key, along with value found and
 * default value, if there are any.
 * @external_error: if this error instance is from an external
 * source, instead of the parsing result, only this field will
 * be filled.
 */
struct Error {
    enum Type {
        NoIdent,
        NoEqualAfterIdent,
        NoValueAfterEqual,
        NoNewlineAfterValue,
        UnterminatedString,
        UnexpectedCharacter,
        ExpectedRightSquare,
        InvalidKey,
        MissingKey,
        MismatchedTypes,
        External,
    };
    Type type = {};
    std::string prev = "", cur = "";
    std::ptrdiff_t line = -1, col = -1;
    std::string key = {};
    conf::Value value = {}, def = {};
    std::error_code external_error = {};
    std::string message();
};

/* A type representing the data of a configuration file. */
using Data = std::map<std::string, Value>;
using ParseResult = std::pair<Data, std::vector<Error>>;

/*
 * Flags used by parse to alter some of its inner workings.
 * @AcceptAnyKey: This flags will make parse() accept any key that
 * is not in the default values passed.
 */
namespace flags {

enum Flags {
    None = 0x0,
    AcceptAnyKey = 0x1,
};

} // namespace flags

/*
 * This function parses a configuration file. It returns the data of
 * the configuration and a list of parse errors.
 * @text: the contents of the configuration file.
 * @defaults: default keys and values. Also used to type check values
 * and find missing or invalid keys.
 * @flags: see above.
 */
ParseResult parse(std::string_view text, const Data &defaults,
    flags::Flags flags = flags::None);

/*
 * Writes or creates a configuration file. The second function finds the right
 * path for you.
 * @path: the path of the file to write.
 * @data: the configuration data to write.
 */
std::error_code write_to(std::filesystem::path path, const Data &data);
std::error_code write(std::string_view appname, const Data &data);

/*
 * Gets the directory where the configuration file should reside.
 * The configuration file's path can then be retrieved by concatenating
 * this path to the file's name.
 * The directory is always created if absent. An error code is returned
 * if failing to create a directory.
 * @appname: name of the application, which is used as the name of the
 * configuration directory.
 */
tl::expected<std::filesystem::path, std::error_code>
getdir(std::string_view appname);

/*
 * Checks if a configuration file exists and, if it doesn't, creates it and
 * fills it with default values. Otherwise the function parses it.
 * @appname: name of the application, which is used as the name of the
 * configuration directory and as the file name.
 * @defaults: default data to write when creating;
 * @flags: see above.
 */
ParseResult parse_or_create(std::string_view appname, const Data &defaults,
    flags::Flags flags = flags::None);

} // namespace conf
