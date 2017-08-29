#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

inline std::vector<std::string> split(std::string text, char delimiter, bool removeEmpties = true) {
    std::vector<std::string> result;
    std::stringstream ss;
    ss.str(text);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!removeEmpties || !token.empty()) {
            result.push_back(token);
        }
    }
    return result;
}