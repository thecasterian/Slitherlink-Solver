#include "slitherlink.hpp"

#include <iostream>
#include <queue>
#include <utility>

using namespace slink;

#define FOR_CELL                           \
    for (int i = 1; i < this->nr + 1; ++i) \
        for (int j = 1; j < this->nc + 1; ++j)

#define FOR_ADJ                 \
    for (int k = 0; k < 4; k++) \
        if (0 <= i + adj[k][0] && i + adj[k][0] < this->nr + 2 && 0 <= j + adj[k][1] && j + adj[k][1] < this->nc + 2)

#define ADJ_REG (region[i + adj[k][0]][j + adj[k][1]])

#define ADJD_REG(IDX) (region[i + adjd[(IDX) % 8][0]][j + adjd[(IDX) % 8][1]])

#define UPDATE(r, nr)                          \
    do {                                       \
        bool _updated;                         \
        if (!update_region(r, nr, _updated)) { \
            return false;                      \
        }                                      \
        if (_updated) {                        \
            changed = true;                    \
        }                                      \
    } while (0)

#define UPDATE_SAME(r1, r2)                                                           \
    do {                                                                              \
        bool _updated1, _updated2;                                                    \
        if (!update_region(r1, r2, _updated1) || !update_region(r2, r1, _updated2)) { \
            return false;                                                             \
        }                                                                             \
        if (_updated1 || _updated2) {                                                 \
            changed = true;                                                           \
        }                                                                             \
    } while (0)

#define UPDATE_DIFF(r1, r2)                                                                                   \
    do {                                                                                                      \
        bool _updated1, _updated2;                                                                            \
        if (!update_region(r1, inv_region(r2), _updated1) || !update_region(r2, inv_region(r1), _updated2)) { \
            return false;                                                                                     \
        }                                                                                                     \
        if (_updated1 || _updated2) {                                                                         \
            changed = true;                                                                                   \
        }                                                                                                     \
    } while (0)

/* Relative indices of adjacent cells in clockwise order. */
static constexpr int adj[4][2] = {
    {-1, 0},
    {0, 1},
    {1, 0},
    {0, -1},
};

/* Relative indices of adjacent and diagonally adjacent cells in clockwise order. */
static constexpr int adjd[8][2] = {
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1},
};

static Region inv_region(Region r);
static bool is_same_region(Region r1, Region r2);
static bool is_diff_region(Region r1, Region r2);
static bool update_region(Region &r, Region nr, bool &updated);

Slitherlink::Slitherlink(const std::vector<std::vector<int>> &grid)
    : nr(grid.size()), nc(grid[0].size()), grid(nr + 2, std::vector<Number>(nc + 2, Number::EMPTY)) {
    FOR_CELL {
        this->grid[i][j] = static_cast<Number>(grid[i - 1][j - 1]);
    }
}

Slitherlink::Slitherlink(const std::vector<std::string> &grid)
    : nr(grid.size()), nc(grid[0].size()), grid(nr + 2, std::vector<Number>(nc + 2, Number::EMPTY)) {
    FOR_CELL {
        this->grid[i][j] = isdigit(grid[i - 1][j - 1]) ? static_cast<Number>(grid[i - 1][j - 1] - '0') : Number::EMPTY;
    }
}

bool Slitherlink::solve(void) {
    std::vector<std::vector<Region>> region(this->nr + 2, std::vector<Region>(this->nc + 2, Region::UNDET));
    for (int i = 0; i < this->nr + 2; ++i) {
        region[i][0] = region[i][this->nc + 1] = Region::OUTER;
    }
    for (int j = 1; j < this->nc + 1; ++j) {
        region[0][j] = region[this->nr + 1][j] = Region::OUTER;
    }

    return this->solve_helper(region);
}

void Slitherlink::print_solution(void) {
    std::vector<std::string> buf(2 * this->nr + 3, std::string(4 * this->nc + 5, ' '));

    for (int i = 1; i < this->nr + 1; ++i) {
        for (int j = 1; j < this->nc + 1; ++j) {
            buf[2 * i][4 * j] = this->grid[i][j] != Number::EMPTY ? static_cast<char>(this->grid[i][j]) + '0' : ' ';
        }
    }

    for (int i = 0; i < this->nr + 1; ++i) {
        for (int j = 1; j < this->nc + 1; ++j) {
            if (is_diff_region(this->region_solved[i][j], this->region_solved[i + 1][j])) {
                buf[2 * i + 1][4 * j - 1] = buf[2 * i + 1][4 * j] = buf[2 * i + 1][4 * j + 1] = '-';
            }
        }
    }
    for (int i = 1; i < this->nr + 1; ++i) {
        for (int j = 0; j < this->nc + 1; ++j) {
            if (is_diff_region(this->region_solved[i][j], this->region_solved[i][j + 1])) {
                buf[2 * i][4 * j + 2] = '|';
            }
        }
    }

    for (int i = 0; i < this->nr + 1; ++i) {
        for (int j = 0; j < this->nc + 1; ++j) {
            int cnt_h = 0, cnt_v = 0;
            if (buf[2 * i + 1][4 * j + 1] == '-') {
                ++cnt_h;
            }
            if (buf[2 * i + 1][4 * j + 3] == '-') {
                ++cnt_h;
            }
            if (buf[2 * i][4 * j + 2] == '|') {
                ++cnt_v;
            }
            if (buf[2 * i + 2][4 * j + 2] == '|') {
                ++cnt_v;
            }

            if (cnt_h == 2) {
                buf[2 * i + 1][4 * j + 2] = '-';
            } else if (cnt_v == 2) {
                buf[2 * i + 1][4 * j + 2] = '|';
            } else if (cnt_h == 1 && cnt_v == 1) {
                buf[2 * i + 1][4 * j + 2] = '+';
            } else {
                buf[2 * i + 1][4 * j + 2] = '.';
            }
        }
    }

    for (int i = 0; i < 2 * this->nr + 3; ++i) {
        std::cout << buf[i] << '\n';
    }
}

bool Slitherlink::solve_helper(const std::vector<std::vector<Region>> &region) {
    std::vector<std::vector<Region>> new_region = region;
    if (!this->apply_heuristics(new_region)) {
        return false;
    }
    if (this->is_answer(new_region)) {
        this->region_solved = new_region;
        return true;
    }

    /* Find the first undetermined region. */
    auto [i0, j0] = this->find_region(new_region, Region::UNDET);
    /* No undetermined region. */
    if (i0 == -1 && j0 == -1) {
        return false;
    }

    /* Try to fill the region with OUTER. */
    new_region[i0][j0] = Region::OUTER;
    if (this->solve_helper(new_region)) {
        return true;
    }
    /* Try to fill the region with INNER. */
    new_region[i0][j0] = Region::INNER;
    if (this->solve_helper(new_region)) {
        return true;
    }
    return false;
}

bool Slitherlink::apply_heuristics(std::vector<std::vector<Region>> &region) {
    if (!this->is_available_partial_solution(region)) {
        return false;
    }

    bool changed = true;
    while (changed) {
        changed = false;

        FOR_CELL {
            /* For 0, its adjacent cell must have the same region. */
            if (this->grid[i][j] == Number::ZERO) {
                FOR_ADJ {
                    UPDATE_SAME(region[i][j], ADJ_REG);
                }
            }

            if (this->grid[i][j] != Number::EMPTY && region[i][j] != Region::UNDET) {
                /* For n > 0, if its region is determined and have n adjacent cells with the different region, then
                    other adjacent regions must have the same region. */
                int cnt = 0;
                FOR_ADJ {
                    if (is_diff_region(ADJ_REG, region[i][j])) {
                        ++cnt;
                    }
                }
                if (cnt == static_cast<int>(this->grid[i][j])) {
                    FOR_ADJ {
                        if (ADJ_REG == Region::UNDET) {
                            UPDATE(ADJ_REG, region[i][j]);
                        }
                    }
                } else if (cnt > static_cast<int>(this->grid[i][j])) {
                    return false;
                }

                /* For n > 0, if its region is determined and have 4 - n adjacent cells with the same region, then
                    other adjacent regions must have the different region. */
                cnt = 0;
                FOR_ADJ {
                    if (is_same_region(ADJ_REG, region[i][j])) {
                        ++cnt;
                    }
                }
                if (cnt == 4 - static_cast<int>(this->grid[i][j])) {
                    FOR_ADJ {
                        if (ADJ_REG == Region::UNDET) {
                            UPDATE(ADJ_REG, inv_region(region[i][j]));
                        }
                    }
                } else if (cnt > 4 - static_cast<int>(this->grid[i][j])) {
                    return false;
                }
            }

            /* Avoid checkerboard pattern. */
            if (is_diff_region(region[i][j], region[i + 1][j]) && is_diff_region(region[i][j], region[i][j + 1])) {
                UPDATE_DIFF(region[i][j], region[i + 1][j + 1]);
            }
            if (is_diff_region(region[i][j + 1], region[i][j]) &&
                is_diff_region(region[i][j + 1], region[i + 1][j + 1])) {
                UPDATE_DIFF(region[i][j + 1], region[i + 1][j]);
            }
            if (is_diff_region(region[i + 1][j], region[i][j]) &&
                is_diff_region(region[i + 1][j], region[i + 1][j + 1])) {
                UPDATE_DIFF(region[i + 1][j], region[i][j + 1]);
            }
            if (is_diff_region(region[i + 1][j + 1], region[i][j + 1]) &&
                is_diff_region(region[i + 1][j + 1], region[i + 1][j])) {
                UPDATE_DIFF(region[i + 1][j + 1], region[i][j]);
            }

            if (this->grid[i][j] == Number::ONE) {
                for (int k = 0; k < 8; k += 2) {
                    if ((is_diff_region(ADJD_REG(k), ADJD_REG(k + 1)) &&
                         is_same_region(ADJD_REG(k + 1), ADJD_REG(k + 2))) ||
                        (is_same_region(ADJD_REG(k), ADJD_REG(k + 1)) &&
                         is_diff_region(ADJD_REG(k + 1), ADJD_REG(k + 2)))) {
                        UPDATE_SAME(region[i][j], ADJD_REG(k + 4));
                        UPDATE_SAME(region[i][j], ADJD_REG(k + 6));
                    }

                    if (is_diff_region(ADJD_REG(k), ADJD_REG(k + 1)) && is_same_region(region[i][j], ADJD_REG(k + 4)) &&
                        is_same_region(region[i][j], ADJD_REG(k + 6))) {
                        UPDATE_SAME(ADJD_REG(k + 1), ADJD_REG(k + 2));
                    }

                    if (is_diff_region(ADJD_REG(k + 1), ADJD_REG(k + 2)) &&
                        is_same_region(region[i][j], ADJD_REG(k + 4)) &&
                        is_same_region(region[i][j], ADJD_REG(k + 6))) {
                        UPDATE_SAME(ADJD_REG(k), ADJD_REG(k + 1));
                    }
                }
            }

            if (this->grid[i][j] == Number::ONE && this->grid[i + 1][j + 1] == Number::ONE) {
                if ((is_same_region(region[i][j], region[i + 1][j]) &&
                     is_same_region(region[i][j], region[i][j + 1])) ||
                    (is_same_region(region[i + 1][j + 1], region[i][j + 1]) &&
                     is_same_region(region[i + 1][j + 1], region[i + 1][j]))) {
                    UPDATE_SAME(region[i][j], region[i + 1][j + 1]);
                }

                if (is_same_region(region[i][j], region[i - 1][j]) && is_same_region(region[i][j], region[i][j - 1])) {
                    UPDATE_SAME(region[i + 1][j + 1], region[i + 1][j + 2]);
                    UPDATE_SAME(region[i + 1][j + 1], region[i + 2][j + 1]);
                }

                if (is_same_region(region[i + 1][j + 1], region[i + 1][j + 2]) &&
                    is_same_region(region[i + 1][j + 1], region[i + 2][j + 1])) {
                    UPDATE_SAME(region[i][j], region[i - 1][j]);
                    UPDATE_SAME(region[i][j], region[i][j - 1]);
                }
            }

            if (this->grid[i][j] == Number::ONE && this->grid[i + 1][j - 1] == Number::ONE) {
                if ((is_same_region(region[i][j], region[i][j - 1]) &&
                     is_same_region(region[i][j], region[i + 1][j])) ||
                    (is_same_region(region[i + 1][j - 1], region[i + 1][j]) &&
                     is_same_region(region[i + 1][j - 1], region[i][j - 1]))) {
                    UPDATE_SAME(region[i][j], region[i + 1][j - 1]);
                }

                if (is_same_region(region[i][j], region[i - 1][j]) && is_same_region(region[i][j], region[i][j + 1])) {
                    UPDATE_SAME(region[i + 1][j - 1], region[i + 1][j - 2]);
                    UPDATE_SAME(region[i + 1][j - 1], region[i + 2][j - 1]);
                }

                if (is_same_region(region[i + 1][j - 1], region[i + 1][j - 2]) &&
                    is_same_region(region[i + 1][j - 1], region[i + 2][j - 1])) {
                    UPDATE_SAME(region[i][j], region[i - 1][j]);
                    UPDATE_SAME(region[i][j], region[i][j + 1]);
                }
            }

            /* When two 3's are adjacent. */
            if (this->grid[i][j] == Number::THREE && this->grid[i][j + 1] == Number::THREE &&
                (is_same_region(region[i][j - 1], region[i][j + 1]) || is_diff_region(region[i][j], region[i][j + 1]) ||
                 is_same_region(region[i][j], region[i][j + 2]))) {
                UPDATE_DIFF(region[i][j - 1], region[i][j]);
                UPDATE_SAME(region[i][j - 1], region[i][j + 1]);
                UPDATE_DIFF(region[i][j - 1], region[i][j + 2]);

                UPDATE_DIFF(region[i][j], region[i][j + 1]);
                UPDATE_SAME(region[i][j], region[i][j + 2]);

                UPDATE_DIFF(region[i][j + 1], region[i][j + 2]);
            }
            if (this->grid[i][j] == Number::THREE && this->grid[i + 1][j] == Number::THREE &&
                (is_same_region(region[i - 1][j], region[i + 1][j]) || is_diff_region(region[i][j], region[i + 1][j]) ||
                 is_same_region(region[i][j], region[i + 2][j]))) {
                UPDATE_DIFF(region[i - 1][j], region[i][j]);
                UPDATE_SAME(region[i - 1][j], region[i + 1][j]);
                UPDATE_DIFF(region[i - 1][j], region[i + 2][j]);

                UPDATE_DIFF(region[i][j], region[i + 1][j]);
                UPDATE_SAME(region[i][j], region[i + 2][j]);

                UPDATE_DIFF(region[i + 1][j], region[i + 2][j]);
            }

            /* When two 3's are diagonally adjacent. */
            if (this->grid[i][j] == Number::THREE && this->grid[i + 1][j + 1] == Number::THREE) {
                UPDATE_DIFF(region[i][j], region[i - 1][j]);
                UPDATE_DIFF(region[i][j], region[i][j - 1]);
                UPDATE_DIFF(region[i + 1][j + 1], region[i + 2][j + 1]);
                UPDATE_DIFF(region[i + 1][j + 1], region[i + 1][j + 2]);
            }
            if (this->grid[i][j] == Number::THREE && this->grid[i + 1][j - 1] == Number::THREE) {
                UPDATE_DIFF(region[i][j], region[i - 1][j]);
                UPDATE_DIFF(region[i][j], region[i][j + 1]);
                UPDATE_DIFF(region[i + 1][j - 1], region[i + 2][j - 1]);
                UPDATE_DIFF(region[i + 1][j - 1], region[i + 1][j - 2]);
            }

            /* When two diagnoally adjacent cells with the same region have common neighbor with the same region and
                the other common neighbor is 3, it must have the different region. */
            if (this->grid[i][j] == Number::THREE) {
                if (is_same_region(region[i - 1][j], region[i - 1][j + 1]) &&
                    is_same_region(region[i - 1][j + 1], region[i][j + 1])) {
                    UPDATE(region[i][j], inv_region(region[i - 1][j]));
                }
                if (is_same_region(region[i][j + 1], region[i + 1][j + 1]) &&
                    is_same_region(region[i + 1][j + 1], region[i + 1][j])) {
                    UPDATE(region[i][j], inv_region(region[i][j + 1]));
                }
                if (is_same_region(region[i + 1][j], region[i + 1][j - 1]) &&
                    is_same_region(region[i + 1][j - 1], region[i][j - 1])) {
                    UPDATE(region[i][j], inv_region(region[i + 1][j]));
                }
                if (is_same_region(region[i][j - 1], region[i - 1][j - 1]) &&
                    is_same_region(region[i - 1][j - 1], region[i - 1][j])) {
                    UPDATE(region[i][j], inv_region(region[i][j - 1]));
                }
            }

            /* When two diagnoally adjacent cells with the same region have common neighbor with the same region and
                the other common neighbor is 1, it must have the same region. */
            if (this->grid[i][j] == Number::ONE) {
                if (is_same_region(region[i - 1][j], region[i - 1][j + 1]) &&
                    is_same_region(region[i - 1][j + 1], region[i][j + 1])) {
                    UPDATE(region[i][j], region[i - 1][j]);
                }
                if (is_same_region(region[i][j + 1], region[i + 1][j + 1]) &&
                    is_same_region(region[i + 1][j + 1], region[i + 1][j])) {
                    UPDATE(region[i][j], region[i][j + 1]);
                }
                if (is_same_region(region[i + 1][j], region[i + 1][j - 1]) &&
                    is_same_region(region[i + 1][j - 1], region[i][j - 1])) {
                    UPDATE(region[i][j], region[i + 1][j]);
                }
                if (is_same_region(region[i][j - 1], region[i - 1][j - 1]) &&
                    is_same_region(region[i - 1][j - 1], region[i - 1][j])) {
                    UPDATE(region[i][j], region[i][j - 1]);
                }
            }
        }

        /* If a 1 is in a corner, it must be in the outer region. */
        if (this->grid[1][1] == Number::ONE) {
            UPDATE(region[1][1], Region::OUTER);
        }
        if (this->grid[1][this->nc] == Number::ONE) {
            UPDATE(region[1][this->nc], Region::OUTER);
        }
        if (this->grid[this->nr][1] == Number::ONE) {
            UPDATE(region[this->nr][1], Region::OUTER);
        }
        if (this->grid[this->nr][this->nc] == Number::ONE) {
            UPDATE(region[this->nr][this->nc], Region::OUTER);
        }

        /* If a 2 is in a corner, its two adjacent cells must be in the inner region. */
        if (this->grid[1][1] == Number::TWO) {
            UPDATE(region[1][2], Region::INNER);
            UPDATE(region[2][1], Region::INNER);
        }
        if (this->grid[1][this->nc] == Number::TWO) {
            UPDATE(region[1][this->nc - 1], Region::INNER);
            UPDATE(region[2][this->nc], Region::INNER);
        }
        if (this->grid[this->nr][1] == Number::TWO) {
            UPDATE(region[this->nr - 1][1], Region::INNER);
            UPDATE(region[this->nr][2], Region::INNER);
        }
        if (this->grid[this->nr][this->nc] == Number::TWO) {
            UPDATE(region[this->nr][this->nc - 1], Region::INNER);
            UPDATE(region[this->nr - 1][this->nc], Region::INNER);
        }

        /* If a 3 is in a corner, it must be in the inner region. */
        if (this->grid[1][1] == Number::THREE) {
            UPDATE(region[1][1], Region::INNER);
        }
        if (this->grid[1][this->nc] == Number::THREE) {
            UPDATE(region[1][this->nc], Region::INNER);
        }
        if (this->grid[this->nr][1] == Number::THREE) {
            UPDATE(region[this->nr][1], Region::INNER);
        }
        if (this->grid[this->nr][this->nc] == Number::THREE) {
            UPDATE(region[this->nr][this->nc], Region::INNER);
        }
    }

    return true;
}

bool Slitherlink::is_available_partial_solution(std::vector<std::vector<Region>> &region) {
    /* No checkerboard pattern is allowed in 2x2 cells. */
    FOR_CELL {
        if (is_diff_region(region[i][j], region[i][j + 1]) && is_diff_region(region[i][j + 1], region[i + 1][j + 1]) &&
            is_diff_region(region[i + 1][j + 1], region[i + 1][j])) {
            return false;
        }
    }

    /* Fill UNDET's and OUTER's to UNDET_BFS and OUTER_BFS, respectively, starting from (0, 0) using BFS. */
    std::queue<std::pair<int, int>> q;
    q.push(std::make_pair(0, 0));
    while (!q.empty()) {
        auto [i, j] = q.front();
        q.pop();
        if (region[i][j] != Region::UNDET && region[i][j] != Region::OUTER) {
            continue;
        }
        region[i][j] = region[i][j] == Region::UNDET ? Region::UNDET_BFS : Region::OUTER_BFS;
        FOR_ADJ {
            if (ADJ_REG == Region::UNDET || ADJ_REG == Region::OUTER) {
                q.push(std::make_pair(i + adj[k][0], j + adj[k][1]));
            }
        }
    }

    /* Check if there is no OUTER's. If there is an UNDET, it must be INNER. Then set back all UNDET_BFS's and
     * OUTER_BFS's to UNDET and OUTER, respectively. */
    bool no_hole = true;
    for (int i = 0; i < this->nr + 2; ++i) {
        for (int j = 0; j < this->nc + 2; ++j) {
            if (region[i][j] == Region::OUTER) {
                no_hole = false;
            } else if (region[i][j] == Region::UNDET) {
                region[i][j] = Region::INNER;
            } else if (region[i][j] == Region::UNDET_BFS) {
                region[i][j] = Region::UNDET;
            } else if (region[i][j] == Region::OUTER_BFS) {
                region[i][j] = Region::OUTER;
            }
        }
    }
    if (!no_hole) {
        return false;
    }

    return true;
}

bool Slitherlink::is_answer(std::vector<std::vector<Region>> &region) {
    /* Check if there is no Region::UNDET. */
    FOR_CELL {
        if (region[i][j] == Region::UNDET) {
            return false;
        }
    }

    /* Check if the region coincises well with the grid. */
    FOR_CELL {
        if (this->grid[i][j] == Number::EMPTY) {
            continue;
        }

        int cnt = 0;
        FOR_ADJ {
            if (region[i][j] != ADJ_REG) {
                ++cnt;
            }
        }
        if (cnt != static_cast<int>(this->grid[i][j])) {
            return false;
        }
    }

    /* Find the first inner region. */
    auto [i2, j2] = this->find_region(region, Region::INNER);
    /* No inner region. */
    if (i2 == -1 && j2 == -1) {
        return true;
    }

    /* Fill INNER's to INNER_BFS starting from (i2, j2) using BFS. */
    std::queue<std::pair<int, int>> q;
    q.push(std::make_pair(i2, j2));
    while (!q.empty()) {
        auto [i, j] = q.front();
        q.pop();
        if (region[i][j] != Region::INNER) {
            continue;
        }
        region[i][j] = Region::INNER_BFS;
        FOR_ADJ {
            if (ADJ_REG == Region::INNER) {
                q.push(std::make_pair(i + adj[k][0], j + adj[k][1]));
            }
        }
    }

    /* Check if there is no INNER's and set back all INNER_BFS's to INNER. */
    bool no_inner = true;
    FOR_CELL {
        if (region[i][j] == Region::INNER) {
            no_inner = false;
        } else if (region[i][j] == Region::INNER_BFS) {
            region[i][j] = Region::INNER;
        }
    }

    return no_inner;
}

std::pair<int, int> Slitherlink::find_region(std::vector<std::vector<Region>> &region, Region val) {
    FOR_CELL {
        if (region[i][j] == val) {
            return std::make_pair(i, j);
        }
    }
    return std::make_pair(-1, -1);
}

void Slitherlink::print_region(const std::vector<std::vector<Region>> &region) {
    for (int i = 1; i < this->nr + 1; ++i) {
        for (int j = 1; j < this->nc + 1; ++j) {
            std::cout << static_cast<int>(region[i][j]) << " ";
        }
        std::cout << std::endl;
    }
}

static Region inv_region(Region r) {
    return r == Region::UNDET ? Region::UNDET : (r == Region::INNER ? Region::OUTER : Region::INNER);
}

static bool is_same_region(Region r1, Region r2) {
    return (r1 == Region::INNER && r2 == Region::INNER) || (r1 == Region::OUTER && r2 == Region::OUTER);
}

static bool is_diff_region(Region r1, Region r2) {
    return (r1 == Region::INNER && r2 == Region::OUTER) || (r1 == Region::OUTER && r2 == Region::INNER);
}

static bool update_region(Region &r, Region nr, bool &updated) {
    updated = false;
    if (is_diff_region(r, nr)) {
        return false;
    }
    if (r == Region::UNDET && nr != Region::UNDET) {
        r = nr;
        updated = true;
    }
    return true;
}
