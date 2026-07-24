// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/compound_statement.h"

namespace bibblec::parser {
    CompoundStatement::CompoundStatement(std::vector<ASTNodePtr> body, scope::ScopePtr ownScope, SourcePair source)
        : ASTNode(ownScope->getParent(), source)
        , mBody(std::move(body))
        , mOwnScope(std::move(ownScope)) {}

    std::vector<ASTNode*> CompoundStatement::getChildren() {
        std::vector<ASTNode*> children;
        for (auto& node : mBody) {
            children.push_back(node.get());
        }
        return children;
    }

    bibblir::Value* CompoundStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        for (auto& node : mBody) {
            node->codegen(builder, module, diag);
        }
        return nullptr;
    }

    void CompoundStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& node : mBody) {
            node->typeCheck(diag, exit);
        }
    }
}
