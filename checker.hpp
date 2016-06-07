#ifndef CHECKER_WRAPPER_H
#define CHECKER_WRAPPER_H

#include <fstream>
#include "checker_result.hpp"

namespace tcs {

template <typename Answer, typename TestCase>
int run_checker(std::function<tcs::CheckerResult(TestCase const&, Answer const&, Answer const&)> check, int argc, char** argv) {
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << " in out wzo" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream fin (argv[1]);
    std::ifstream fout (argv[2]);
    std::ifstream fwzo (argv[3]);
    if(!fin || !fout || !fwzo) {
        std::cerr << "Unable to open file." << std::endl;
        return EXIT_FAILURE;
    }

    TestCase tc;
    fin >> tc;

    Answer azaw, awzo;
    awzo.read(fwzo, tc);
    azaw.read(fout, tc);
    
    bool flag = !!fout;
    char c;
    fout >> c;
    flag &= fout.eof();
    if(!flag) {
        std::cout << tcs::CheckerResult::WA("Syntax error");
        return EXIT_SUCCESS;
    }
    
    auto result = check(tc, awzo, azaw);
    std::cout << result;
    return EXIT_SUCCESS;
}

template <typename Answer, typename TestCase>
int run_checker(tcs::CheckerResult (*check) (TestCase const&, Answer const&, Answer const&), int argc, char** argv) {
    return run_checker(std::function<tcs::CheckerResult(TestCase const&, Answer const&, Answer const&)>(check), argc, argv);
}

}

#endif /* end of include guard: CHECKER_WRAPPER_H */
