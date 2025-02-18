# C-Lisp Interpreter

This is a simple Lisp interpreter written in C.

## Features

- Supports basic arithmetic operations: addition, subtraction, multiplication, and division.
- Supports variable and function definitions.
- Supports nested compound expressions.

## File Structure

- `interpreter.c`: Main interpreter code.
- `error.h`, `ast.h`, `env.h`, `proc.h`: Header files defining error handling, abstract syntax tree, environment, and procedure-related structures and functions.
- `parser.c`: Parser code.

## Compilation and Execution

1. Compile the code using the following command:

    ```sh
    gcc -o lisp_interpreter interpreter.c
    ```

2. Run the interpreter:

    ```sh
    ./lisp_interpreter
    ```

## Usage

Once the interpreter is running, you can input Lisp expressions to evaluate or define variables and functions. For example:

- Evaluate an expression:

    ```lisp
    (+ 1 2 3)
    ```

- Define a variable:

    ```lisp
    (define x 10)
    ```

- Define a function:

    ```lisp
    (define (square x) (* x x))
    ```

- Call a function:

    ```lisp
    (square 5)
    ```

Type `exit` to quit the interpreter.

## Examples

Here are some example inputs and outputs:

```lisp
> (+ 1 2 3)
6

> (define x 10)
> x
10

> (define (square x) (* x x))
> (square 5)
25
```

## Contribution
Feel free to submit issues and contribute code!

## License
This project is licensed under the MIT License.