#include "function.hpp"
#include <iostream>

template<typename T>
T square(const T& n)
{
    return n * n;
}

template<typename T1, typename T2>
struct add
{
    auto operator()(const T1& lhs, const T2& rhs)
    {
        return lhs + rhs;
    }
};

struct value
{
    double val = 3.14;
};

int main()
{
    // lambda
    Function<int(int)> mult = [](int n) { return 2 * n; };
    std::cout << "2 * 2 = " << mult(2) << "\n";

    // lambda2
    mult = [k = 1](int n) mutable { return k++ * n; };
    std::cout << "1 * 3 = " << mult(3) << "\n";
    std::cout << "2 * 3 = " << mult(3) << "\n";

    // function
    Function<double(double)> sq = square<double>;
    std::cout << "3 * 3 = " << sq(3) << "\n";

    // functor
    Function<double(double, int)> adder = add<double, double>();
    std::cout << "4 + 7 = " << adder(4.0, 7) << "\n";
     
    // member ptr
    Function<double(value*)> getter = &value::val;
    value val;
    std::cout << "value: " << getter(&val) << "\n";
}
