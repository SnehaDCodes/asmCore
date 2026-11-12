AsmCore - IA-32 Opcode Analyzer

Team Members

Sneha Deshmukh (25111055)
Abhishek Patare (25111001)
Karan Ahire (25111004)
Sangam Nirmal (25111038)

Team Name: AsmCore

GitHub Repository:https://github.com/SnehaDCodes/asmCore.git

About the Project
This project is based on IA-32 (32-bit) assembly language. It is developed in C++.
The program takes an opcode data file and an assembly program file as input. It reads the instructions from the opcode file and
stores the mnemonics in a Binary Search Tree (BST). It then reads the assembly program and checks its instructions and operands.
The project also includes a one-pass assembler part for generating object-code output for the supported instructions.
What the Program Does
Reads the opcode information from a text file.
Stores instruction mnemonics in a BST.
Displays the mnemonics in sorted order.
Displays the BST.
Searches for a given mnemonic.
Reads an assembly program from a file.
Identifies operands and their types.
Displays the symbol table.
Runs the one-pass assembler.
Displays macro tables and expanded output.
Operand Types
The program identifies these operand types:
Example Type
 EAX, EBX, ECX Register 10, 20, -5 Constant [EAX], [EBX] Memory VALUE, LABEL Symbol
Input Files
The program needs two input files:
opcode_data.txt - contains the opcode table.
assembly.asm - contains the assembly instructions to be checked.
Enter the correct file names when the program asks for them.
How to Compile
Open the terminal in the folder containing the C++ file and compile it using:
g++ -std=c++11 Assignment7.cpp -o asmcore
 How to Run
On Linux:
./asmcore
 On Windows:.\asmcore.exe
 The program will ask for the opcode file name and the assembly program file name.
Example:
Enter opcode file name: opcode_data.txt
Enter assembly file name: assembly.asm
 Menu Options
The program provides the following options:
1. Display Mnemonics
2. Display BST
3. Search Mnemonic
4. Display Operands and Their Types
5. Display Symbol Table
6. Run One-Pass Assembler
7. Display Macro Tables (MNT / MDT / Expansion)
8. Exit
 Choose an option by entering its number.
Output Files
The assembler generates these files:
object.txt - contains the generated object-code output.
error.txt - contains errors found while assembling the program.
Technologies Used
C++
IA-32 Assembly Language
Binary Search Tree
File Handling
String Processing
Conclusion
Through this project, we worked with opcode tables, assembly instructions, operands and Binary Search Trees. We also implemented
parts of a one-pass assembler and macro processing
