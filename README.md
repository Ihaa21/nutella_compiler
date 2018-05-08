# Nutella Compiler

This repository contains a compiler which translates a subset of C to x86 assembly implemented in C++. The program is built for visual studio with the .sln files stored in build_win32.

Build: To build the program, open a command promt and navigate to the build directory. Run build.bat and the project should be built.

Program Entry Point: The main entry point for the program is found at nutella_compiler.cpp. 

Controls: At the top of the main function in nutella_compiler, you can specify which .nutella file you want to compile. Nutella files are files with C code and a WinMain function as their entry point. You can find reference nutella files in the data folder. The program will output a file with x86 assembly in it, that then must go through an assembler to become a executable.
