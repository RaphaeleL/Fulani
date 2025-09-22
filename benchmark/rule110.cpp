#include <iostream>
#include <vector>

void print_cells(const std::vector<int>& cells) {
    for (int c : cells)
        std::cout << (c ? "x " : "  ");
    std::cout << "\n";
}

int rule_110(int left, int center, int right) {
    int pattern = (left << 2) | (center << 1) | right;
    return (pattern == 1 || pattern == 2 || pattern == 3 || pattern == 5 || pattern == 6) ? 1 : 0;
}

int main() {
    int size = 50;
    int generations = size;

    std::vector<int> cells(size, 0);
    cells[size - 1] = 1;

    print_cells(cells);

    for (int gen = 0; gen < generations; gen++) {
        std::vector<int> next_gen(size);
        for (int i = 0; i < size; i++) {
            int left = (i > 0) ? cells[i - 1] : 0;
            int center = cells[i];
            int right = (i < size - 1) ? cells[i + 1] : 0;
            next_gen[i] = rule_110(left, center, right);
        }
        print_cells(next_gen);
        cells = next_gen;
    }
    return 0;
}

