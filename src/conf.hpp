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

#include <cstdio>
#include <string>
#include <string_view>
#include <map>
#include <variant>
#include <optional>
#include <functional>
#include <filesystem>
#include <system_error>
#include <expected.hpp>

namespace conf {

enum class Type { Int, Float, Bool, String };

inline std::string_view type_to_string(conf::Type t)
{
    switch (t) {
    case conf::Type::Int:    return "int";
    case conf::Type::Float:  return "float";
    case conf::Type::Bool:   return "bool";
    case conf::Type::String: return "string";
    default: return "";
    }
}

/*
 * A Value in a configuration file. Can be one of: number (integer), number
 * (floating point), boolean or string.
 */
struct Value {
    std::variant<int, float, bool, std::string> value;

    Value() = default;
    explicit Value(bool v)               : value(v) {}
    explicit Value(int v)                : value(v) {}
    explicit Value(float v)              : value(v) {}
    explicit Value(const std::string &v) : value(v) {}
    explicit Value(const char *v)        : value(std::string(v)) {}
    explicit Value(char *v)              : value(std::string(v)) {}

    template <typename T> T as() const { return std::get<T>(value); }
    Type type() const { return static_cast<Type>(value.index()); }

    std::string to_string() const
    {
        switch (value.index()) {
        case 0: return std::to_string(as<int>());
        case 1: return std::to_string(as<float>());
        case 2: return as<bool>() ? "true" : "false";
        case 3: return "\"" + as<std::string>() + "\"";
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

/*
 * An error found while parsing. If any of these is produced while parsing,
 * the result should be discarded.
 * @error: what kind of error. Can also be something external, e.g. not enough
 *         permissions to write a file, etc.
 * @line, @col: line and column into the file
 * @prev, @cur: tokens affected (might be the same)
 */
struct ParseError {
    enum Type {
        NoIdent,
        NoEqualAfterIdent,
        NoValueAfterEqual,
        NoNewlineAfterValue,
        UnterminatedString,
        UnexpectedCharacter,
    };
    std::error_condition error;
    std::ptrdiff_t line = -1, col = -1;
    std::string prev = "", cur = "";
    std::string message();
};

/* An error found while validating. Can be ignored. */
struct Warning {
    enum class Type {
        InvalidKey,
        MissingKey,
        MismatchedTypes,
    } type;
    std::string key;
    conf::Value newval, orig;
    std::string message();
};

/* Error category for ParseError. Meant to be used only for constructing the
 * @error field. */
struct ConfErrorCategory : public std::error_category {
    ~ConfErrorCategory() {}
    const char *name() const noexcept { return "conf error"; }
    std::string message(int n) const;
};

inline ConfErrorCategory errcat;

/* A type representing the data of a configuration file. */
using Data = std::map<std::string, Value>;

/* Result of parsing is either the configuration data or a list of errors. */
using ParseResult = tl::expected<Data, std::vector<ParseError>>;

/*
 * This function parses a configuration file. It returns either the data of
 * the configuration, or a list of parse errors.
 * @text: the contents of the configuration file.
 */
ParseResult parse(std::string_view text);

/*
 * Validates configuration data using @valid_data as a template.
 * The function checks for missing keys (keys that are in @valid but not in
 * @data), invalid keys (keys that are in @data but not in @valid) and
 * mismatched types. Invalid keys are deleted, everything else is replaced
 * with a default value.
 * @data: the configuration data to validate.
 * @valid_data: a template for which keys are valid and which type of values
 *              they should have. Values are taken as default values.
 */
std::vector<Warning> validate(Data &data, const Data &valid_data);

/*
 * Creates a configuration file.
 * @path: the path of the file to create.
 * @data: the configuration data to write.
 */
std::error_code create(std::filesystem::path path, const Data &data);

/*
 * Gets the directory where the configuration file should reside.
 * The configuration file's path can then be retrieved by concatenating
 * this path to the file's name.
 * The directory is always created if absent.
 * @appname: name of the application, which is used as the name of the
 * configuration directory.
 */
std::filesystem::path getdir(std::string_view appname);

/*
 * Checks if a configuration file exists and creates the configuration file,
 * filling it with default values, if it doesn't exist. Otherwise it parses
 * and validates it using the default values.
 * @appname: name of the application, which is used as the name of the
 * configuration directory and as the file name.
 * @defaults: default data to write when creating;
 */
ParseResult parse_or_create(std::string_view appname, const Data &defaults);

} // namespace conf
