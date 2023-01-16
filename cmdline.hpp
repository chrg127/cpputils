#pragma once

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <span>
#include <fmt/core.h>

namespace cmdline {

enum class ParamType { None, Single };

struct Argument {
    char short_opt;
    std::string_view long_opt;
    std::string_view desc;
    ParamType param_type = ParamType::None;
    std::string_view default_param = "";
};

struct Result {
    std::unordered_set<char> found;
    std::unordered_map<char, std::string_view> params;
    std::vector<std::string_view> items;
    bool has(char flag) const { return found.find(flag) != found.end(); }
};

inline auto find_arg(std::string_view arg, std::span<const Argument> l) { return std::find_if(l.begin(), l.end(), [&](const auto &a) { return a.long_opt  == arg; }); }
inline auto find_arg(            char arg, std::span<const Argument> l) { return std::find_if(l.begin(), l.end(), [&](const auto &a) { return a.short_opt == arg; }); }

inline Result parse(int argc, char *argv[], std::span<const Argument> valid, auto &&warning)
{
    Result res;
    while (++argv, --argc > 0) {
        std::string_view curr = *argv;
        if (curr[0] != '-' || (curr[0] == '-' && curr.size() == 1)) {
            res.items.push_back(curr);
            continue;
        }
        auto arg = curr[1] == '-'   ? find_arg(curr.substr(2), valid)
                 : curr.size() == 2 ? find_arg(curr[1], valid)
                 :                    valid.end();
        if (arg == valid.end()) {
            warning(fmt::format("invalid argument: {}", curr));
            continue;
        }
        if (res.has(arg->short_opt)) {
            warning(fmt::format("argument {} was specified multiple times", curr));
            continue;
        }
        res.found.insert(arg->short_opt);
        if (arg->param_type != ParamType::None) {
            ++argv; --argc;
            if (argc == 0) {
                warning(fmt::format("argument {} needs a parameter (default \"{}\" will be used)", curr, arg->default_param));
                res.params[arg->short_opt] = arg->default_param;
            } else {
                res.params[arg->short_opt] = *argv;
            }
        }
    }
    return res;
}

inline Result parse(int argc, char *argv[], std::span<const Argument> valid)
{
    return parse(argc, argv, valid, [](const auto &s) { fmt::print("{}\n", s); });
}

inline void print_args(std::span<const Argument> args, auto &&output)
{
    const auto maxwidth = std::max_element(args.begin(), args.end(), [](const auto &p, const auto &q) {
        return p.long_opt.size() < q.long_opt.size();
    })->long_opt.size();
    output("Valid arguments:");
    for (const auto &arg : args)
        output(fmt::format("    -{}, --{:{}}    {}", arg.short_opt, arg.long_opt, maxwidth, arg.desc));
}

inline void print_args(std::span<const Argument> args)
{
    print_args(args, [](const auto &s) { fmt::print("{}\n", s); });
}

} // namespace cmdline
