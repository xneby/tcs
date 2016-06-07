#ifndef TCS_TESTS_H
#define TCS_TESTS_H

#include <vector>

namespace tcs {

template <typename... Args>
std::tuple<Args...> G(Args... args) {
    return std::make_tuple(args...);
}

template <typename Gen>
struct with_seed_impl : tcs::abstract_generator<typename Gen::TestCase> {
    int seed;
    Gen generator;

    with_seed_impl(int _seed, Gen _generator) : seed(_seed), generator(_generator) {}

    typename Gen::TestCase call(int _seed) {
        return generator.call(seed);
    }
};

template <typename Gen>
with_seed_impl<Gen> with_seed(int seed, Gen gen) {
    return {seed, gen};
}

template <int idx, typename ...Args>
typename std::enable_if<idx == std::tuple_size<std::tuple<Args...>>::value>::type
run_group(std::string const&, int seed, std::tuple<Args...> tests) {}

template <int idx, typename ...Args>
typename std::enable_if<(idx < std::tuple_size<std::tuple<Args...>>::value)>::type
run_group(std::string const& name, int seed, std::tuple<Args...> tests) {
    run_test(name, seed * 31 + idx, seed, idx, std::get<idx>(tests));
    run_group<idx+1>(name, seed, tests);
}

template <int idx, typename TestCase>
void run_group(std::string const& name, int seed, tcs::abstract_generator<TestCase> & gen) {
    run_test(name, seed * 31, seed, -1, gen);
}

std::string makeFileName(std::string const& name, int group, int test) {
    std::string main;
    if(test == -1) {
        main = std::to_string(group + 1);
    } else {
        main = std::to_string(group + 1) + std::string(1, 'a' + test);
    }

    return name + main + ".in";
}

template <typename TestCase>
void run_test(std::string const& name, int seed, int group, int test, tcs::abstract_generator<TestCase> & gen) {
    std::ofstream out (makeFileName(name, group, test));
    out << gen.call(seed);
}

template <int idx, typename ...Args>
typename std::enable_if<idx == std::tuple_size<std::tuple<Args...>>::value>::type
run_groups(std::string const& name, std::tuple<Args...> groups) {}

template <int idx, typename ...Args>
typename std::enable_if<(idx < std::tuple_size<std::tuple<Args...>>::value)>::type
run_groups(std::string const& name, std::tuple<Args...> groups) {
    run_group<0>(name, idx, std::get<idx>(groups));
    run_groups<idx + 1>(name, groups);
}

template <typename ...Args>
void run_generator(std::function<std::tuple<Args...>()> generator, std::string const& short_name) {
    auto t = generator();

    run_groups<0>(short_name, t);
}
template <typename ...Args>
void run_generator(std::tuple<Args...> (*generator)(), std::string const& short_name) {
    run_generator(std::function<std::tuple<Args...>()>(generator), short_name);
}

};

#endif /* end of include guard: TCS_TESTS_H */
