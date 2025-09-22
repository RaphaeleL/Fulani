package main

import "fmt"

func printCells(cells []int) {
    for _, c := range cells {
        if c == 1 {
            fmt.Print("x ")
        } else {
            fmt.Print("  ")
        }
    }
    fmt.Println()
}

func rule110(left, center, right int) int {
    pattern := (left << 2) | (center << 1) | right
    switch pattern {
    case 1, 2, 3, 5, 6:
        return 1
    default:
        return 0
    }
}

func main() {
    size := 50
    generations := size

    cells := make([]int, size)
    cells[size-1] = 1

    printCells(cells)

    for gen := 0; gen < generations; gen++ {
        nextGen := make([]int, size)
        for i := 0; i < size; i++ {
            left, center, right := 0, cells[i], 0
            if i > 0 {
                left = cells[i-1]
            }
            if i < size-1 {
                right = cells[i+1]
            }
            nextGen[i] = rule110(left, center, right)
        }
        printCells(nextGen)
        cells = nextGen
    }
}

