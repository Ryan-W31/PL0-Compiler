# PL0-Compiler

This is a class project for a Systeems Software course at UCF. It is a functional compiler and virtual machine for the PL/0 language. This project was completed using the C language.

# How To Use
1) Using your terminal or another method, clone the repo using ```git clone https://github.com/Ryan-W31/PL0-Compiler.git```
2) Using your terminal, navigate into the repo on your local device. Ensure you have a GCC compiler on your device in order to run the C programs.
3) Once you have a C compiler, use the following commands:
   1) ```gcc compiler.c```
   2) On macOS/Linux: ```./a.out valid.txt``` | On Windows: ```./a.exe valid.txt```
4) There should now be output in the terminal as well as the elf.txt file.
5) Now use these commands:
   1) ```gcc vm.c```
   2) On macOS/Linux: ```./a.out elf.txt``` | On Windows: ```./a.exe elf.txt```
6) There should now be output in ther minal representing the call stack and activation record(s).
