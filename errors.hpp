#ifndef ERRORS_H
#define ERRORS_H


#include <stdexcept>

namespace tcs {
struct validation_error : public ::std::runtime_error {
	validation_error(std::string const& msg): runtime_error(msg) {}
};

struct improperly_configured : public ::std::runtime_error {
    improperly_configured(std::string const& msg): runtime_error(msg) {}
};

}

#endif /* end of include guard: ERRORS_H */
