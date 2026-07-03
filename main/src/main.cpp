// Copyright 2026 Jannik Laugmand Bülow

#include <BibbleC/compiler.h>

#include <iostream>

using namespace bibblec;

int main(int argc, char** argv) {
    diagnostic::Diagnostics diag(std::cerr, std::cerr);
    Compiler compiler(diag);

    for (int i = 1; i < argc; i++) {
        std::filesystem::path inputFile = argv[i];
        std::filesystem::path outputFile = inputFile;
        outputFile.replace_extension(".bmod");

        compiler.addFile(std::move(inputFile), std::move(outputFile));
    }

    compiler.compile();

    return 0;
}
