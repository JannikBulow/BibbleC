// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_MODULE_H
#define BIBBLEC_MODULE_H

#include <BibbleC/lexer/token.h>

#include <BibbleC/parser/ast/node.h>

#include <BibbleC/scope/scope.h>

#include <BibblIR/module.h>

#include <filesystem>

namespace bibblec {
    struct Module {
        std::filesystem::path path;
        std::string pathString;

        std::string text;

        std::string name;

        std::vector<lexer::Token> tokens;
        std::vector<parser::ASTNodePtr> ast;

        scope::ScopePtr globalScope;

        bibblir::Module module{"error"};
    };
}

#endif //BIBBLEC_MODULE_H
