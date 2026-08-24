// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/global/import_statement.h"

namespace bibblec::parser {
    ImportStatement::ImportStatement(scope::Scope* scope, std::vector<std::string> module, SourcePair source)
        : ASTNode(scope, source)
        , mModule(std::move(module)) {}

    const std::vector<std::string>& ImportStatement::getModule() const {
        return mModule;
    }

    bibblir::Value* ImportStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        return nullptr;
    }

    void ImportStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }
}
