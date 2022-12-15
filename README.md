# Albatross Compiler

A compiler for the ***albatross programming language*** developed in CS473: Compiler Design - Fall 22.

The compiler is written in C which compiles the albatross code into MIPS instructions.

The compiler works in the following stages:

1. Lexing: extracting tokens (Uses flex)
  
2. Parsing: checking program syntax (Uses yacc)
  
  - AST generation
    
3. Semantic analysis
  
  - Symbol resolution
    
  - Type checking
    
4. AST transformations

5. MIPS instruction generation.
  

## Usage

To create the executable of the compiler use `make`. This should generate an executable named `albatrosscc`.

Then use the `albatrosscc` to compile your program.

```bash
./albatrosscc <in.albatross> <out.mips>
```
`in.albatross` is some file that contains your albatross program, and `out.mips` is the file where the generated mips instructions would be written.


## Testing

Run the `runtests.sh` script to run the tests contained within the `tests` directory in the project root.

## Links

Checkout the interpreter written for albatross [here](https://github.com/yashkurkure/albatross_interpreter).

