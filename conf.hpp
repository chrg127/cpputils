/*
 * This is a library for configuration files.
 * The configuration file defined by this library is a list of key-value pairs,
 * in which the key is a string and the value can be either a number (integer
 * or float), a string or a boolean.
 * The library implements function for reading and write these kind of files.
 *
 * It is worth noting that this library depends on io.hpp and string.hpp.
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
        case 1: return std::to_string(as<float>()) + "f";
        case 2: return as<bool>() ? "true" : "false";
        case 3: return "\"" + as<std::string>() + "\"";
        default: return "";
        }
    }
};

/* A type representing the data of a configuration file. */
using Data        = std::map<std::string, Value>;

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
 * @display_error: a callback function that display an error to the user. It
 *                 takes a single string parameter, which is the message to
 *                 print.
 * It returns the data of the configuration, or std::nullopt on parse error.
 */
std::optional<Data> parse(
    std::string_view text,
    const ValidConfig &valid,
    std::function<void(std::string_view)> display_error
);

/*
 * Creates a configuration file.
 * @path: the path of the file to create.
 * @conf: the configuration data to write.
 * @display_error: a callback function that displays an error. Works like in
 *                 the function above.
 */
void create(
    std::filesystem::path path,
    const Data &conf,
    std::function<void(std::string_view)> display_error
);

/*
 * Checks if a configuration file exists and:
 *
 *     - Parses the file if it exists;
 *     - Create the file with default values if it doesn't.
 *
 * @path: the path of the file to parse/create;
 * @valid: describe what the valid configuration data looks like;
 * @display_error: a callback function that displays an error. Works like in
 *                 the functions above.
 */
std::optional<Data> parse_or_create(
    std::filesystem::path path,
    const ValidConfig &valid,
    std::function<void(std::string_view)> display_error
);

/*
 * Tries to search for the configuration file on the file system.
 * Returns the path of the filesystem, or std::nullopt if not found.
 */
std::optional<std::filesystem::path> find_file(std::string_view name);

} // namespace conf
