#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <array>
#include <unicode/unistr.h>
#include <cmath>

int main() {
    std::string s1{"Brébœuf"};
    std::cout << s1.size() << " " << strlen(s1.c_str()) << std::endl;
}
