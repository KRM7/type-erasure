#include "any.hpp"
#include <string>
#include <cassert>

using namespace any;
using namespace std::string_literals;

int main()
{
    // ctor
    {
        Any any = 3;

        assert(any.type().name() == typeid(int).name());
        assert(any.get<int>() == 3);
    }

    // cpy ctor
    {
        Any any1 = "Hello"s;
        Any any2 = any1;

        assert(any2.type().name() == typeid(std::string).name());
        assert(any1.type().name() == any2.type().name());
        assert(any1.get<std::string>() == any2.get<std::string>());
    }

    // cpy assign
    {
        Any any1 = "Hello"s;
        Any any2 = 3;

        any2 = any1;

        assert(any2.type().name() == typeid(std::string).name());
        assert(any1.type().name() == typeid(std::string).name());
        assert(any2.get<std::string>() == "Hello"s);
    }

    // move assign
    {
        Any any = 3.14;
        any = Any("Hello"s);

        assert(any.type().name() == typeid(std::string).name());
        assert(any.get<std::string>() == "Hello"s);
    }

    // move ctor
    {
        Any any1 = 3.14;

        Any any2 = std::move(any1);

        assert(any2.type().name() == typeid(double).name());
        assert(any2.get<double>() == 3.14);
    }

    // reset + has value
    {
        Any any = 3.14;
        assert(any.has_value());

        any.reset();

        assert(!any.has_value());
        assert(any.type().name() == typeid(void).name());
    }

    // swap
    {
        Any any1 = "Hello"s;
        Any any2 = 3U;

        any1.swap(any2);

        assert(any1.type().name() == typeid(unsigned).name());
        assert(any1.get<unsigned>() == 3U);
        assert(any2.type().name() == typeid(std::string).name());
        assert(any2.get<std::string>() == "Hello"s);
    }

    // array
    {
        //Any any = "Hello";
    }
}