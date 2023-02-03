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

namespace conf {

namespace {

struct Token {
    enum class Type {
        Ident, Int, Float, True, False, String, EqualSign, Newline,
        Unterminated, InvalidChar, End,
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

    Token make(Token::Type type)
    {
        return Token {
            .type = type,
            .text = text.substr(start, cur - start),
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
            return make(Token::Type::Unterminated);
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
             : make(Token::Type::InvalidChar);
    }
};

struct Parser {
    Lexer lexer;
    Token cur, prev;
    std::vector<ParseError> errors;

    explicit Parser(std::string_view s) : lexer{s} {}

    void error(Token t, ErrorType err)
    {
        auto [line, col] = lexer.position_of(t);
        throw ParseError {
            .error = std::error_condition(static_cast<int>(err), conf_error_category),
            .line = line, .col = col,
            .prev = std::string(prev.text),
            .cur  = t.type == Token::Type::End ? "end" : std::string(t.text),
        };
    }

    void advance()
    {
        prev = cur, cur = lexer.lex();
        if (cur.type == Token::Type::Unterminated) error(cur, ErrorType::UnterminatedString);
        if (cur.type == Token::Type::InvalidChar)  error(cur, ErrorType::UnexpectedCharacter);
    }

    void consume(Token::Type type, ErrorType err) { cur.type == type ? advance() : error(prev, err); }
    bool match(Token::Type type)                  { if (cur.type != type) return false; else { advance(); return true; } }

    ParseResult parse()
    {
        conf::Data data;
        advance();
        while (!lexer.at_end()) {
            try {
                if (!match(Token::Type::Newline)) {
                    consume(Token::Type::Ident, ErrorType::NoIdent);
                    auto ident = prev;
                    consume(Token::Type::EqualSign, ErrorType::NoEqualAfterIdent);
                    auto &pos = data[std::string(ident.text)];
                         if (match(Token::Type::Int))    pos = conf::Value(string::to_number<  int>(prev.text).value());
                    else if (match(Token::Type::Float))  pos = conf::Value(string::to_number<float>(prev.text).value());
                    else if (match(Token::Type::String)) pos = conf::Value(std::string(prev.text.substr(1, prev.text.size() - 2)));
                    else if (match(Token::Type::True)
                          || match(Token::Type::False))  pos = conf::Value(prev.type == Token::Type::True);
                    else error(prev, ErrorType::NoValueAfterEqual);
                    consume(Token::Type::Newline, ErrorType::NoNewlineAfterValue);
                }
            } catch (const ParseError &error) {
                errors.push_back(error);
                while (cur.type != Token::Type::End && cur.type != Token::Type::Newline)
                    advance();
                advance();
            }
        }
        if (!errors.empty())
            return tl::unexpected(errors);
        return data;
    }
};

} // namespace

std::string ConfErrorCategory::message(int n) const
{
    switch (static_cast<ErrorType>(n)) {
    case ErrorType::NoIdent:                return "expected identifier";
    case ErrorType::NoEqualAfterIdent:      return "expected '=' after identifier";
    case ErrorType::NoValueAfterEqual:      return "expected value after '='";
    case ErrorType::NoNewlineAfterValue:    return "expected newline after value";
    case ErrorType::UnterminatedString:     return "unterminated string";
    case ErrorType::UnexpectedCharacter:    return "unexpected character";
    default:                                return "unknown error";
    }
}

ParseResult parse(std::string_view text)
{
    Parser parser{text};
    return parser.parse();
}

std::error_code create(fs::path path, const Data &conf)
{
    auto file = io::File::open(path, io::Access::Write);
    if (!file)
        return file.error();
    auto width = std::max_element(conf.begin(), conf.end(), [](const auto &a, const auto &b) {
        return a.first.size() < b.first.size();
    })->first.size();
    for (auto [k, v] : conf)
        fmt::print(file.value().data(), "{:{}} = {}\n", k, width, v.to_string());
    return std::error_code{};
}

ParseResult parse_or_create(fs::path path, const Data &defaults)
{
    if (auto text = io::read_file(path); text)
        return parse(text.value());
    if (auto err = create(path, defaults); err)
        return tl::unexpected(std::vector{ParseError::make(err)});
    return defaults;
}

std::vector<Warning> validate(Data &conf, const Data &valid_conf)
{
    std::vector<Warning> warnings;
    // remove invalid keys
    for (auto [k, _] : conf) {
        if (auto r = valid_conf.find(k); r == valid_conf.end()) {
            warnings.push_back({.type = Warning::Type::InvalidKey, .key = k });
            conf.erase(k);
        }
    }
    // find missing keys and type match existing ones
    for (auto [k, v] : valid_conf) {
        auto r = conf.find(k);
        if (r == conf.end()) {
            warnings.push_back({.type = Warning::Type::MissingKey, .key = k });
            conf[k] = v;
        } else if (v.type() != r->second.type()) {
            warnings.push_back({.type = Warning::Type::MismatchedTypes,
                                .key  = k });
            conf[k] = v;
        }
    }
    return warnings;
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
