#pragma once

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include "common.hpp"

namespace io {

namespace detail {
    inline void file_deleter(FILE *fp)
    {
        if (fp && fp != stdin && fp != stdout && fp != stderr)
            std::fclose(fp);
    }

    template <typename T>
    inline std::expected<T, std::error_code> read_file(std::filesystem::path path)
    {
        FILE *file = fopen(path.string().c_str(), "rb");
        if (!file) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }
        fseek(file, 0l, SEEK_END);
        long size = ftell(file);
        rewind(file);
        auto buf = T(size, ' ');
        size_t bytes_read = fread(buf.data(), sizeof(char), size, file);
        if (bytes_read < std::size_t(size)) {
            return std::unexpected(std::error_code(errno, std::system_category()));
        }
        fclose(file);
        return buf;
    }
} // namespace detail

/* A file can be opened in one of four ways: */
enum class Access { Read, Write, Modify, Append, };

/*
 * This is a RAII-style thin wrapper over a FILE *. It will automatically close
 * the file when destroyed, unless it is one of stdin, stdout, stderr.
 *
 * @open: a static method for opening Files;
 * @assoc: used to associate an existing FILE * with a File object. Mostly
 *         useful for stdin, stdout and stderr. Using assoc(), the filename
 *         will always be set to '/';
 * @get_word: gets a single word from the File and returns it as a string. A
 *            word is delimited by any kind of space;
 * @get_line: same as above, but for lines. The line delimiter is a single
 *            newline;
 * @filename: returns the file's name, which is NOT the entire file path;
 * @file_path: returns the file's entire path as a path object;
 * @data: returns the underlying FILE *;
 * @getc: corresponds to std::fgetc;
 * @ungetc: corresponds to std::ungetc;
 * @close: manually closes the file. To be used if you care about the return value;
 */
class File {
    std::unique_ptr<FILE, void (*)(FILE *)> file_ptr = { nullptr, detail::file_deleter };
    std::filesystem::path filepath;

    File(FILE *f, std::filesystem::path p)
        : file_ptr{f, detail::file_deleter}, filepath{std::move(p)}
    { }

public:
    static auto open(std::filesystem::path pathname, Access access)
        -> std::expected<File, std::error_code> 
    {
        FILE *fp = [&](const std::string &name) -> FILE * {
            switch (access) {
            case Access::Read:   return fopen(name.c_str(), "rb"); break;
            case Access::Write:  return fopen(name.c_str(), "wb"); break;
            case Access::Modify: return fopen(name.c_str(), "rb+"); break;
            case Access::Append: return fopen(name.c_str(), "ab"); break;
            default:             return nullptr;
            }
        }(pathname.string());
        if (!fp)
            return std::unexpected{std::error_code(errno, std::system_category())};
        return File{fp, pathname};
    }

    static File assoc(FILE *fp) { return {fp, std::filesystem::path("/")}; }

    int close() { return std::fclose(file_ptr.release()); }

    std::string           name() const noexcept { return filepath.filename().string(); }
    std::filesystem::path path() const noexcept { return filepath; }
    FILE *                data() const noexcept { return file_ptr.get(); }

    int getc()        { return std::fgetc(file_ptr.get()); }
    int ungetc(int c) { return std::ungetc(c, file_ptr.get()); }

    bool get_word(std::string &str)
    {
        auto is_delim = [](int c) { return c == ' '  || c == '\t' || c == '\r' || c == '\n'; };
        auto is_space = [](int c) { return c == ' '  || c == '\t' || c == '\r'; };
        str.erase();
        int c;
        while (c = getc(), is_space(c) && c != EOF)
            ;
        ungetc(c);
        while (c = getc(), !is_delim(c) && c != EOF)
            str += c;
        ungetc(c);
        return !(c == EOF);
    }

    bool get_line(std::string &str, int delim = '\n')
    {
        str.erase();
        int c;
        while (c = getc(), c != delim && c != EOF)
            str += c;
        return !(c == EOF);
    }

    std::vector<u8> read(std::size_t n)
    {
        auto buf = std::vector<u8>(n, 0);
        auto num_read = std::fread(buf.data(), sizeof(u8), buf.size(), file_ptr.get());
        buf.resize(num_read);
        return buf;
    }

    std::size_t read(std::span<u8> buf)
    {
        return std::fread(buf.data(), sizeof(u8), buf.size(), file_ptr.get());
    }

    std::size_t write(std::span<u8> bytes)
    {
        return std::fwrite(bytes.data(), sizeof(u8), bytes.size(), file_ptr.get());
    }
};

inline std::expected<std::string, std::error_code> read_file(std::filesystem::path path)
{
    return detail::read_file<std::string>(path);
}

inline std::expected<std::vector<u8>, std::error_code> read_file_bytes(std::filesystem::path path)
{
    return detail::read_file<std::vector<u8>>(path);
}

} // namespace io
