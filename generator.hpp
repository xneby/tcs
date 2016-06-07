#ifndef TCS_GENERATOR_H
#define TCS_GENERATOR_H

#include <random>
#include <algorithm>

namespace tcs {

template <typename TestCase>
struct abstract_generator {
    virtual TestCase call(int) = 0;
};

template <typename TestCase>
struct generator : abstract_generator<TestCase> {
    std::mt19937 rand;

    void srand(int seed) { rand.seed(seed); }
    int rand_int(int a, int b) {
        std::uniform_int_distribution<int> dist(a, b);
        return dist(rand);
    }
    bool rand_bool() {
        return rand_int(0, 1) == 1;
    }
    template <typename Iter>
    void shuffle(Iter first, Iter last) {
        std::shuffle(first, last, rand);
    }
};



template <typename Impl>
struct MakeGenerator : Impl {
    template <typename R, typename... Args>
    static R _cast(R (Impl::*) (Args...)) {}

    using TestCase = decltype(MakeGenerator::_cast(&Impl::generate));
    std::function<TestCase(MakeGenerator &)> generate_f;

    template <typename... Args>
    MakeGenerator(Args... _args): generate_f(std::bind(std::mem_fn(&Impl::generate), std::placeholders::_1, _args...)) {}

    TestCase call(int seed) {
        this->srand(seed);
        return generate_f(*this);
    }
};

}

#endif /* end of include guard: TCS_GENERATOR_H */
