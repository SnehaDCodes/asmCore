# IA-32 Opcode Analyzer

## Team Name

AsmCore

## Team Members

- Sneha Deshmukh - 25111055
- Abhishek Patare - 25111001
- Karan Ahire - 25111004
- Sangam Nirmal - 25111038

## GitHub Repository

[https://github.com/SnehaDCodes/asmCore.git](https://github.com/SnehaDCodes/asmCore.git)

## Introduction

This project is an IA-32 (32-bit) opcode analyzer developed using C++.

The program takes two input files:

1. Opcode file
2. Assembly program file

The opcode file contains information about IA-32 instructions and their opcodes. The program reads the mnemonics from this file and stores them in a Binary Search Tree (BST).

The assembly program file contains assembly instructions. The program reads the instructions and identifies the mnemonic and operands used in each instruction.

## Objectives

The main objectives of this project are:

- Read the opcode table from a file.
- Store instruction mnemonics in a Binary Search Tree.
- Display the mnemonics.
- Display the Binary Search Tree.
- Search for a particular mnemonic.
- Read an assembly program from a file.
- Identify operands used in the assembly program.
- Identify the type of each operand.

## Data Structure Used

A Binary Search Tree is used to store the instruction mnemonics.

Each node contains:

- Mnemonic
- Left pointer
- Right pointer

The mnemonics are inserted alphabetically into the tree.

## Operand Types

The program identifies the following types of operands:

| Operand             | Type     |
| ------------------- | -------- |
| EAX, EBX, ECX, etc. | Register |
| 10, 20, -5, etc.    | Constant |
| [EAX], [EBX], etc.  | Memory   |
| VALUE, LABEL, etc.  | Symbol   |

The registers supported by the program are:

```text
EAX
ECX
EDX
EBX
ESP
EBP
ESI
EDI
```

## Program Features

### 1. Display Mnemonics

Displays all mnemonics stored in the Binary Search Tree using inorder traversal.

### 2. Display BST

Displays the structure of the Binary Search Tree.

### 3. Search Mnemonic

The user can enter a mnemonic and check whether it is present in the opcode table.

Example:

```text
Enter mnemonic to search: MOV

MOV found.
```

If the mnemonic is not present:

```text
Enter mnemonic to search: PUSH

PUSH not found.
```

### 4. Display Operands and Their Types

The program reads the assembly program and displays the operands along with their types.

Example:

```text
Line 1: MOV EAX, EBX
Mnemonic: MOV
EAX -> Register
EBX -> Register
```

## Input Files

The program requires two input files.

### Opcode File

The opcode file contains the IA-32 instruction and encoding information.

It includes instructions such as:

```text
MOV
ADD
SUB
AND
OR
XOR
CMP
NOT
MUL
IMUL
DIV
INC
DEC
JMP
JZ
JNZ
CALL
RET
```

### Assembly File

The assembly file contains the sample assembly program that needs to be analyzed.

## How to Compile

Open the terminal in the project folder and run:

```bash
g++ opcode.cpp -o opcode
```

For C++11:

```bash
g++ -std=c++11 opcode.cpp -o opcode
```

## How to Run

Run the program using:

```bash
./opcode
```

The program will ask for the opcode file name:

```text
Enter opcode file name:
```

Enter the opcode file name, for example:

```text
opcode_data.txt
```

Then enter the assembly file name:

```text
Enter assembly program file name:
```

For example:

```text
assembly.asm
```

## Menu

After entering the files, the following menu is displayed:

```text
1. Display Mnemonics
2. Display BST
3. Search Mnemonic
4. Display Operands and Their Types
5. Exit
```

Enter the required option number.

## Project Structure

```text
AsmCore/
│
├── README.md
├── opcode_data.txt
├── assembly.asm
└── opcode.cpp
```

## Technologies Used

- C++
- Binary Search Tree
- File Handling
- String Processing
- Assembly Language
- IA-32 Instruction Set
- Ubuntu/Linux

## Conclusion

This project helped us understand the basic concepts of IA-32 instructions, opcode tables, assembly programs and Binary Search Trees.
