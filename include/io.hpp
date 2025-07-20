/*
 * This is a library that provides File classes. It is meant to be a thin
 * wrapper over stdio.h, as well replacing iostreams.
 *
 * There are two file classes provided: a File class, which is a simple wrapper
 * over a FILE *, and a MappedFile class, which is a wrapper over an memory
 * mapped file.
 *
 * There is also some additional stuff, such as a function that directly reads
 * a file and returns a string, and functions for getting standard paths.
 *
 * More specific documentation can be found below.
 */

#pragma once

// defines io::Access and io::File
#include "io-file.hpp"

namespace io {

namespace detail {
    auto open_mapped_file(std::filesystem::path path, Access access)
        -> std::expected<std::pair<u8 *, std::size_t>, std::error_code>;
    int close_mapped_file(u8 *ptr, std::size_t len);
} // namespace detail

/*
 * This is a RAII-style wrapper over a memory mapped file. It is worth
 * mentioning that, just like an std::unique_ptr, a MappedFile object is only
 * movable.
 * Because the data underneath is essentially an array, the class also provides
 * a small iterator inteface. Only the methods not belonging to this interface
 * will be described.
 *
 * @open: a static method for opening MappedFiles, same as File::open;
 * @slice: returns a slice, i.e. a part of the file's contents;
 * @filename and @file_path: return, respectively, the file's name and file's
 *                           path, just like in File;
 */
class MappedFile {
    u8 *ptr = nullptr;
    std::size_t len = 0;
    std::filesystem::path filepath;

    MappedFile(u8 *p, std::size_t s, std::filesystem::path pa)
        : ptr{p}, len{s}, filepath{pa}
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
        std::swap(filepath, m.filepath);
        return *this;
    }

    static std::expected<MappedFile, std::error_code> open(std::filesystem::path path, Access access)
    {
        auto v = detail::open_mapped_file(path, access);
        if (!v)
            return std::unexpected(v.error());
        return MappedFile(v.value().first, v.value().second, path);
    }

    int close() { auto r = detail::close_mapped_file(ptr, len); ptr = nullptr; len = 0; return r; }

    using value_type      = u8;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       =       value_type &;
    using const_reference = const value_type &;
    using pointer         =       value_type *;
    using const_pointer   = const value_type *;
    using iterator        =       value_type *;
    using const_iterator  = const value_type *;

    reference               operator[](size_type pos)                               { return ptr[pos]; }
    const_reference         operator[](size_type pos)                const          { return ptr[pos]; }
    u8 *                    data()                                         noexcept { return ptr; }
    const u8 *              data()                                   const noexcept { return ptr; }
    iterator                begin()                                        noexcept { return ptr; }
    const_iterator          begin()                                  const noexcept { return ptr; }
    iterator                end()                                          noexcept { return ptr + len; }
    const_iterator          end()                                    const noexcept { return ptr + len; }
    size_type               size()                                   const noexcept { return len; }
    std::span<u8>           slice(size_type start, size_type length)                { return { ptr + start, length}; }
    std::span<const u8>     slice(size_type start, size_type length) const          { return { ptr + start, length}; }
    std::span<u8>           bytes()                                                 { return { ptr, len }; }
    std::span<const u8>     bytes()                                  const          { return { ptr, len }; }
    std::string             name()                                   const noexcept { return filepath.filename().string(); }
    std::filesystem::path   path()                                   const noexcept { return filepath; }
};

namespace directory {

// The user's home directory (~ on linux, C:\Users\<user> on Windows).
std::filesystem::path home();
// The user's config directory (.config on linux).
std::filesystem::path config();
// The user's data directory (.local/share on linux).
std::filesystem::path data();
// The user's applications directory.
std::filesystem::path applications();

} // namespace directory

} // namespace io
