#ifndef HELPER_H
#define HELPER_H

#include <bits/stdc++.h>

namespace tcs {

void ensure(bool cond, std::string const& message) {
	if(!cond) throw validation_error(message);
}

template <typename Iter>
bool unique(Iter begin, Iter end) {
	return std::distance(begin, end) == std::set<typename Iter::value_type>(begin, end).size();
}

template <typename Container>
bool unique(Container const& C) {
	return ::tcs::unique(C.begin(), C.end());
}

std::string arr_name(std::string name, int index) {
	return name + "[" + std::to_string(index) + "]";
}

template <typename T>
bool in_range(T const& left, T const& middle, T const& right) {
	return left <= middle && middle <= right; 
}

}

#endif /* end of include guard: HELPER_H */
