// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/compiler.h"

#include <BibbleBytecode/buffer.h>
#include <BibbleBytecode/writer.h>

#include <BibbleC/lexer/Lexer.h>

#include <BibbleC/parser/parser.h>

#include <BibblIR/visitor/codegen_visitor.h>
#include <BibblIR/visitor/print_visitor.h>

#include <algorithm>
#include <fstream>

namespace bibblec {
    Compiler::Compiler(diagnostic::Diagnostics& diag)
        : mDiag(diag) {}

    void Compiler::addFile(std::filesystem::path inputFile, std::filesystem::path outputFile) {
        mFiles.emplace_back(std::move(inputFile), std::move(outputFile));
    }

    void Compiler::compile() {
        Type::Init();

        compileModules();
    }

    void Compiler::compileModules() {
        for (FilePair& file : mFiles) {
            std::filesystem::create_directories(file.output.parent_path());

            Module& module = mModules[file.input];
            module.path = file.input;
            module.pathString = file.input.string();

            lex(module);
            parseModuleName(module);
            parse(module);
        }

        for (FilePair& file : mFiles) {
            compileModule(mModules[file.input], file.output);
        }
    }

    void Compiler::lex(Module& module) {
        std::ifstream inputFile(module.path);
        std::stringstream buffer;
        buffer << inputFile.rdbuf();

        module.text = std::move(buffer).str();

        inputFile.close();

        lexer::Lexer lexer(module.text, module.pathString);
        module.tokens = lexer.lex();

        mDiag.addText(module.pathString, module.text);
    }

    void Compiler::parseModuleName(Module& module) {
        module.name = module.path.filename().replace_extension("").string();
    }

    void Compiler::parse(Module& module) {
        module.globalScope = std::make_unique<scope::Scope>();

        parser::Parser parser(module.tokens, mDiag, module.globalScope.get());

        module.ast = parser.parse();
    }

    void Compiler::compileModule(Module& module, std::filesystem::path outputFilePath) {
        module.module = bibblir::Module(module.name);

        bibblir::IRBuilder builder;

        bool exit = false;
        for (auto& node : module.ast) {
            node->typeCheck(mDiag, exit);
        }
        if (exit) std::exit(1);

        for (auto& node : module.ast) {
            node->setEmittedValue(builder, module.module, mDiag);
        }

        for (auto& node : module.ast) {
            node->codegen(builder, module.module, mDiag);
        }

        bibblir::PrintVisitor printer(std::cout);
        module.module.accept(printer);

        bibblir::CodegenVisitor codegen;
        module.module.accept(codegen);

        bibbleasm::Module builtModule = codegen.buildModule();

        bibblebytecode::WritableByteBuffer buffer;
        if (!bibblebytecode::writer::WriteModule(buffer, builtModule.module())) {
            std::exit(1);
        }

        std::ofstream outputFile(outputFilePath, std::ios::binary);
        buffer.emit(outputFile);
    }
}
