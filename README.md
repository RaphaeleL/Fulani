# Fulani Programming Language

Fulani is a simple C-like programming language with a straightforward syntax, designed for educational purposes. It includes essential programming constructs like variables, functions, conditions, and loops.

## Building and Running

To build the Fulani interpreter:

```bash
make clean && make
```

To run a program:

```bash
./ownlang path/to/your/program.fu
```

To run in debug mode (shows execution details):

```bash
./ownlang --debug path/to/your/program.fu
```

## Language Features

### Data Types

- `int`: Integer values
- `float`: Floating-point values
- `string`: Text strings
- `void`: Used for functions that don't return a value

### Comments

- Single-line comments: `// This is a comment`
- Multi-line comments: `/* This is a multi-line comment */`
- Nested comments are supported: `/* Outer comment /* Inner comment */ still outer */`

### Variables

Variables must be declared with a type and can be initialized in the declaration:

```
int x = 10;
float y = 3.14;
string message = "Hello, World!";
```

Multiple variables of the same type can be declared together:

```
int a, b, c;
```

### Operators

- Arithmetic: `+`, `-`, `*`, `/`, `%` (modulo)
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Assignment: `=`

### Control Flow

**Conditional statements**:

```
if (condition) {
    // code
} else {
    // code
}
```

**Loops**:

```
while (condition) {
    // code
}
```

### Functions

Functions are defined with a return type, name, parameters, and body:

```
int add(int a, int b) {
    return a + b;
}
```

The `main` function is the entry point for execution:

```
void main() {
    // Your program starts here
}
```

### Built-in Functions

- `print(value1, value2, ...)`: Prints values without a newline
- `println(value1, value2, ...)`: Prints values followed by a newline

## Examples

### Fibonacci Sequence

```
int fib(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    int a = 0;
    int b = 1;
    int i = 2;
    int result = 0;

    while (i <= n) {
        result = a + b;
        a = b;
        b = result;
        i = i + 1;
    }

    return b;
}

void main() {
    int i = 0;
    while (i <= 10) {
        println(i, ": ", fib(i));
        i = i + 1;
    }
}
```

### Prime Number Check

```
int is_prime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    
    int i = 2;
    while (i * i <= n) { 
        if (n % i == 0) {
            return 0;
        }
        i = i + 1;
    }
    
    return 1;
}

void main() {
    int i = 1;
    while (i <= 20) {
        if (is_prime(i)) {
            println(i, " is prime");
        } else {
            println(i, " is not prime");
        }
        i = i + 1;
    }
}
```

## Language Implementation

The language implementation consists of several components:

- **Lexer**: Converts source code into tokens
- **Parser**: Builds an Abstract Syntax Tree (AST) from tokens
- **Interpreter**: Executes the AST

Each phase performs specific checks and transformations to ensure the program is valid and can be executed correctly.

## Future Improvements

Potential enhancements for the language include:

- Array support
- For loops
- User-defined types/structures
- More built-in functions
- Standard library

## License

This project is available for educational purposes.

