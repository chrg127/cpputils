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

/* Types of erros found by the parser. */
enum class ErrorType {
    NoIdent,
    NoEqualAfterIdent,
    NoValueAfterEqual,
    NoNewlineAfterValue,
    UnterminatedString,
    UnexpectedCharacter,
};

/* For constructing std::error_conditions. */
struct ConfErrorCategory : public std::error_category {
    ~ConfErrorCategory() {}
    const char *name() const noexcept { return "conf error"; }
    std::string message(int n) const;
};

inline ConfErrorCategory conf_error_category;

/*
 * A parse error:
 * @line, @col: line and column into the file
 * @prev, @cur: tokens affected (might be the same)
 * @error: what kind of error
 */
struct ParseError {
    std::error_condition error;
    int line, col;
    std::string prev, cur;
    static ParseError make(std::error_code e) {
        return { .error = e.default_error_condition(),
                 .line = -1, .col = -1, .prev = "", .cur = "" };
    }
};

/* A type representing the data of a configuration file. */
using Data = std::map<std::string, Value>;

/* Result of parsing is either the configuration data or a list of errors. */
using ParseResult = tl::expected<Data, std::vector<ParseError>>;

/*
 * This function parses a configuration file.
 * @text: the contents of the configuration file.
 * @valid: describes which are valid keys and value types, as well the default
 *         values for each key.
 * @error: a callback function that display an error to the user. It
 *                 takes as a parameter the error message.
 * It returns the data of the configuration, or std::nullopt on parse error.
 */
ParseResult parse(std::string_view text);

/*
 * Creates a configuration file.
 * @path: the path of the file to create.
 * @conf: the configuration data to write.
 * @error: a callback function that should display an error. It takes as a
 *           parameter the error message.
 */
std::error_code create(std::filesystem::path path, const Data &conf);

/*
 * Checks if a configuration file exists and:
 *
 *     - Parses the file if it exists;
 *     - Create the file with default values if it doesn't.
 *
 * @path: the path of the file to parse/create;
 * @defaults: default data to write when creating;
 * @error: a callback function that should display an error. It takes as a
 *           parameter the error message.
 */
ParseResult parse_or_create(std::filesystem::path path, const Data &defaults);

/*
 * Validates configuration data using @valid as a template.
 * The function checks for missing keys (keys that are in @valid but not in
 * @data), invalid keys (keys that are in @data but not in @valid) and
 * mismatched types. Invalid keys are deleted, everything else is replaced
 * with a default value.
 * @data: the configuration data to validate.
 * @valid: a template for which keys are valid and which type of values they
 *         should have. Values are taken as default values.
 * @warning: a callback function that should display an error. It takes as a
 *           parameter the warning message.
 */
Data validate(
    Data conf,
    const Data &valid_conf,
    std::function<void(std::string_view)> warning
);

/*
 * Tries to search for the configuration file on the file system.
 * Returns the path of the filesystem, or std::nullopt if not found.
 */
std::optional<std::filesystem::path> find_file(std::string_view name);

} // namespace conf
