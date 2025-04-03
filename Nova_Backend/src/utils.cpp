#include "../include/utils.hpp"
#include <algorithm>

// Implementation of the trim function
std::string trim(const std::string& str) {
    auto first = str.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    auto last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}
