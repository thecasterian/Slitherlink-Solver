#ifndef SLITHERLINK_HPP
#define SLITHERLINK_HPP

#include <utility>
#include <string>
#include <vector>

namespace slink {

enum class Region {
    UNDET,
    INNER,
    OUTER,

    UNDET_BFS,
    INNER_BFS,
    OUTER_BFS,
};

enum class Number {
    EMPTY = -1,
    ZERO = 0,
    ONE = 1,
    TWO = 2,
    THREE = 3,
};

class Slitherlink {
public:
    Slitherlink(const std::vector<std::vector<int>> &grid);
    Slitherlink(const std::vector<std::string> &grid);

    bool solve(void);
    void print_solution(void);

private:
    bool solve_helper(const std::vector<std::vector<Region>> &region);
    bool apply_heuristics(std::vector<std::vector<Region>> &region);
    bool is_available_partial_solution(std::vector<std::vector<Region>> &region);
    bool is_answer(std::vector<std::vector<Region>> &region);
    std::pair<int, int> find_region(std::vector<std::vector<Region>> &region, Region val);
    void print_region(const std::vector<std::vector<Region>> &region);

    const int nr, nc;
    std::vector<std::vector<Number>> grid;
    std::vector<std::vector<Region>> region_solved;
};

}

#endif
