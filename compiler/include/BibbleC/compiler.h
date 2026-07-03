// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_COMPILER_H
#define BIBBLEC_COMPILER_H

#include "BibbleC/module.h"

#include "BibbleC/api2.h"

#include <unordered_map>
#include <vector>

namespace bibblec {
    class BIBBLE_EXPORT Compiler {
    public:
        explicit Compiler(diagnostic::Diagnostics& diag);

        void addFile(std::filesystem::path inputFile, std::filesystem::path outputFile);

        void compile();

    private:
        struct FilePair {
            std::filesystem::path input;
            std::filesystem::path output;
        };

        diagnostic::Diagnostics& mDiag;

        std::unordered_map<std::filesystem::path, Module> mModules;
        std::vector<FilePair> mFiles;

        void compileModules();

        void lex(Module& module);
        void parseModuleName(Module& module);
        void parse(Module& module);
        void compileModule(Module& module, std::filesystem::path outputFilePath);
    };
}

#endif //BIBBLEC_COMPILER_H
