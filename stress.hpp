#ifndef STRESS_H
#define STRESS_H

#include "errors.hpp"
#include "checker_result.hpp"
#include <limits>
#include <iostream>
#include <fstream>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <iomanip>

namespace tcs {

struct StressStats {
    int threads;
    std::mutex & output_mutex;

    struct Row {
        int id;
        int test_number;
        std::vector<std::chrono::steady_clock::duration> ticks;
        std::chrono::steady_clock::time_point last_measure;
        using my_seconds = std::chrono::duration<double>;

        Row(int _id): id(_id) {}

        void reset(int test) {
            test_number = test;
            ticks.clear();
            measure();
        }

        void measure() {
            last_measure = std::chrono::steady_clock::now();
        }

        void tick() {
            ticks.push_back(std::chrono::steady_clock::now() - last_measure);
        }

        void output0() const {
            std::cout << "T"
                 << std::setw(2) << std::left << id
                 << ": "
                 << std::setw(4) << std::right << test_number;
            for(auto const& interval : ticks) {
                std::cout << " " << std::setw(5) << std::right << std::fixed << std::setprecision(2) << my_seconds(interval).count() << "s";
            }
        }

        void output() const {
            std::cout << "\033[K";
            output0();
            std::cout << std::endl;
        }

        void output_outcome(bool outcome) const {
            output0();
            std::cout << (outcome ? " \033[32mOK\033[0m" : " \033[31mWA\033[0m") << std::endl;
        }
    };

    std::vector<Row> rows;

    StressStats(int threads, std::mutex & mutex): threads(threads), output_mutex(mutex) {
        for(int i = 0; i < threads; i++)
            rows.emplace_back(i);
    }

    void start(int id, int test_id) {
        rows[id].reset(test_id);
        print();
    }

    void print0() {
        std::cout << std::string(40, '-') << std::endl;
        for(auto const& r : rows) r.output();
        std::cout << "\033[" << (1+threads) << "A";
    }

    void print() {
        std::lock_guard<std::mutex> _lock(output_mutex);

        print0();
    }

    void tick(int id) {
        rows[id].tick();
        print();
        rows[id].measure();
    }

    void finish(int id, bool outcome) {
        std::lock_guard<std::mutex> _lock(output_mutex);
        rows[id].tick();
        rows[id].output_outcome(outcome);
        print0();
    }
};

template <typename ConfigClass>
struct Stress {
    ConfigClass const& config;

    std::mutex next_seed_mutex;
    std::mutex output_mutex;
    std::vector<std::pair<int, CheckerResult>> counters;
    int next_seed;
    bool finished;

    StressStats stats;

    Stress(ConfigClass const& config): config(config), next_seed(0), finished(false), stats(config.threads, output_mutex) {}

    int get_next_test() {
        std::lock_guard<std::mutex> _lock{next_seed_mutex};
        if(next_seed >= config.test_count || finished) return -1;
        return next_seed ++;
    }

    void counter(int seed, CheckerResult const& cr) {
        std::lock_guard<std::mutex> _lock2(next_seed_mutex);
        finished = true;
        counters.emplace_back(seed, cr);
    }

    void print_counter(int seed, CheckerResult const& cr) {
        std::cout << std::string(40, '-') << std::endl;
        std::cout << "Seed: " << seed << "; Checker output: ";
        std::cout << cr;
    }

    template <typename T>
    static void write_file(int test, std::string extension, T const& obj) {
        std::ofstream file(
            "stress-" + std::to_string(test) + "." + extension
        );
        file << obj;
    }

    static void worker(int id, Stress & stress) {
        int test;
        auto generator = stress.config.generator;
        while((test = stress.get_next_test()) != -1) {
            stress.stats.start(id, test);

            auto tc = generator(test);
            stress.stats.tick(id);
            auto model = stress.config.model(tc);
            stress.stats.tick(id);
            auto stressed = stress.config.stressed(tc);
            stress.stats.tick(id);
            CheckerResult checker = stress.config.checker(tc, model, stressed);
            stress.stats.finish(id, checker.correct);
            if(!checker.correct) {
                write_file(test, "in", tc);
                write_file(test, "model", model);
                write_file(test, "stressed", stressed);
                stress.counter(test, checker);
            }
        }
    }

    void run() {
        std::vector<std::thread> threads;

        for(int i = 0; i < config.threads; i++) {
            threads.emplace_back(Stress::worker, i, std::ref(*this));
        }

        for(auto & thread: threads) {
            thread.join();
        }

        std::cout << "\033[J";

        if(counters.empty()) {
            std::cout << std::endl << "Stress finished, didn't found counterexamples." << std::endl;
        } else {
            std::cout << std::endl << "Stress finished, did found some counterexamples." << std::endl;
        }

        for(auto & c : counters) {
            print_counter(c.first, c.second);
        }

    }
};

template <typename Answer, typename TestCase>
struct StressConfig {
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
    StressConfig& setThreads() { return setThreads(std::max(1u, std::thread::hardware_concurrency() - 1)); }

    void run() const {
        Stress<StressConfig<Answer, TestCase>>{*this}.run();
    }
};

template <typename Answer, typename TestCase>
StressConfig<Answer, TestCase> configure_stress(
    typename StressConfig<Answer, TestCase>::Solution model,
    typename StressConfig<Answer, TestCase>::Solution stressed,
    typename StressConfig<Answer, TestCase>::Generator generator,
    typename StressConfig<Answer, TestCase>::Checker checker
) {
    return StressConfig<Answer, TestCase>{}.setModel(model).setStressed(stressed).
        setGenerator(generator).setChecker(checker);
}

template <typename Answer, typename TestCase, typename GeneratorClass>
StressConfig<Answer, TestCase> configure_stress(
    Answer (*model)(TestCase const&),
    Answer (*stressed)(TestCase const&),
    GeneratorClass && generator,
    CheckerResult (*checker)(TestCase const&, Answer const&, Answer const&)
) {
    auto generator_lambda = [generator](int seed) {auto copy = generator; return copy.call(seed); };
    return configure_stress<Answer, TestCase>(
        std::function<Answer(TestCase const&)>{model},
        std::function<Answer(TestCase const&)>{stressed},
        std::function<TestCase(int)>{generator_lambda},
        std::function<CheckerResult(TestCase const&, Answer const&, Answer const&)>{checker}
    );
}

}
#endif /* end of include guard: STRESS_H */
