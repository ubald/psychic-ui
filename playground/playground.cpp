#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <array>
#include <psychic-ui/utils/StringUtils.hpp>

using namespace psychic_ui;

int main() {
    std::cout << string_utils::split("#allo", '#').size() << std::endl;
    std::cout << string_utils::split("coucou#allo", '#').size()  << std::endl;
    std::cout << string_utils::split("coucou#", '#').size()  << std::endl;
    std::cout << string_utils::split("pouet", '#').size()  << std::endl;
    std::cout << string_utils::split("#", '#').size()  << std::endl;
}
