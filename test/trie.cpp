#include "Trie.hpp"

#include <iostream>

struct A {
    A(int x) : x(x) {}
    size_t raw_size() const { return 4; }

    friend std::ostream &operator<<(std::ostream &os, const A &a)
    { return os << "A(" << a.x << ")"; }

    int x;
};

int main()
{
    DrLD::Trie<A> x;
    x.put((const u8*)"test", 4, A(10), [](auto x){ return true; });
    std::cout << x.get((const u8*)"test", 4).value_or(A(0)) << std::endl;
    x.put((const u8*)"test1", 5, A(9), [](auto x){ return false; });
    std::cout << x.get((const u8*)"test", 4).value_or(A(0)) << std::endl;
    x.put((const u8*)"test", 4, A(8), [](auto x){ return true; });
    std::cout << x.get((const u8*)"test", 4).value_or(A(0)) << std::endl;
    std::cout << x.get((const u8*)"test1", 5).value_or(A(0)) << std::endl;
}
