// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_STATEMENT_COMPOUND_STATEMENT_H
#define BIBBLEC_PARSER_AST_STATEMENT_COMPOUND_STATEMENT_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT CompoundStatement : public ASTNode {
    public:
        CompoundStatement(std::vector<ASTNodePtr> body, scope::ScopePtr ownScope, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::vector<ASTNodePtr> mBody;
        scope::ScopePtr mOwnScope;
    };

    using CompoundStatementPtr = std::unique_ptr<CompoundStatement>;
}

#endif //BIBBLEC_PARSER_AST_STATEMENT_COMPOUND_STATEMENT_H
