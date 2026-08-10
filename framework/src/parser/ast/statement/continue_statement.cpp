// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/continue_statement.h"

namespace bibblec::parser {
    ContinueStatement::ContinueStatement(std::string label, scope::Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mLabel(std::move(label)) {}

    bibblir::Value* ContinueStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::BasicBlock* bb = mScope->getContinueBB(mLabel);
        if (!bb) {
            diag.reportCompilerError(mSource, "continue statement not within a loop");
            std::exit(1);
        }
        builder.createBr(bb);
        return nullptr;
    }

    void ContinueStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }
}
