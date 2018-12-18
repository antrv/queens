#include <vector>
#include <stack>
#include <functional>
#include <iostream>
#include "types.h"

// global variables
struct
{
    uint size;
    uint size_minus_1;
    uint horizontal_size;
    uint cartesian_max_y;
    uint cell_count;

    struct
    {
        // this numbers help to check whether cartesian coords are inside the board
        uint x_plus_y_max;
        int x_minus_y_min;
    } borders;

    // cumulative column-by-column cell count in a board, starting from left column
    std::vector<uint> cumulative_cell_count;
} board;

inline uint abs_of_diff(const uint a, const uint b) noexcept
{
    return a >= b ? a - b : b - a;
}

// hexagonal coords: see https://en.wikipedia.org/wiki/Hexagonal_chess
// https://en.wikipedia.org/wiki/Hexagonal_chess#/media/File:Hexagonal_chess.svg
// https://codegolf.stackexchange.com/questions/116131/working-on-my-knight-moves

struct hexagonal_coords;

struct cartesian_coords
{
    uint x;
    uint y;

    bool inside_board() const noexcept
    {
        int sum = x + y;
        int diff = static_cast<int>(x) - static_cast<int>(y);

        bool result = x >= 0 && x < board.horizontal_size &&
                      y >= 0 && y <= board.cartesian_max_y &&
                      (sum & 1) == 1 &&
                      sum >= board.size_minus_1 && sum <= board.borders.x_plus_y_max &&
                      diff >= board.borders.x_minus_y_min &&
                      diff <= static_cast<int>(board.size_minus_1);

        return result;
    }

    explicit operator hexagonal_coords() const noexcept;
};

struct hexagonal_coords
{
    uint x;
    uint y;

    static hexagonal_coords from_index(const uint index) noexcept
    {
        // index < board.cell_count

        const auto &cumulative_cell_count = board.cumulative_cell_count;

        // find column: binary search
        size_t left = 0, right = board.horizontal_size;
        while (left <= right)
        {
            size_t middle = (left + right) / 2;
            uint value = cumulative_cell_count[middle];
            if (index < value)
            {
                // cell is somewhere on the left
                right = middle - 1;
            }
            else if (index >= cumulative_cell_count[middle + 1])
            {
                // cell is somewhere on the right
                left = middle + 1;
            }
            else
            {
                // cell is in this column
                return {static_cast<uint>(middle), index - value};
            }
        }

        // must not reach here
        return {};
    }

    uint to_index() const noexcept
    {
        return board.cumulative_cell_count[x] + y;
    }

    explicit operator cartesian_coords() const noexcept
    {
        uint cartesian_y = board.size_minus_1 * 4 - abs_of_diff(x, board.size_minus_1) - 2 * y;
        return {x, cartesian_y};
    }
};

cartesian_coords::operator hexagonal_coords() const noexcept
{
    uint hexagonal_y = (board.size_minus_1 * 4 - abs_of_diff(x, board.size_minus_1) - y) / 2;
    return {x, hexagonal_y};
}

class bitmap
{
    std::vector<size_t> _data;

    constexpr static size_t get_array_size(size_t bit_count)
    {
        // 0 --> 0
        // 1 --> 1
        // ...
        // 64 -> 1 (on 64-bit PC)
        // 65 -> 2 (on 64-bit PC)
        return (bit_count + sizeof(size_t) - 1) / sizeof(size_t);
    }

  public:
    bitmap() : _data(get_array_size(board.cell_count))
    {
    }

    inline bool get_bit(size_t index) const noexcept
    {
        return _data[index / sizeof(size_t)] & (1 << (index % sizeof(size_t)));
    }

    inline void set_bit(size_t index) noexcept
    {
        _data[index / sizeof(size_t)] |= (1 << (index % sizeof(size_t)));
    }

    inline void clear_bit(size_t index) noexcept
    {
        _data[index / sizeof(size_t)] &= ~(1 << (index % sizeof(size_t)));
    }
};

// board state
struct board_state
{
    // queen coordinates
    std::vector<uint> queens;
    // captured cells
    bitmap cells;
    // index of the next cell to place a queen
    uint index; // must be less than board.cell_count

  private:
    void mark_captured_cells(const cartesian_coords initial,
                             const std::function<cartesian_coords(const cartesian_coords)> iterator) noexcept
    {
        cartesian_coords coords = iterator(initial);
        while (coords.inside_board())
        {
            uint index = static_cast<hexagonal_coords>(coords).to_index();
            cells.set_bit(index);

            coords = iterator(coords);
        }
    }

  public:
    void print(bool with_captured_cells = false)
    {
        for (size_t i = 0; i < queens.size(); i++)
        {
            hexagonal_coords c = hexagonal_coords::from_index(queens[i]);
            std::cout << "Queen " << (i + 1) << " coordinates: (" << (c.x + 1) << ", " << (c.y + 1) << ")\n";
        }

        // output ascii art
        for (uint y = 0; y <= board.cartesian_max_y; y++)
        {
            for (uint x = 0; x < board.horizontal_size; x++)
            {
                cartesian_coords coords{x, y};
                char output = ' ';

                if (coords.inside_board())
                {
                    uint index = static_cast<hexagonal_coords>(coords).to_index();
                    bool queen_in_cell = std::find(queens.begin(), queens.end(), index) != queens.end();
                    if (queen_in_cell)
                    {
                        output = 'Q';
                    }
                    else if (with_captured_cells && cells.get_bit(index))
                    {
                        output = '*';
                    }
                    else
                    {
                        output = 'o';
                    }
                }

                std::cout << output;
                std::cout << "  ";
            }

            std::cout << "\n";
        }
    }

    void place_queen() noexcept
    {
        // index must be valid here and cell must not be captured yet

        // add queen to list
        queens.push_back(index);

        // mark the cell as captured
        cells.set_bit(index);

        // iterate captured cells by this new queen and mark them as captured
        cartesian_coords queen_coords = static_cast<cartesian_coords>(hexagonal_coords::from_index(index));

        // up
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x, c.y - 2}; });

        // down
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x, c.y + 2}; });

        // left
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x - 2, c.y}; });

        // right
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x + 2, c.y}; });

        // major diagonals

        // up-left
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x - 1, c.y - 1}; });

        // up-right
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x + 1, c.y - 1}; });

        // down-left
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x - 1, c.y + 1}; });

        // down-right
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x + 1, c.y + 1}; });

        // minor diagonals

        // up-left
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x - 1, c.y - 3}; });

        // up-right
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x + 1, c.y - 3}; });

        // down-left
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x - 1, c.y + 3}; });

        // down-right
        mark_captured_cells(queen_coords, [](cartesian_coords c) { return cartesian_coords{c.x + 1, c.y + 3}; });

        // some debug output
        /*std::cout << "-------------------\n";
        std::cout << "Place new queen to (";
        auto c1 = hexagonal_coords::from_index(index);
        std::cout << (c1.x + 1) << ", " << (c1.y + 1) << ")\n";
        print(true);*/

        // advance index
        advance_index();
    }

    void advance_index() noexcept
    {
        index++;

        // skip captured cells
        while (index < board.cell_count && cells.get_bit(index))
        {
            index++;
        }
    }
};

void calculate_cumulative_cell_count()
{
    size_t size = board.horizontal_size + 1;
    auto &v = board.cumulative_cell_count = std::vector<uint>(size);

    // 6,7,...,10,11,10,...,6
    uint cell_count = 0;
    for (uint x = 0; x < board.horizontal_size; x++)
    {
        uint cells_in_column = board.size + board.size_minus_1 - abs_of_diff(board.size_minus_1, x);
        cell_count += cells_in_column;
        v[x + 1] = cell_count;
    }
}

void solve(uint size) // size >= 16
{
    board.size = size;
    board.size_minus_1 = size - 1;
    board.horizontal_size = size * 2 - 1;
    board.cartesian_max_y = (size - 1) * 4;
    board.cell_count = 3 * size * (size - 1) + 1;

    board.borders.x_plus_y_max = board.cartesian_max_y + (size - 1);
    board.borders.x_minus_y_min = static_cast<int>(size - 1) - static_cast<int>(board.cartesian_max_y);

    calculate_cumulative_cell_count();

    std::stack<board_state> stack{}; // stack of states
    stack.push(board_state{});       // add empty state

    // termination condition: all 2 * size - 1 queens are placed or no available cells to place a queen
    while (stack.size() > 0)
    {
        board_state &st = stack.top(); // last state
        if (st.index >= board.cell_count)
        {
            // no cells available to place a queen
            //std::cout << "Remove queen\n";
            stack.pop(); // remove last state

            if (stack.size() > 0)
            {
                // advance index
                stack.top().advance_index();
            }
        }
        else
        {
            board_state new_state = st; // copy state
            new_state.place_queen();    // place one queen

            if (new_state.queens.size() == board.horizontal_size)
            {
                // solved!
                std::cout << "Solved!\n";
                new_state.print();
                return;
            }

            stack.push(new_state); // push a new state to the stack
        }
    }

    std::cout << "No solution!\n";
    return;
}
