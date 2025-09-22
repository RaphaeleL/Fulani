# Rule 110 Cellular Automaton Implementation
# Rule 110 is a 1D cellular automaton that has been proven to be Turing complete

def print_cells(cells):
    """Helper function to print a generation of cells."""
    for cell in cells:
        print("x " if cell == 1 else "  ", end="")
    print()

def rule_110(left, center, right):
    """Apply Rule 110 to a triplet of cells."""
    pattern = (left << 2) | (center << 1) | right
    # Rule 110 mapping: 111->0, 110->1, 101->1, 100->0, 011->1, 010->1, 001->1, 000->0
    return 1 if pattern in [1, 2, 3, 5, 6] else 0

def main():
    print("Rule 110 Cellular Automaton")
    print("--------------------------")
    print("Rule 110 Pattern:")
    print("111 -> 0    011 -> 1")
    print("110 -> 1    010 -> 1")
    print("101 -> 1    001 -> 1")
    print("100 -> 0    000 -> 0")
    print()

    size = 50          # Size of the cellular automaton
    generations = size # Number of generations

    # Initialize cells with a single live cell at the end
    cells = [1 if i == size - 1 else 0 for i in range(size)]

    print_cells(cells)

    # Run the simulation
    for _ in range(generations):
        next_generation = [
            rule_110(
                cells[i - 1] if i > 0 else 0,
                cells[i],
                cells[i + 1] if i < size - 1 else 0
            )
            for i in range(size)
        ]
        print_cells(next_generation)
        cells = next_generation

    print("\nRule 110 is proven to be Turing complete,")
    print("meaning it can simulate any Turing machine and")
    print("therefore compute anything that is computable.")

if __name__ == "__main__":
    main()

