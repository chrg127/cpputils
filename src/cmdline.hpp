/*
 * This is a library for command line parsing.
 * It implements a parsing function, parse(), that parses arguments in
 * accordance to most conventions:
 *
 *     - There are short and long options. Short options start with '-', long
 *       options start with '--';
 *     - Short options can be grouped, for example: -abc;
 *     - You can pass arguments to options. The argument follows the option;
 *     - For short options, the argument can appear as the next argument or
 *       packed together (-i input.txt or -iinput.txt);
 *     - For long options, the argument can appear as the next argument or
 *       after a '=' (--input input.txt or --input=input.txt)
 *     - Optional argument options exists: these can be specified with or
 *       without an argument;
 *     - It's possible to use '--' to control where options end (anything after
 *       -- is treated as a non-option.
 *
 * It's worth noting however, that any invalid option is simply ignored.
 *
 * The library also implements a function, print_options that formats a help
 * text.
 *
 * Refer to the comments below for how to use the library.
 */

#ifndef CMDLINE_HPP_INCLUDED
#define CMDLINE_HPP_INCLUDED

#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <span>
#include <fmt/core.h>

namespace cmdline {

/*
 * This struct defines a valid option. The user must define a list of these to
 * begin parsing.
 * @shortopt: what name to use when specifying the option using a single dash.
 *            Set it to '\0' for no short option.
 * @longopt: same as above, but with two dashes. This should always be specified.
 * @arg: argument type. Can be:
 *    - None: has no argument;
 *    - Required: needs an argument and won't be counted if not provided;
 *    - Optional: can have an argument, but will still be counted even if there
 *      are none;
 * @default_value: a default value for the argument if it wasn't found.
 * @argname: used when printing the help text. This name will be displayed after
 * the option name (for example: -f, --file FILE, where FILE is the argname).
 */
enum class ArgType { None, Required, Optional };

struct Option {
    char shortopt;
    std::string_view longopt;
    std::string_view desc;
    ArgType arg = ArgType::None;
    std::string_view default_value = "";
    std::string_view argname = "";
};

/*
 * This is returned as the result of parsing.
 * @opts: A set containing the valid options found. It is recommended to use
 *        the found() method for testing whether an option was found.
 * @args: If an option was found, its long name is mapped to an argument
 *        in this map (provided it specified an argument, of course).
 * @non_opts: Any non-options found, in order of how they are found
 *            in the argument list.
 * @argc and @argv: these are set at the end of parsing to tell where
 *                  the parser stopped. Without any flag, these will always
 *                  be 0 and one past the end of argv; with the flag
 *                  StopAtFirstNonOption, it is possible to use these to
 *                  implement subcommands.
 * @got_error: tracks whether any error were found during parsing.
 */
struct Result {
    std::unordered_set<std::string_view> opts;
    std::unordered_map<std::string_view, std::string_view> args;
    std::vector<std::string_view> non_opts;
    int argc; char **argv;
    bool got_error = false;
    bool found(std::string_view s) const             { return opts.find(s) != opts.end(); }
    void add(std::string_view o)                     { opts.insert(o); }
    void add(std::string_view o, std::string_view a) { opts.insert(o); args[o] = a; }
    Result &update_argcv(int i, int c, char **v)     { argc = c - i; argv = &v[i]; return *this; }
};

/*
 * These flags are used for parsing. You can specify them to control how
 * parsing is done.
 *     - StopAtFirstNonOption: stops parsing when encountering the first
 *       non-option. This flag can be used both for no mixing of options and
 *       non. The parse() function also keeps track of the remaining arguments
 *       before exiting, so this flag can be used for implementing subcommands.
 *     - StopAtFirstError: stops parsing on the first error encountered, rather
 *       than ignoring errors. The result's argc and argv are set to where the
 *       parser found an error. got_error indicates whether an error occurred,
 *       as usual.
 */
enum class Flags {
    None = 0x0,
    StopAtFirstNonOption = 0x1,
    StopAtFirstError = 0x2,
};

inline Flags operator|(Flags a, Flags b) { return static_cast<Flags>(static_cast<int>(a) | static_cast<int>(b)); }
inline Flags operator&(Flags a, Flags b) { return static_cast<Flags>(static_cast<int>(a) & static_cast<int>(b)); }

/*
 * The parse() function can produce these four warning.
 *     - InvalidOption means an invalid option was found;
 *     - ArgRequired means an argument that was required was not found;
 *     - ArgDefault warns that it will be using a default value for the
 *       argument. It sets the optional parameter for the warning function
 *       to the default value;
 *     - ArgIgnored indicates that a supplied argument was ignored because
 *       the option doesn't take any argument. Sets the optional parameter
 *       to the ignored argument;
 */
enum class Warn { InvalidOption, ArgRequired, ArgDefault, ArgIgnored };

namespace detail {

inline auto find_opt(            char c, std::span<const Option> os) { return std::find_if(os.begin(), os.end(), [&](const auto &o) { return o.shortopt == c; }); }
inline auto find_opt(std::string_view s, std::span<const Option> os) { return std::find_if(os.begin(), os.end(), [&](const auto &o) { return o.longopt == s; }); }
inline auto length_of(const Option &o) { return (o.shortopt == '\0' ? 0 : 4) + o.longopt.size() + 3 + o.argname.size(); }

} // namespace detail

inline void default_printer(Warn w, std::string_view o, std::string_view s)
{
    switch (w) {
    case Warn::InvalidOption: fmt::print("{}: invalid option\n", o); break;
    case Warn::ArgRequired:   fmt::print("{}: argument required\n", o); break;
    case Warn::ArgDefault:    fmt::print("{}: argument required (default {} will be used)\n", o, s); break;
    case Warn::ArgIgnored:    fmt::print("{}: argument {} ignored\n", o, s); break;
    }
}

/*
 * The main parsing function.  * It takes as parameters:
 * @argc and @argv: no explanation needed.
 * @opts: A collection (can be whatever you want, even a C array) of Option
 *        objects that specifies the valid options and arguments.
 * @flags: a value that controls details about how parsing is done. The
 *         possible flags you can pass are listed above.
 * @warning: a callback function to print error messages. It has three
 *           parameters: warning type, name of option, optional string.
 *           Refer to above for information about warning.
 */
inline Result parse(int argc, char *argv[], std::span<const Option> opts,
    Flags flags, auto &&warning)
{
    Result r;
    int i = 1;
    for ( ; i < argc; i++) {
        auto cur = std::string_view(argv[i]);
        if (cur == "--")            // all next arguments are non-options
            if ((flags & Flags::StopAtFirstNonOption) != Flags::None)
                break;
            else
                for ( ; i < argc; i++)
                    r.non_opts.push_back(cur);
        else if (cur[0] != '-')     // argument is a non-option
            if ((flags & Flags::StopAtFirstNonOption) != Flags::None)
                break;
            else
                r.non_opts.push_back(cur);
        else if (cur[1] != '-') {   // (group of) short option(s)
            for (auto j = 1u; j < cur.size(); j++) {
                auto it = detail::find_opt(cur[j], opts);
                if (it == opts.end()) {
                    warning(Warn::InvalidOption, cur.substr(j, 1), "");
                    r.got_error = true;
                    if ((flags & Flags::StopAtFirstError) != Flags::None)
                        return r.update_argcv(i, argc, argv);
                } else if (it->arg == ArgType::None)
                    r.add(it->longopt);
                else if (j+1 != cur.size()) // argument is rest of the string.
                    r.add(it->longopt, cur.substr(++j));
                else if (i+1 != argc)       // argument is next in argv
                    r.add(it->longopt, argv[++i]);
                else if (it->arg == ArgType::Optional)
                    r.add(it->longopt);
                else if (it->default_value != "") {
                    warning(Warn::ArgDefault, it->longopt, it->default_value);
                    r.add(it->longopt, it->default_value);
                } else {
                    warning(Warn::ArgRequired, it->longopt, "");
                    r.got_error = true;
                    if ((flags & Flags::StopAtFirstError) != Flags::None)
                        return r.update_argcv(i, argc, argv);
                }
            }
        } else {                    // long option
            auto eqpos = cur.find('=', 2);  // split up by '='
            auto opt = eqpos == cur.npos ? cur.substr(2)
                                         : cur.substr(2, eqpos-2);
            auto it = detail::find_opt(opt, opts);
            if (it == opts.end()) {
                warning(Warn::InvalidOption, opt, "");
                r.got_error = true;
                if ((flags & Flags::StopAtFirstError) != Flags::None)
                    return r.update_argcv(i, argc, argv);
            } else if (it->arg == ArgType::None) {
                if (eqpos != cur.npos)
                    warning(Warn::ArgIgnored, opt, cur.substr(eqpos+1));
                r.add(it->longopt);
            } else if (eqpos != cur.npos)
                r.add(it->longopt, cur.substr(eqpos+1));
            else if (i+1 != argc)
                r.add(it->longopt, argv[++i]);
            else if (it->arg == ArgType::Optional)
                r.add(it->longopt);
            else if (it->default_value != "") {
                warning(Warn::ArgDefault, it->longopt, it->default_value);
                r.add(it->longopt, it->default_value);
            } else {
                warning(Warn::ArgRequired, it->longopt, "");
                r.got_error = true;
                if ((flags & Flags::StopAtFirstError) != Flags::None)
                    return r.update_argcv(i, argc, argv);
            }
        }
    }
    return r.update_argcv(i, argc, argv);
}

/* A simple helper that sets no flags and the warning callback to the default printer. */
inline Result parse(int argc, char *argv[], std::span<const Option> opts, Flags f = Flags::None)
{
    return parse(argc, argv, opts, f, default_printer);
}

/*
 * Prints a help text.
 * @opts: a list of Option objects. Same as parse().
 * @output: a callback function to where should the messages be printed.
 */
inline void print_options(std::span<const Option> opts, auto &&output)
{
    const auto maxopt = std::max_element(opts.begin(), opts.end(), [&](const auto &p, const auto &q) {
        return detail::length_of(p) < detail::length_of(q);
    });
    auto width = detail::length_of(*maxopt);
    output("Valid arguments:");
    for (const auto &o : opts)
        output(fmt::format("    {:{}}    {}",
                o.shortopt != '\0' ? fmt::format("-{}, --{} {}", o.shortopt, o.longopt, o.argname)
                                   : fmt::format("--{} {}", o.longopt, o.argname),
                width, o.desc));
}

/*
 * A helper for print_options that sets the output callback to a function
 * that only prints to stdout.
 */
inline void print_options(std::span<const Option> opts)
{
    print_options(opts, [](const auto &s) { fmt::print("{}\n", s); });
}

} // namespace cmdline

#endif
