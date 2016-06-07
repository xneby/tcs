#ifndef SOLUTION_H
#define SOLUTION_H

#include <functional>

namespace tcs {

template <typename Answer, typename TestCase>
void run_solution(std::function<Answer(TestCase const&)> solution) {
	TestCase tc;
	std::cin >> tc;
	std::cout << solution(tc);
}

template <typename Answer, typename TestCase>
void run_solution(Answer (*solution) (TestCase const&)) {
    run_solution(std::function<Answer(TestCase const&)>(solution));
}

}

#endif /* end of include guard: SOLUTION_H */
