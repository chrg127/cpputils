#include <cstdint>
#include <cstddef>
#include <vector>
#include <charconv>
#include <filesystem>
#include <fmt/core.h>
#include "conf.hpp"
#include "io.hpp"
#include "string.hpp"

namespace fs = std::filesystem;

namespace {

std::string_view type_to_string(conf::Type t)
{
    switch (t) {
    case conf::Type::Int:    return "int";
    case conf::Type::Float:  return "float";
    case conf::Type::Bool:   return "bool";
    case conf::Type::String: return "string";
    default: return "";
    }
}

struct Token {
    enum class Type {
        Ident, Int, Float, True, False,
        String, EqualSign, Newline, Error, End,
    } type;
    std::string_view text;
    std::size_t pos;
};

bool is_ident_char(char c) { return string::is_alpha(c) || c == '_' || c == '-'; }

struct Lexer {
    std::string_view text;
    std::size_t cur = 0, start = 0;

    explicit Lexer(std::string_view s) : text{s} {}

    char peek() const               { return text[cur]; }
    char advance()                  { return text[cur++]; }
    bool at_end() const             { return text.size() == cur; }

    auto position_of(Token t) const
    {
        auto tmp = text.substr(0, t.pos);
        auto line = std::count(tmp.begin(), tmp.end(), '\n') + 1;
        auto column = t.pos - tmp.find_last_of('\n');
        return std::make_pair(line, column);
    }

    Token make(Token::Type type, std::string_view msg = "")
    {
        return Token {
            .type = type,
            .text = msg.empty() ? text.substr(start, cur - start) : msg,
            .pos  = start,
        };
    }

    void skip()
    {
        for (;;) {
            switch (peek()) {
            case ' ': case '\r': case '\t': cur++; break;
            case '#':
                while (peek() != '\n')
                    advance();
                break;
            default:
                return;
            }
        }
    }

    Token number()
    {
        while (string::is_digit(text[cur]))
            advance();
        if (peek() != '.')
            return make(Token::Type::Int);
        advance();
        while (string::is_digit(text[cur]))
            advance();
        return make(Token::Type::Float);
    }

    Token ident()
    {
        while (is_ident_char(peek()) || string::is_digit(peek()))
            advance();
        auto word = text.substr(start, cur - start);
        return make(word == "true"  ? Token::Type::True
                  : word == "false" ? Token::Type::False
                  :                   Token::Type::Ident);
    }

    Token string_token()
    {
        while (peek() != '"' && !at_end())
            advance();
        if (at_end())
            return make(Token::Type::Error, "unterminated string");
        advance();
        return make(Token::Type::String);
    }

    Token lex()
    {
        skip();
        start = cur;
        if (at_end())
            return make(Token::Type::End);
        char c = advance();
        return c == '='    ? make(Token::Type::EqualSign)
             : c == '\n'   ? make(Token::Type::Newline)
             : c == '"'    ? string_token()
             : string::is_digit(c) ? number()
             : is_ident_char(c) ? ident()
             : make(Token::Type::Error, "unexpected character");
    }
};

struct Parser {
    Lexer lexer;
    Token cur, prev;
    bool had_error = false;

    struct ParseError : std::runtime_error {
        using std::runtime_error::runtime_error;
        using std::runtime_error::what;
    };

    explicit Parser(std::string_view s) : lexer{s} {}

    void error(Token t, std::string_view msg)
    {
        had_error = true;
        auto [line, col] = lexer.position_of(t);
        throw ParseError(
            fmt::format("{}:{}: parse error{}: {}",
                line, col, t.type == Token::Type::End   ? " on end of file"
                         : t.type == Token::Type::Error ? ""
                         : fmt::format(" at '{}'", t.text), msg)
        );
    }

    void advance()
    {
        if (prev = cur, cur = lexer.lex(), cur.type == Token::Type::Error)
            error(cur, cur.text);
    }

    void consume(Token::Type type, std::string_view msg) { cur.type == type ? advance() : error(prev, msg); }
    bool match(Token::Type type)                         { if (cur.type != type) return false; else { advance(); return true; } }

    std::optional<conf::Data> parse(auto &&display_error)
    {
        conf::Data data;
        advance();
        while (!lexer.at_end()) {
            try {
                if (!match(Token::Type::Newline)) {
                    consume(Token::Type::Ident, "expected identifier");
                    auto ident = prev;
                    consume(Token::Type::EqualSign, fmt::format("expected '=' after identifier '{}'", ident.text));
                    auto &pos = data[std::string(ident.text)];
                         if (match(Token::Type::Int))    pos = conf::Value(string::to_number<  int>(prev.text).value());
                    else if (match(Token::Type::Float))  pos = conf::Value(string::to_number<float>(prev.text).value());
                    else if (match(Token::Type::String)) pos = conf::Value(std::string(prev.text.substr(1, prev.text.size() - 2)));
                    else if (match(Token::Type::True)
                          || match(Token::Type::False))  pos = conf::Value(prev.type == Token::Type::True);
                    else error(prev, "expected value after '='");
                    consume(Token::Type::Newline, fmt::format("expected newline after value '{}'", prev.text));
                }
            } catch (const ParseError &error) {
                display_error(error.what());
                while (cur.type != Token::Type::End && cur.type != Token::Type::Newline)
                    advance();
                advance();
            }
        }
        if (had_error)
            return std::nullopt;
        return data;
    }
};

} // namespace

namespace conf {

std::optional<Data> parse(std::string_view text, DisplayCallback error)
{
    Parser parser{text};
    auto res = parser.parse(error);
    if (!res)
        return std::nullopt;
    return res;
}

Data validate(Data conf, const Data &valid_conf, DisplayCallback warning)
{
    // remove invalid keys
    for (auto [k, v] : conf) {
        if (auto r = valid_conf.find(k); r == valid_conf.end()) {
            warning(fmt::format("warning: invalid key '{}' (will be removed)", k));
            conf.erase(k);
        }
    }
    // find missing keys and type match existing ones
    for (auto [k, v] : valid_conf) {
        auto r = conf.find(k);
        if (r == conf.end()) {
            warning(fmt::format("warning: missing key '{}' (default '{}' will be used)", k, v.to_string()));
            conf[k] = v;
        } else if (v.type() != r->second.type()) {
            warning(fmt::format("warning: mismatched types for key '{}' (expected {}, got {}) (default '{}' will be used)",
                    k, type_to_string(r->second.type()), type_to_string(v.type()), v.to_string()));
            conf[k] = v;
        }
    }
    return conf;
}

void create(fs::path path, const Data &conf, DisplayCallback error)
{
    if (conf.size() == 0) {
        error(fmt::format("error: no data"));
        return;
    }
    auto file = io::File::open(path, io::Access::Write);
    if (!file) {
        error(fmt::format("error: couldn't create file {}: {}\n",
                path.string(), file.error().message()));
        return;
    }
    auto width = std::max_element(conf.begin(), conf.end(), [](const auto &a, const auto &b) {
        return a.first.size() < b.first.size();
    })->first.size();
    for (auto [k, v] : conf)
        fmt::print(file.value().data(), "{:{}} = {}\n", k, width, v.to_string());
}

std::optional<Data> parse_or_create(fs::path path, const Data &defaults,
    DisplayCallback error)
{
    auto text = io::read_file(path);
    if (!text) {
        error(fmt::format("warning: couldn't open file {} ({}), creating new one...",
                path.string(), text.error().message()));
        create(path, defaults, error);
        return defaults;
    }
    return parse(text.value(), error);
}

std::optional<fs::path> find_file(std::string_view name)
{
    auto home = io::user_home();
    auto paths = std::array {
        home / ".config" / name / name,
        home / fmt::format(".{}", name) / fmt::format("{}.conf", name),
        home / fmt::format("{}.conf", name)
    };
    for (const auto &path : paths) {
        if (fs::exists(path))
            return path;
    }
    return std::nullopt;
}

} // namespace conf
