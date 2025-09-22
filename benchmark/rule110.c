#include <stdio.h>
#include <stdlib.h>

void print_cells(int *cells, int size) {
    for (int i = 0; i < size; i++) {
        printf(cells[i] ? "x " : "  ");
    }
    printf("\n");
}

int rule_110(int left, int center, int right) {
    int pattern = (left << 2) | (center << 1) | right;
    return (pattern == 1 || pattern == 2 || pattern == 3 || pattern == 5 || pattern == 6) ? 1 : 0;
}

int main() {
    int size = 50;
    int generations = size;

    int *cells = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        cells[i] = (i == size - 1) ? 1 : 0;
    }

    print_cells(cells, size);

    for (int gen = 0; gen < generations; gen++) {
        int *next_gen = malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            int left = (i > 0) ? cells[i - 1] : 0;
            int center = cells[i];
            int right = (i < size - 1) ? cells[i + 1] : 0;
            next_gen[i] = rule_110(left, center, right);
        }
        print_cells(next_gen, size);
        int *temp = cells;
        cells = next_gen;
        free(temp);
    }

    free(cells);
    return 0;
}
