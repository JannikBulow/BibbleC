// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/break_statement.h"

namespace bibblec::parser {
    BreakStatement::BreakStatement(std::string label, scope::Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mLabel(std::move(label)) {}

    bibblir::Value* BreakStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::BasicBlock* bb = mScope->getBreakBB(mLabel);
        if (!bb) {
            diag.reportCompilerError(mSource, "break statement not within a loop");
            std::exit(1);
        }
        builder.createBr(bb);
        return nullptr;
    }

    void BreakStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }
}
