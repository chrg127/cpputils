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

/*
 * A Value in a configuration file. Can be one of: number (integer), number
 * (floating point), boolean or string.
 */
enum class Type { Int, Float, Bool, String };

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
    std::size_t line, col;
    std::string prev, cur;
};

/* A type representing the data of a configuration file. */
using Data = std::map<std::string, Value>;

/* A type used for error callbacks */
using DisplayCallback = std::function<void(std::string_view)>;

using ParseResult = tl::expected<Data, std::vector<ParseError>>;

/*
 * A type describing which combination of keys and values are valid for a
 * configuration file. The parser uses this information to discard invalid keys
 * or keys with invalid value types.
 * For example:
 *
 * const ValidConfig valid_conf = {
 *     { "logfile", conf::Value("error.log") },
 *     { "width",   conf::Value(0) },
 *     { "height",  conf::Value(1.0f) }
 * };
 *
 * The values of the Value's describe the default value for the key: if, during
 * the parsing process, a key is found, but its value is invalid, then the
 * default specified will be selected.
 */
using ValidConfig = std::map<std::string, Value>;

/*
 * This function parses a configuration file.
 * @text: the contents of the configuration file.
 * @valid: describes which are valid keys and value types, as well the default
 *         values for each key.
 * @error: a callback function that display an error to the user. It
 *                 takes as a parameter the error message.
 * It returns the data of the configuration, or std::nullopt on parse error.
 */
std::optional<Data> parse(
    std::string_view text,
    DisplayCallback error
);

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
    DisplayCallback warning
);

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
std::optional<Data> parse_or_create(
    std::filesystem::path path,
    const Data &defaults,
    DisplayCallback error
);

/*
 * Tries to search for the configuration file on the file system.
 * Returns the path of the filesystem, or std::nullopt if not found.
 */
std::optional<std::filesystem::path> find_file(std::string_view name);

} // namespace conf
