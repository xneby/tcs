#ifndef STRESS_H
#define STRESS_H

#include "errors.hpp"
#include "checker_result.hpp"
#include <limits>
#include <iostream>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

namespace tcs {

template <typename Answer, typename TestCase>
class StressConfig {
    using Solution = ::std::function<Answer(TestCase const&)>;
    using Generator = ::std::function<TestCase(int)>;
    using Checker = ::std::function<CheckerResult(TestCase const&, Answer const&, Answer const&)>;
    Solution model;
    Solution stressed;
    Generator generator;
    Checker checker;
    int threads = 1;
    int test_count = ::std::numeric_limits<int>::max();
    bool print_tests = false;

public:
    bool validate() const {
        if(model == nullptr) throw improperly_configured("missing setModel()");
        if(stressed == nullptr) throw improperly_configured("missing setStressed()");
        if(generator == nullptr) throw improperly_configured("missing setGenerator()");
        if(checker == nullptr) throw improperly_configured("missing setChecker()");

        if(test_count < 0) throw improperly_configured("negative number of tests");

        return true;
    }

    StressConfig() {}

    StressConfig& setModel(Solution model) { this->model = model; return *this; }
    StressConfig& setStressed(Solution stressed) { this->stressed = stressed; return *this; }
    StressConfig& setGenerator(Generator generator) { this->generator = generator; return *this; }
    StressConfig& setChecker(Checker checker) { this->checker = checker; return *this; }
    StressConfig& setTestsCount(int count) { this->test_count = count; return *this; }
    StressConfig& printTests(bool value = true) { this->print_tests = value; return *this; }
    StressConfig& setThreads(int threads) { this->threads = threads; return *this; }
    StressConfig& setThreads() { return setThreads(std::thread::hardware_concurrency() - 1); }

};


}
#endif /* end of include guard: STRESS_H */
