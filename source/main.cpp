#include <iostream>
#include "slitherlink.hpp"

int main(void) {
    std::string s;
    std::vector<std::string> grid;

    while (std::getline(std::cin, s)) {
        grid.push_back(s);
    }

    slink::Slitherlink sl(grid);

    if (sl.solve()) {
        sl.print_solution();
    } else {
        std::cout << "No solution" << std::endl;
    }

    return 0;
}