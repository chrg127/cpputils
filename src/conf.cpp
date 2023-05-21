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
using namespace std::literals::string_literals;

namespace conf {

namespace {

struct Token {
    enum class Type {
        Ident, Int, Float, True, False, String, EqualSign, Newline,
        LeftSquareBracket, RightSquareBracket, Comma,
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
             : c == '['    ? make(Token::Type::LeftSquareBracket)
             : c == ']'    ? make(Token::Type::RightSquareBracket)
             : c == ','    ? make(Token::Type::Comma)
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

    void error(Token t, int err)
    {
        auto [line, col] = lexer.position_of(t);
        throw ParseError {
            .error = std::error_condition(static_cast<int>(err), errcat),
            .line = line, .col = std::ptrdiff_t(col),
            .prev = std::string(prev.text),
            .cur  = t.type == Token::Type::End ? "end" : std::string(t.text),
        };
    }

    void advance()
    {
        prev = cur, cur = lexer.lex();
        if (cur.type == Token::Type::Unterminated) error(cur, ParseError::UnterminatedString);
        if (cur.type == Token::Type::InvalidChar)  error(cur, ParseError::UnexpectedCharacter);
    }

    void consume(Token::Type type, int err) { cur.type == type ? advance() : error(prev, err); }
    bool match(Token::Type type)            { if (cur.type != type) return false; else { advance(); return true; } }

    std::optional<conf::Value> parse_value()
    {
             if (match(Token::Type::Int))    return conf::Value(string::to_number<  int>(prev.text).value());
        else if (match(Token::Type::Float))  return conf::Value(string::to_number<float>(prev.text).value());
        else if (match(Token::Type::String)) return conf::Value(std::string(prev.text.substr(1, prev.text.size() - 2)));
        else if (match(Token::Type::True)
              || match(Token::Type::False))  return conf::Value(prev.type == Token::Type::True);
        else if (match(Token::Type::LeftSquareBracket)) return parse_list();
        return std::nullopt;
    }

    conf::Value parse_list()
    {
        std::vector<conf::Value> values;
        do
            if (auto v = parse_value(); v)
                values.push_back(v.value());
        while (match(Token::Type::Comma));
        consume(Token::Type::RightSquareBracket, ParseError::ExpectedComma);
        return conf::Value(std::move(values));
    }

    ParseResult parse()
    {
        conf::Data data;
        advance();
        while (!lexer.at_end()) {
            try {
                if (!match(Token::Type::Newline)) {
                    consume(Token::Type::Ident, ParseError::NoIdent);
                    auto ident = prev;
                    consume(Token::Type::EqualSign, ParseError::NoEqualAfterIdent);
                    auto &pos = data[std::string(ident.text)];
                    if (auto v = parse_value(); v)
                        pos = v.value();
                    else
                        error(prev, ParseError::NoValueAfterEqual);
                    consume(Token::Type::Newline, ParseError::NoNewlineAfterValue);
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

std::string ParseError::message()
{
    return error.category() != conf::errcat
        ? "error: " + error.message()
        : std::to_string(line) + ":" + std::to_string(col)
        + ": parse error: " + error.message();
}

std::string Warning::message()
{
    switch (type) {
    case Warning::Type::InvalidKey: return "invalid key '" + key + "'";
    case Warning::Type::MissingKey:
        return "missing key '" + key + "' (default " + newval.to_string()
             + " will be used)";
    case Warning::Type::MismatchedTypes:
         return "mismatched types for key '" + key
              + "' (expected type '" + std::string(type_to_string(newval.type()))
              + "', got '" + orig.to_string() + "' of type '"
              + std::string(type_to_string(orig.type()))
              + "') (default '" + newval.to_string() + "' will be used)";
    default: return "";
    }
}

std::string ConfErrorCategory::message(int n) const
{
    switch (n) {
    case ParseError::NoIdent:             return "expected identifier";
    case ParseError::NoEqualAfterIdent:   return "expected '=' after identifier";
    case ParseError::NoValueAfterEqual:   return "expected value after '='";
    case ParseError::NoNewlineAfterValue: return "expected newline after value";
    case ParseError::UnterminatedString:  return "unterminated string";
    case ParseError::UnexpectedCharacter: return "unexpected character";
    case ParseError::ExpectedComma:       return "expected comma";
    default:                              return "unknown error";
    }
}

ParseResult parse(std::string_view text)
{
    Parser parser{text};
    return parser.parse();
}

std::vector<Warning> validate(Data &data, const Data &valid_data)
{
    std::vector<Warning> ws;
    // remove invalid keys
    for (auto [k, _] : data) {
        if (auto r = valid_data.find(k); r == valid_data.end()) {
            ws.push_back({ .type = Warning::Type::InvalidKey, .key = k });
            data.erase(k);
        }
    }
    // find missing keys and type match existing ones
    for (auto [k, v] : valid_data) {
        auto r = data.find(k);
        if (r == data.end()) {
            ws.push_back({.type = Warning::Type::MissingKey, .key = k, .newval = v });
            data[k] = v;
        } else if (v.type() != r->second.type()) {
            ws.push_back({ .type = Warning::Type::MismatchedTypes,
                           .key = k, .newval = v, .orig = r->second });
            data[k] = v;
        }
    }
    return ws;
}

std::error_code create(std::filesystem::path path, const Data &data)
{
    auto file = io::File::open(path, io::Access::Write);
    if (!file)
        return file.error();
    auto width = std::max_element(data.begin(), data.end(), [](const auto &a, const auto &b) {
        return a.first.size() < b.first.size();
    })->first.size();
    for (auto [k, v] : data)
        fmt::print(file.value().data(), "{:{}} = {}\n", k, width, v.to_string());
    return std::error_code{};
}

std::filesystem::path getdir(std::string_view appname)
{
    auto config = io::directory::config();
    auto appdir = fs::exists(config) ? config / appname
                                     : io::directory::home() / ("." + std::string(appname));
    if (!fs::exists(appdir))
        fs::create_directory(appdir);
    return appdir;
}

ParseResult parse_or_create(std::string_view appname, const Data &defaults)
{
    auto file_path = getdir(appname) / (std::string(appname) + ".conf");
    if (auto text = io::read_file(file_path); text)
        return parse(text.value());
    if (auto err = create(file_path, defaults); err)
        return tl::unexpected(std::vector{ ParseError { .error = err.default_error_condition() } });
    return defaults;
}

} // namespace conf
