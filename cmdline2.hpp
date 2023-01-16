#pragma once

#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <fmt/core.h>

namespace cmdline2 {

enum class ArgType { None, Required, Optional };

struct Option {
    char shortopt;
    std::string_view longopt;
    std::string_view desc;
    ArgType arg = ArgType::None;
    std::string_view default_arg = "";
    std::string_view arg_name = "";
};

struct Result {
    std::unordered_set<std::string_view> opts;
    std::unordered_map<std::string_view, std::string_view> args;
    std::vector<std::string_view> non_opts;
    bool found(std::string_view s) const { return opts.find(s) != opts.end(); }
    void add(std::string_view o) { opts.insert(o); }
    void add(std::string_view o, std::string_view a) { opts.insert(o); args[o] = a; }
};

inline auto find_opt(            char c, std::span<const Option> os) { return std::find_if(os.begin(), os.end(), [&](const auto &o) { return o.shortopt == c; }); }
inline auto find_opt(std::string_view s, std::span<const Option> os) { return std::find_if(os.begin(), os.end(), [&](const auto &o) { return o.longopt == s; }); }

inline Result parse(int argc, char *argv[], std::span<const Option> opts, auto &&output)
{
    Result r;
    for (int i = 1; i < argc; i++) {
        auto cur = std::string_view(argv[i]);
        if (cur == "--")    // this tells us all next args are non-options
            for ( ; i < argc; i++)
                r.non_opts.push_back(cur);
        else if (cur[0] != '-')
            r.non_opts.push_back(cur);
        else if (cur[1] != '-') {
            for (int j = 1; j < cur.size(); j++) {
                auto it = find_opt(cur[j], opts);
                if (it == opts.end())
                    output(fmt::format("{}: invalid option", cur[j]));
                else if (it->arg == ArgType::None)
                    r.add(it->longopt);
                else if (j+1 != cur.size()) // argument is the rest of the string.
                    r.add(it->longopt, cur.substr(++j));
                else if (i+1 != argc)       // argument is next in argv
                    r.add(it->longopt, argv[++i]);
                else if (it->arg == ArgType::Optional)
                    r.add(it->longopt);
                else if (it->default_arg != "") {
                    output(fmt::format("option {} requires an argument (default {} will be used)", it->longopt, it->default_arg));
                    r.add(it->longopt, it->default_arg);
                } else
                    output(fmt::format("option {} requires an argument", it->longopt));
            }
        } else {
            auto eqpos = cur.find('=', 2);  // split up by '='
            auto opt = eqpos == cur.npos ? cur.substr(2)
                                         : cur.substr(2, eqpos-2);
            auto it = find_opt(opt, opts);
            if (it == opts.end())
                output(fmt::format("{}: invalid option", opt));
            else if (it->arg == ArgType::None) {
                if (eqpos != cur.npos)
                    output(fmt::format("option {} doesn't specify any arguments", opt));
                r.add(it->longopt);
            } else if (eqpos != cur.npos)
                r.add(it->longopt, cur.substr(eqpos+1));
            else if (i+1 != argc)
                r.add(it->longopt, argv[++i]);
            else if (it->arg == ArgType::Optional)
                r.add(it->longopt);
            else if (it->default_arg != "") {
                output(fmt::format("option {} requires an argument (default {} will be used)", it->longopt, it->default_arg));
                r.add(it->longopt, it->default_arg);
            } else
                output(fmt::format("option {} requires an argument", it->longopt));
        }
    }
    return r;
}

inline Result parse(int argc, char *argv[], std::span<const Option> opts)
{
    return parse(argc, argv, opts, [](const auto &s) { fmt::print("{}\n", s); });
}

inline void print_options(std::span<const Option> opts, auto &&output)
{
    const auto maxwidth = std::max_element(opts.begin(), opts.end(), [](const auto &p, const auto &q) {
        return p.longopt.size() < q.longopt.size();
    })->longopt.size();
    output("Valid arguments:");
    for (const auto &o : opts)
        output(fmt::format("    -{}, --{:{}}    {}", o.shortopt, o.longopt, maxwidth, o.desc));
}

inline void print_options(std::span<const Option> opts)
{
    print_options(opts, [](const auto &s) { fmt::print("{}\n", s); });
}

} // namespace cmdline
