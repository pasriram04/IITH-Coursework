# RISC-V Assembler

## Overview
This project provides a simple RISC-V assembler that converts high-level RISC-V instructions into low-level hexadecimal code. The project consists of the following files:

1. **Makefile**: Used to compile the code and generate an executable named `riscv_asm`.
2. **main.cpp**: The main program that performs the instruction conversion.
3. **input.s**: A test input file containing RISC-V instructions (replace this with your own test cases).
4. **report.pdf**: A report detailing the implementation and testing approaches.

## Usage Instructions

### Compilation
To compile the program, use the following command:
```bash
make
```

### Running the Program
To run the program and test the assembler using `input.s`, use:
```bash
make run
```
The output would be a file named `output.hex`.

### Input Format
The input should be provided in a file, `input.s`. The program expects commas or spaces to separate out the operands and instructions. If the format is incorrect, the program will terminate and display an error.

## Error Handling
In case of a syntax error in the RISC-V code (e.g., missing commas, incorrect instruction format), the program will terminate and output an error message to the terminal, helping you debug the issue.

## Report
For more details on the implementation and testing methodology, or executing code in other environments, refer to `report.pdf`.