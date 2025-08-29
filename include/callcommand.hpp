#pragma once

/*
 * A library for command parsing in a gdb-style command line application.
 *
 * How to use:
 *
 * call_command is the main function to use. First and second parameters are
 * the command and its arguments taken from the user or from some other source.
 * (A secondary version of call_command takes these two parameters and puts
 * them together as the first parameter, with the name being args[0])
 * Third parameter is a function that creates an error string. The function
 * should have this signature:
 *
 * std::string parse_error_message(int which, std::string_view name, int num_params)
 *
 * 'which' tells us the type of error: 0 for an invalid command, 1 for a valid
 * command with a wrong number of parameters.
 *
 * The next parameters are declarations of valid command. For example, if we
 * have two commands named 'sum' and 'mul', each taking two parameters, we
 * would declare them like so:
 *
 * void sum_numbers(int, int);
 * void multiply_numbers(int, int);
 *
 * Command{ "sum", "s", sum_numbers }
 * Command{ "mul", "m", multiply_numbers }
 *
 * Note that 's' and 'm' are abbreviations for the commands. Always make sure
 * names and abbreviations are unique, or else the function gets confused.
 * We can also use class methods or lambdas instead of function pointers. When
 * using lambdas, due to weird C++ limitations, we must also declare the
 * types the Command should get as parameters, like so:
 *
 * Command<int, int>{ "sum", "s", [](int a, int b) { std::print("{}\n", a+b); } }
 *
 * While trying to find a match, call_command will call a function,
 * try_convert_impl, which will try to convert an argument to a new type.
 * For example, the two commands above need this following definition of
 * try_convert_impl:
 *
 * template <>
 * int util::try_convert_impl<int>(std::string_view str)
 * {
 *     if (convert_to_int(str))
 *         return the converted number
 *     throw ParseError("invalid number")
 * }
 *
 * Throwing a ParseError is required when the conversion fails. Note also
 * that this library provides no definition for any type; each type you use
 * for the Command's must also have an appropriate try_convert_impl definition.
 */

#include <functional>
#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <span>

namespace util {

struct ParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
    using std::runtime_error::what;
};

template <typename... P>
struct Command {
    std::string name;
    std::string abbrev;
    std::function<void(P...)> fn;
    Command(std::string_view n, std::string_view a, std::function<void(P...)> f) : name(n), abbrev(a), fn(f) { }
    Command(std::string_view n, std::string_view a, void (*f)(P...))             : name(n), abbrev(a), fn(f) { }
    Command(std::string_view n, std::string_view a, auto &&f)                    : name(n), abbrev(a), fn(f) { }
    template <typename T>
    Command(std::string_view n, std::string_view a, void (T::*f)(P...), T *t)    : name(n), abbrev(a)
    {
        fn = [t, f](P&&... p) { (t->*f)(p...); };
    }
};

template <typename T> T try_convert_impl(std::string_view str);

namespace detail {

template<int N, typename... Ts>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;

template <typename... P>
auto call_one(std::string_view name, std::span<std::string> args, bool &wrong_num_params, const Command<P...> &cmd)
{
    auto call_forward = [&]<std::size_t... Is>(std::index_sequence<Is...> const&) {
        cmd.fn( try_convert_impl<NthTypeOf<Is, P...>>(std::string_view(args[Is]))... );
    };

    if (cmd.name != name && cmd.abbrev != name)
        return false;
    if (args.size() != sizeof...(P)) {
        wrong_num_params = true;
        return false;
    }
    call_forward(std::index_sequence_for<P...>());
    return true;
};

} // namespace detail

void call_command(std::string_view name, std::span<std::string> args, auto &&error_msg, auto&&... commands)
{
    bool wrong_num_params = false;
    if (!(detail::call_one(name, args, wrong_num_params, commands) || ...))
        throw ParseError(error_msg(wrong_num_params ? 1 : 0, name, args.size()));
}

void call_command(std::span<std::string> args, auto &&error_msg, auto&&... commands)
{
    call_command(args[0], args.subspan(1, args.size()-1), error_msg, commands...);
}

} // namespace util
