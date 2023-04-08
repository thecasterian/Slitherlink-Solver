#ifndef SLITHERLINK_HPP
#define SLITHERLINK_HPP

#include <utility>
#include <string>
#include <vector>

namespace slink {

enum Region {
    UNDET = 0xF,
    INNER = 0x1,
    OUTER = 0x2,

    UNDET_BFS = 0xF0,
    INNER_BFS = 0x10,
    OUTER_BFS = 0x20,
};

enum Number {
    EMPTY = -1,
};

class Slitherlink {
public:
    Slitherlink(const std::vector<std::vector<int>> &grid);
    Slitherlink(const std::vector<std::string> &grid);

    bool solve(void);
    void print_solution(void);

private:
    bool solve_helper(const std::vector<std::vector<int>> &region);
    bool apply_heuristics(std::vector<std::vector<int>> &region);
    bool is_available_partial_solution(std::vector<std::vector<int>> &region);
    bool is_answer(std::vector<std::vector<int>> &region);
    std::pair<int, int> find_region(std::vector<std::vector<int>> &region, int val);
    void print_region(const std::vector<std::vector<int>> &region);

    const int nr, nc;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> region_solved;
};

}

#endif
