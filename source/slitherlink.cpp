#include <iostream>
#include <queue>
#include <utility>
#include "slitherlink.hpp"

using namespace slink;

#define FOR_CELL for (int i = 1; i < this->nr + 1; ++i) for (int j = 1; j < this->nc + 1; ++j)
#define FOR_ADJ for (int k = 0; k < 4; k++) if (0 <= i + adj[k][0] && i + adj[k][0] < this->nr + 2 && 0 <= j + adj[k][1] && j + adj[k][1] < this->nc + 2)
#define ADJ_REG (region[i + adj[k][0]][j + adj[k][1]])
#define INV(r) ((r) == UNDET ? UNDET : ((r) == INNER ? OUTER : INNER))
#define IS_SAME(r1, r2) (((r1) == INNER && (r2) == INNER) || ((r1) == OUTER && (r2) == OUTER))
#define IS_DIFF(r1, r2) (((r1) == INNER && (r2) == OUTER) || ((r1) == OUTER && (r2) == INNER))
#define UPDATE(r, nr) do { if (IS_DIFF(r, nr)) { return false; } if ((r) == UNDET && (nr) != UNDET) { (r) = (nr); changed = true; } } while (0)

constexpr int adj[4][2] = {
    { -1,  0 },
    {  0,  1 },
    {  1,  0 },
    {  0, -1 },
};

Slitherlink::Slitherlink(const std::vector<std::vector<int>> &grid) :
    nr(grid.size()),
    nc(grid[0].size()),
    grid(nr + 2, std::vector<int>(nc + 2, EMPTY)) {
    FOR_CELL {
        this->grid[i][j] = grid[i - 1][j - 1];
    }
}

Slitherlink::Slitherlink(const std::vector<std::string> &grid) :
    nr(grid.size()),
    nc(grid[0].size()),
    grid(nr + 2, std::vector<int>(nc + 2, EMPTY)) {
    FOR_CELL {
        this->grid[i][j] = isdigit(grid[i - 1][j - 1]) ? grid[i - 1][j - 1] - '0' : EMPTY;
    }
}

bool Slitherlink::solve(void) {
    std::vector<std::vector<int>> region(this->nr + 2, std::vector<int>(this->nc + 2, UNDET));
    for (int i = 0; i < this->nr + 2; ++i) {
        region[i][0] = region[i][this->nc + 1] = OUTER;
    }
    for (int j = 1; j < this->nc + 1; ++j) {
        region[0][j] = region[this->nr + 1][j] = OUTER;
    }

    return this->solve_helper(region);
}

void Slitherlink::print_solution(void) {
    std::vector<std::string> buf(2 * this->nr + 3, std::string(4 * this->nc + 5, ' '));

    for (int i = 1; i < this->nr + 1; ++i) {
        for (int j = 1; j < this->nc + 1; ++j) {
            buf[2 * i][4 * j] = this->grid[i][j] != -1 ? this->grid[i][j] + '0' : ' ';
        }
    }

    for (int i = 0; i < this->nr + 1; ++i) {
        for (int j = 1; j < this->nc + 1; ++j) {
            if (IS_DIFF(this->region_solved[i][j], this->region_solved[i + 1][j])) {
                buf[2 * i + 1][4 * j - 1] = buf[2 * i + 1][4 * j] = buf[2 * i + 1][4 * j + 1] = '-';
            }
        }
    }
    for (int i = 1; i < this->nr + 1; ++i) {
        for (int j = 0; j < this->nc + 1; ++j) {
            if (IS_DIFF(this->region_solved[i][j], this->region_solved[i][j + 1])) {
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

bool Slitherlink::solve_helper(const std::vector<std::vector<int>> &region) {
    std::vector<std::vector<int>> new_region = region;
    if (!this->apply_heuristics(new_region)) {
        return false;
    }
    if (this->is_answer(new_region)) {
        this->region_solved = new_region;
        return true;
    }

    /* Find the first undetermined region. */
    auto [i0, j0] = this->find_region(new_region, UNDET);
    /* No undetermined region. */
    if (i0 == (int)(-1) && j0 == (int)(-1)) {
        return false;
    }

    /* Try to fill the region with OUTER. */
    new_region[i0][j0] = OUTER;
    if (this->solve_helper(new_region)) {
        return true;
    }
    /* Try to fill the region with INNER. */
    new_region[i0][j0] = INNER;
    if (this->solve_helper(new_region)) {
        return true;
    }
    return false;
}

bool Slitherlink::apply_heuristics(std::vector<std::vector<int>> &region) {
    if (!this->is_available_partial_solution(region)) {
        return false;
    }

    bool changed = true;
    while (changed) {
        changed = false;

        FOR_CELL {
            /* For 0, its adjacent cell must have the same region. */
            if (this->grid[i][j] == 0) {
                FOR_ADJ {
                    UPDATE(region[i][j], ADJ_REG);
                }
                FOR_ADJ {
                    UPDATE(ADJ_REG, region[i][j]);
                }
            }

            if (this->grid[i][j] > 0 && region[i][j] != UNDET) {
                /* For n > 0, if its region is determined and have n adjacent cells with the different region, then
                    other adjacent regions must have the same region. */
                int cnt = 0;
                FOR_ADJ {
                    if (IS_DIFF(ADJ_REG, region[i][j])) {
                        ++cnt;
                    }
                }
                if (cnt == this->grid[i][j]) {
                    FOR_ADJ {
                        if (ADJ_REG == UNDET) {
                            UPDATE(ADJ_REG, region[i][j]);
                        }
                    }
                } else if (cnt > this->grid[i][j]) {
                    return false;
                }

                /* For n > 0, if its region is determined and have 4 - n adjacent cells with the same region, then
                    other adjacent regions must have the different region. */
                cnt = 0;
                FOR_ADJ {
                    if (IS_SAME(ADJ_REG, region[i][j])) {
                        ++cnt;
                    }
                }
                if (cnt == 4 - this->grid[i][j]) {
                    FOR_ADJ {
                        if (ADJ_REG == UNDET) {
                            UPDATE(ADJ_REG, INV(region[i][j]));
                        }
                    }
                } else if (cnt > 4 - this->grid[i][j]) {
                    return false;
                }
            }

            /* When two 3's are adjacent. */
            if (this->grid[i][j] == 3 && this->grid[i][j + 1] == 3 &&
                (IS_SAME(region[i][j - 1], region[i][j + 1]) || IS_DIFF(region[i][j], region[i][j + 1]) ||
                IS_SAME(region[i][j], region[i][j + 2]))) {
                UPDATE(region[i][j - 1], INV(region[i][j]));
                UPDATE(region[i][j - 1], region[i][j + 1]);
                UPDATE(region[i][j - 1], INV(region[i][j + 2]));

                UPDATE(region[i][j], INV(region[i][j - 1]));
                UPDATE(region[i][j], INV(region[i][j + 1]));
                UPDATE(region[i][j], region[i][j + 2]);

                UPDATE(region[i][j + 1], region[i][j - 1]);
                UPDATE(region[i][j + 1], INV(region[i][j]));
                UPDATE(region[i][j + 1], INV(region[i][j + 2]));

                UPDATE(region[i][j + 2], INV(region[i][j - 1]));
                UPDATE(region[i][j + 2], region[i][j]);
                UPDATE(region[i][j + 2], INV(region[i][j + 1]));
            }
            if (this->grid[i][j] == 3 && this->grid[i + 1][j] == 3 &&
                (IS_SAME(region[i - 1][j], region[i + 1][j]) || IS_DIFF(region[i][j], region[i + 1][j]) ||
                IS_SAME(region[i][j], region[i + 2][j]))) {
                UPDATE(region[i - 1][j], INV(region[i][j]));
                UPDATE(region[i - 1][j], region[i + 1][j]);
                UPDATE(region[i - 1][j], INV(region[i + 2][j]));

                UPDATE(region[i][j], INV(region[i - 1][j]));
                UPDATE(region[i][j], INV(region[i + 1][j]));
                UPDATE(region[i][j], region[i + 2][j]);

                UPDATE(region[i + 1][j], region[i - 1][j]);
                UPDATE(region[i + 1][j], INV(region[i][j]));
                UPDATE(region[i + 1][j], INV(region[i + 2][j]));

                UPDATE(region[i + 2][j], INV(region[i - 1][j]));
                UPDATE(region[i + 2][j], region[i][j]);
                UPDATE(region[i + 2][j], INV(region[i + 1][j]));
            }

            /* When two 3's are diagonally adjacent. */
            if (this->grid[i][j] == 3 && this->grid[i + 1][j + 1] == 3) {
                UPDATE(region[i][j], INV(region[i - 1][j]));
                UPDATE(region[i][j], INV(region[i][j - 1]));
                UPDATE(region[i - 1][j], INV(region[i][j]));
                UPDATE(region[i][j - 1], INV(region[i][j]));

                UPDATE(region[i + 1][j + 1], INV(region[i + 1][j + 2]));
                UPDATE(region[i + 1][j + 1], INV(region[i + 2][j + 1]));
                UPDATE(region[i + 1][j + 2], INV(region[i + 1][j + 1]));
                UPDATE(region[i + 2][j + 1], INV(region[i + 1][j + 1]));
            }
            if (this->grid[i][j] == 3 && this->grid[i + 1][j - 1] == 3) {
                UPDATE(region[i][j], INV(region[i - 1][j]));
                UPDATE(region[i][j], INV(region[i][j + 1]));
                UPDATE(region[i - 1][j], INV(region[i][j]));
                UPDATE(region[i][j + 1], INV(region[i][j]));

                UPDATE(region[i + 1][j - 1], INV(region[i + 1][j - 2]));
                UPDATE(region[i + 1][j - 1], INV(region[i + 2][j - 1]));
                UPDATE(region[i + 1][j - 2], INV(region[i + 1][j - 1]));
                UPDATE(region[i + 2][j - 1], INV(region[i + 1][j - 1]));
            }
        }

        FOR_CELL {
            /* When two diagnoally adjacent cells with the same region have common neighbor with the same region and
                the other common neighbor is 3, it must have the different region. */
            if (this->grid[i][j] == 3) {
                if (IS_SAME(region[i - 1][j], region[i - 1][j + 1]) &&
                    IS_SAME(region[i - 1][j + 1], region[i][j + 1])) {
                    UPDATE(region[i][j], INV(region[i - 1][j]));
                }
                if (IS_SAME(region[i][j + 1], region[i + 1][j + 1]) &&
                    IS_SAME(region[i + 1][j + 1], region[i + 1][j])) {
                    UPDATE(region[i][j], INV(region[i][j + 1]));
                }
                if (IS_SAME(region[i + 1][j], region[i + 1][j - 1]) &&
                    IS_SAME(region[i + 1][j - 1], region[i][j - 1])) {
                    UPDATE(region[i][j], INV(region[i + 1][j]));
                }
                if (IS_SAME(region[i][j - 1], region[i - 1][j - 1]) &&
                    IS_SAME(region[i - 1][j - 1], region[i - 1][j])) {
                    UPDATE(region[i][j], INV(region[i][j - 1]));
                }
            }

            /* When two diagnoally adjacent cells with the same region have common neighbor with the same region and
                the other common neighbor is 1, it must have the same region. */
            if (this->grid[i][j] == 1) {
                if (IS_SAME(region[i - 1][j], region[i - 1][j + 1]) &&
                    IS_SAME(region[i - 1][j + 1], region[i][j + 1])) {
                    UPDATE(region[i][j], region[i - 1][j]);
                }
                if (IS_SAME(region[i][j + 1], region[i + 1][j + 1]) &&
                    IS_SAME(region[i + 1][j + 1], region[i + 1][j])) {
                    UPDATE(region[i][j], region[i][j + 1]);
                }
                if (IS_SAME(region[i + 1][j], region[i + 1][j - 1]) &&
                    IS_SAME(region[i + 1][j - 1], region[i][j - 1])) {
                    UPDATE(region[i][j], region[i + 1][j]);
                }
                if (IS_SAME(region[i][j - 1], region[i - 1][j - 1]) &&
                    IS_SAME(region[i - 1][j - 1], region[i - 1][j])) {
                    UPDATE(region[i][j], region[i][j - 1]);
                }
            }
        }

        /* If a 1 is in a corner, it must be in the outer region. */
        if (this->grid[1][1] == 1) {
            UPDATE(region[1][1], OUTER);
        }
        if (this->grid[1][this->nc] == 1) {
            UPDATE(region[1][this->nc], OUTER);
        }
        if (this->grid[this->nr][1] == 1) {
            UPDATE(region[this->nr][1], OUTER);
        }
        if (this->grid[this->nr][this->nc] == 1) {
            UPDATE(region[this->nr][this->nc], OUTER);
        }

        /* If a 2 is in a corner, its two adjacent cells must be in the inner region. */
        if (this->grid[1][1] == 2) {
            UPDATE(region[1][2], INNER);
            UPDATE(region[2][1], INNER);
        }
        if (this->grid[1][this->nc] == 2) {
            UPDATE(region[1][this->nc - 1], INNER);
            UPDATE(region[2][this->nc], INNER);
        }
        if (this->grid[this->nr][1] == 2) {
            UPDATE(region[this->nr - 1][1], INNER);
            UPDATE(region[this->nr][2], INNER);
        }
        if (this->grid[this->nr][this->nc] == 2) {
            UPDATE(region[this->nr][this->nc - 1], INNER);
            UPDATE(region[this->nr - 1][this->nc], INNER);
        }

        /* If a 3 is in a corner, it must be in the inner region. */
        if (this->grid[1][1] == 3) {
            UPDATE(region[1][1], INNER);
        }
        if (this->grid[1][this->nc] == 3) {
            UPDATE(region[1][this->nc], INNER);
        }
        if (this->grid[this->nr][1] == 3) {
            UPDATE(region[this->nr][1], INNER);
        }
        if (this->grid[this->nr][this->nc] == 3) {
            UPDATE(region[this->nr][this->nc], INNER);
        }
    }

    return true;
}

bool Slitherlink::is_available_partial_solution(std::vector<std::vector<int>> &region) {
    /* No checkerboard pattern is allowed in 2x2 cells. */
    FOR_CELL {
        if (IS_DIFF(region[i][j], region[i][j + 1]) && IS_DIFF(region[i][j + 1], region[i + 1][j + 1]) &&
            IS_DIFF(region[i + 1][j + 1], region[i + 1][j])) {
            return false;
        }
    }

    /* Fill UNDET's and OUTER's to UNDET_BFS and OUTER_BFS, respectively, starting from (0, 0) using BFS. */
    std::queue<std::pair<int, int>> q;
    q.push(std::make_pair(0, 0));
    while (!q.empty()) {
        auto [i, j] = q.front();
        q.pop();
        if (region[i][j] != UNDET && region[i][j] != OUTER) {
            continue;
        }
        region[i][j] = region[i][j] == UNDET ? UNDET_BFS : OUTER_BFS;
        FOR_ADJ {
            if (ADJ_REG == UNDET || ADJ_REG == OUTER) {
                q.push(std::make_pair(i + adj[k][0], j + adj[k][1]));
            }
        }
    }

    /* Check if there is no OUTER's. If there is an UNDET, it must be INNER. Then set back all UNDET_BFS's and
        OUTER_BFS's to UNDET and OUTER, respectively. */
    bool no_hole = true;
    for (int i = 0; i < this->nr + 2; ++i) {
        for (int j = 0; j < this->nc + 2; ++j) {
            if (region[i][j] == OUTER) {
                no_hole = false;
            } else if (region[i][j] == UNDET) {
                region[i][j] = INNER;
            } else if (region[i][j] == UNDET_BFS) {
                region[i][j] = UNDET;
            } else if (region[i][j] == OUTER_BFS) {
                region[i][j] = OUTER;
            }
        }
    }
    if (!no_hole) {
        return false;
    }

    return true;
}

bool Slitherlink::is_answer(std::vector<std::vector<int>> &region) {
    /* Check if there is no UNDET. */
    FOR_CELL {
        if (region[i][j] == UNDET) {
            return false;
        }
    }

    /* Check if the region coincises well with the grid. */
    FOR_CELL {
        if (this->grid[i][j] == EMPTY) {
            continue;
        }

        int cnt = 0;
        FOR_ADJ {
            if (region[i][j] != ADJ_REG) {
                ++cnt;
            }
        }
        if (cnt != this->grid[i][j]) {
            return false;
        }
    }

    /* Find the first inner region. */
    auto [i2, j2] = this->find_region(region, INNER);
    /* No inner region. */
    if (i2 == (int)(-1) && j2 == (int)(-1)) {
        return true;
    }

    /* Fill INNER's to INNER_BFS starting from (i2, j2) using BFS. */
    std::queue<std::pair<int, int>> q;
    q.push(std::make_pair(i2, j2));
    while (!q.empty()) {
        auto [i, j] = q.front();
        q.pop();
        if (region[i][j] != INNER) {
            continue;
        }
        region[i][j] = INNER_BFS;
        FOR_ADJ {
            if (ADJ_REG == INNER) {
                q.push(std::make_pair(i + adj[k][0], j + adj[k][1]));
            }
        }
    }

    /* Check if there is no INNER's and set back all INNER_BFS's to INNER. */
    bool no_inner = true;
    FOR_CELL {
        if (region[i][j] == INNER) {
            no_inner = false;
        } else if (region[i][j] == INNER_BFS) {
            region[i][j] = INNER;
        }
    }

    return no_inner;
}

std::pair<int, int> Slitherlink::find_region(std::vector<std::vector<int>> &region, int val) {
    FOR_CELL {
        if (region[i][j] == val) {
            return std::make_pair(i, j);
        }
    }
    return std::make_pair((int)(-1), (int)(-1));
}

void Slitherlink::print_region(const std::vector<std::vector<int>> &region) {
    for (int i = 1; i < this->nr + 1; ++i) {
        for (int j = 1; j < this->nc + 1; ++j) {
            std::cout << region[i][j] << " ";
        }
        std::cout << std::endl;
    }
}
