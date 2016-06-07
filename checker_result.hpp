#ifndef CHECKER_RESULT_H
#define CHECKER_RESULT_H

#include <string>
#include <iostream>

namespace tcs {
struct CheckerResult {
	bool correct;
	std::string message;
	int points;

private:
	CheckerResult(bool _correct, std::string const& _message, int _points):
		correct(_correct), message(_message), points(_points) {}

public:
	static CheckerResult OK(std::string const& message="", int points=100) {
		return {true, message, points};
	}

	static CheckerResult WA(std::string const& message) {
		return {false, message, 0};
	}

	static CheckerResult LE(std::string const& message) {
		return {false, "LOGIC ERROR [!]: " + message, 0};
	}

	friend std::ostream& operator << (std::ostream& out, CheckerResult const& chk) {
		return out << (chk.correct ? "OK" : "WRONG") << std::endl <<
		              chk.message << std::endl <<
					  chk.points << std::endl;
	}
};
}
#endif /* end of include guard: CHECKER_RESULT_H */
