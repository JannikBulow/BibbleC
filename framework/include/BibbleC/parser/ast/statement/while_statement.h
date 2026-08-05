// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_STATEMENT_WHILE_STATEMENT_H
#define BIBBLEC_PARSER_AST_STATEMENT_WHILE_STATEMENT_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT WhileStatement : public ASTNode {
    public:
        WhileStatement(ASTNodePtr condition, ASTNodePtr body, scope::Scope* scope, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mCondition;
        ASTNodePtr mBody;
    };

    using WhileStatementPtr = std::unique_ptr<WhileStatement>;
}

#endif //BIBBLEC_PARSER_AST_STATEMENT_WHILE_STATEMENT_H
