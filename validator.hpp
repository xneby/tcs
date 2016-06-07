#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "helper.hpp"
#include <string>

namespace tcs {

template <typename TestCase>
int run_validator() {
    TestCase tc;
    try {
        std::cin >> std::noskipws >>= tc;
    } catch (validation_error & ex) {
        std::cerr << "error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cerr << "ok " << tc.stats() << std::endl;
    return EXIT_SUCCESS;
}

template <typename Type, typename = std::enable_if<std::is_integral<Type>::value>>
struct number {
    Type &target;
    Type low, high;
    std::string name;

    explicit number(Type &_target, Type _low, Type _high, std::string const& _name) :
        target(_target),
        low(_low),
        high(_high),
        name(_name) {}

    explicit number(Type &_target, Type _low, Type _high) : number(_target, _low, _high, "") {}

    std::string getName() const {
        if(name == "") return "<unnamed>";
        return name;
    }

    friend std::istream & operator >> (std::istream & in, number && num) {
        in >> num.target;
        if(!in) throw validation_error {"Syntax error while reading " + num.getName()};
        if(num.low > num.target || num.high < num.target)
            throw validation_error {"Value error: " + num.getName() + "=" + std::to_string(num.target) + " is not in range [" + std::to_string(num.low) +", " + std::to_string(num.high) + "]"};
        return in;
    }
};

struct expect {
    char value;

    explicit expect(char _value): value(_value) {}

    static std::string repr(char c) {
        if(c == ' ') return "SPACE";
        if(c == '\n') return "NEWLINE";
        if(c < 32) return "#" + std::to_string(+c);
        return "'" + std::to_string(c) + "'";
    }

    friend std::istream & operator >> (std::istream & in, expect const& en) {
        char tmp;
        in >> tmp;
        if(!in) throw validation_error {"Failed to read char " + repr(en.value)};
        if(tmp != en.value) throw validation_error {"Read " + repr(tmp) + " but expected " + repr(en.value)};
        return in;
    }
};

expect space(' ');
expect nl('\n');

struct oneof {
    char & target;
    std::string possibilities;
    std::string name;
    
    explicit oneof(char & _target, std::string _possibilities): oneof(_target, _possibilities, create_name(_possibilities)) {}
    explicit oneof(char & _target, std::string _possibilities, std::string _name): target(_target), possibilities(_possibilities), name(_name) {}
    
    static std::string create_name(std::string possibilities) {
        std::ostringstream ss;
        
        ss << "one of";
        for(auto c: possibilities) 
            ss << " " << expect::repr(c);
        
        return ss.str();
    }
    
    friend std::istream & operator >> (std::istream & in, oneof && one) {
        in >> one.target;
        if(!in) throw validation_error {"Failed to read " + one.name };
        if(one.possibilities.find(one.target) == std::string::npos)
            throw validation_error {"While reading " + one.name + ": read " + expect::repr(one.target) + " but "
            "expected " + create_name(one.possibilities)};
        return in;
    }   
};

using integer = number<int>;
using uinteger = number<unsigned>;
using longlong = number<long long>;

std::istream& eof(std::istream& in) {
    char c;
    in >> c;
    if(!in.eof()) throw validation_error {"File didn't end where expected."};
    return in;
}

}

#endif /* end of include guard: VALIDATOR_H */
