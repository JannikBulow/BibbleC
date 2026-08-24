// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/compiler.h"

#include <BibbleBytecode/buffer.h>
#include <BibbleBytecode/writer.h>

#include <BibbleC/lexer/lexer.h>

#include <BibbleC/parser/parser.h>

#include <BibblIR/pass/codegen/codegen.h>

#include <BibblIR/pass/optimizer/constant_folding.h>
#include <BibblIR/pass/optimizer/peephole.h>

#include <BibblIR/pass/pass_manager.h>

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
            doImports(mModules[file.input]);
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
        auto& tokens = module.tokens;

        if (tokens[0].getTokenType() == lexer::TokenType::ModuleKeyword) {
            if (tokens[1].getTokenType() == lexer::TokenType::Identifier) {
                int pos = 1;
                std::string moduleName;
                while (tokens[pos].getTokenType() == lexer::TokenType::Identifier) {
                    moduleName += tokens[pos++].getText();
                    if (tokens[pos].getTokenType() == lexer::TokenType::Semicolon) break;

                    if (tokens[pos].getTokenType() == lexer::TokenType::Dot) {
                        pos++;
                        moduleName += '.';
                    }
                }
                mModuleFiles[moduleName].push_back(module.path);
                module.name = std::move(moduleName);
            } else {
                mDiag.reportCompilerError(
                    tokens[1].getStartLocation(),
                    tokens[1].getEndLocation(),
                    std::format("expected module name after '{}module{}' keyword", fmt::bold, fmt::reset));
                std::exit(1);
            }
        } else {
            mDiag.reportCompilerError(
                    tokens[0].getStartLocation(),
                    tokens[0].getEndLocation(),
                    std::format("expected '{}module{}' keyword", fmt::bold, fmt::reset));
            std::exit(1);
        }
    }

    void Compiler::parse(Module& module) {
        module.globalScope = std::make_unique<scope::Scope>(module.name);

        parser::Parser parser(module.tokens, mDiag, module.globalScope.get(), "");

        module.ast = parser.parse();
    }

    void Compiler::doImports(Module& module) {
        auto& ast = module.ast;

        std::vector<std::string> modules{module.name};
        for (auto& node : ast) {
            if (auto* import = dynamic_cast<parser::ImportStatement*>(node.get())) {
                std::string moduleName;
                for (auto& name : import->getModule()) {
                    moduleName += name;
                    moduleName += '.';
                }
                moduleName.pop_back();
                modules.push_back(moduleName);

                if (!mModuleFiles.contains(moduleName) && !mImportedModules.contains(moduleName)) {
                    mDiag.reportCompilerError(import->getSource(),
                        std::format("could not find module '{}{}{}'",
                            fmt::bold, moduleName, fmt::reset));
                    std::exit(1);
                }
            }
        }

        for (auto& moduleName : modules) {
            for (auto& file : mModuleFiles[moduleName]) {
                if (file != module.path) {
                    auto& ast = mModules[file].ast;

                    for (auto& node : ast) {
                        if (auto cloned = node->cloneExternal(module.globalScope.get())) {
                            module.ast.insert(module.ast.begin(), std::move(cloned));
                        }
                    }
                }
            }

            if (mImportedModules.contains(moduleName)) {
                for (auto& func : mImportedModules[moduleName]) {
                    if (auto cloned = func->cloneExternal(module.globalScope.get())) {
                        module.ast.insert(module.ast.begin(), std::move(cloned));
                    }
                }
            }
        }
    }

    void Compiler::compileModule(Module& module, std::filesystem::path outputFilePath) {
        module.module = bibblir::Module(module.name);

        bibblir::IRBuilder builder(module.module);

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

        std::cout << "\n\n";

        bibblir::PassRegistry passRegistry = bibblir::PassRegistry::Default();
        bibblir::PassManager passManager(passRegistry);
        passManager.addPass(passRegistry.create(bibblir::GetPassID<bibblir::ConstantFoldingPass>()));
        passManager.addPass(passRegistry.create(bibblir::GetPassID<bibblir::CodegenPass>()));
        passManager.addPass(passRegistry.create(bibblir::GetPassID<bibblir::PeepholePass>()));

        passManager.buildPipeline().run(module.module);

        std::cout << "\n\n";

        bibbleasm::Module builtModule = module.module.bytecodeModule().build();

        bibblebytecode::WritableByteBuffer buffer;
        if (!bibblebytecode::writer::WriteModule(buffer, builtModule.module())) {
            std::exit(1);
        }

        std::ofstream outputFile(outputFilePath, std::ios::binary);
        buffer.emit(outputFile);
    }
}
