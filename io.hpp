#pragma once

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <expected.hpp>
#include "common.hpp"

#if defined(PLATFORM_LINUX)
#   include <fcntl.h>
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <unistd.h>
#else
#   warning "platform not supported"
#endif

namespace io {

namespace detail {

inline void file_deleter(FILE *fp)
{
    if (fp && fp != stdin && fp != stdout && fp != stderr)
        std::fclose(fp);
}

#ifdef PLATFORM_LINUX

inline std::pair<u8 *, std::size_t> open_mapped_file(std::filesystem::path path)
{
    int fd = ::open(path.c_str(), O_RDWR);
    if (fd < 0)
        return {nullptr, 0};
    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if (err < 0)
        return {nullptr, 0};
    auto *ptr = (u8 *) mmap(nullptr, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
        return {nullptr, 0};
    close(fd);
    return {ptr, static_cast<std::size_t>(statbuf.st_size)};
}

inline void close_mapped_file(u8 *ptr, std::size_t len)
{
    if (ptr) ::munmap(ptr, len);
}

#endif

inline std::error_code make_error()
{
    return std::error_code(errno, std::system_category());
}

} // namespace detail

enum class Access { Read, Write, Modify, Append, };

template <typename T>
using Result = tl::expected<T, std::error_code>;

class File {
    std::unique_ptr<FILE, void (*)(FILE *)> file_ptr = { nullptr, detail::file_deleter };
    std::filesystem::path path;

    File(FILE *f, std::filesystem::path p)
        : file_ptr{f, detail::file_deleter}, path{std::move(p)}
    { }

public:
    static Result<File> open(std::filesystem::path pathname, Access access)
    {
        FILE *fp = [&](const char *name) -> FILE * {
            switch (access) {
            case Access::Read:   return fopen(name, "rb"); break;
            case Access::Write:  return fopen(name, "wb"); break;
            case Access::Modify: return fopen(name, "rb+"); break;
            case Access::Append: return fopen(name, "ab"); break;
            default:             return nullptr;
            }
        }(pathname.c_str());
        if (!fp)
            return tl::unexpected{detail::make_error()};
        return File{fp, pathname};
    }

    static File assoc(FILE *fp) { return {fp, std::filesystem::path("/")}; }

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

    std::string filename() const noexcept            { return path.filename().c_str(); }
    std::filesystem::path file_path() const noexcept { return path; }
    FILE *data() const noexcept                      { return file_ptr.get(); }
    int getc()                                       { return std::fgetc(file_ptr.get()); }
    int ungetc(int c)                                { return std::ungetc(c, file_ptr.get()); }
};

class MappedFile {
    u8 *ptr = nullptr;
    std::size_t len = 0;
    std::filesystem::path path;

    MappedFile(u8 *p, std::size_t s, std::filesystem::path pa)
        : ptr{p}, len{s}, path{pa}
    { }

public:
    ~MappedFile() { detail::close_mapped_file(ptr, len); }

    MappedFile(const MappedFile &) = delete;
    MappedFile & operator=(const MappedFile &) = delete;
    MappedFile(MappedFile &&m) noexcept { operator=(std::move(m)); }
    MappedFile & operator=(MappedFile &&m) noexcept
    {
        std::swap(ptr, m.ptr);
        std::swap(len, m.len);
        std::swap(path, m.path);
        return *this;
    }

    static Result<MappedFile> open(std::filesystem::path path)
    {
        auto [p, s] = detail::open_mapped_file(path);
        if (!p)
            return tl::unexpected(detail::make_error());
        return MappedFile(p, s, path);
    }

    std::span<u8> slice(std::size_t start, std::size_t length) { return { ptr + start, length}; }
    u8 operator[](std::size_t index)                           { return ptr[index]; }
    u8 *begin() const                                          { return ptr; }
    u8 *end() const                                            { return ptr + len; }
    const u8 *data() const                                     { return ptr; }
    u8 *data()                                                 { return ptr; }
    std::size_t size() const                                   { return len; }
    std::string filename() const                               { return path.filename().c_str(); }
    std::filesystem::path file_path() const noexcept           { return path; }
};

// reads an entire file into a string, skipping any file object construction.
inline Result<std::string> read_file(std::filesystem::path path)
{
    FILE *file = fopen(path.c_str(), "rb");
    if (!file)
        return tl::unexpected(detail::make_error());
    fseek(file, 0l, SEEK_END);
    long size = ftell(file);
    rewind(file);
    std::string buf(size, ' ');
    size_t bytes_read = fread(buf.data(), sizeof(char), size, file);
    if (bytes_read < std::size_t(size))
        return tl::unexpected(detail::make_error());
    fclose(file);
    return buf;
}

inline std::filesystem::path user_home()
{
#ifdef PLATFORM_LINUX
    return getenv("HOME");
#endif
}

} // namespace io
