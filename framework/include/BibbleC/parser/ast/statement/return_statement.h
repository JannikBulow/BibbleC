// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_STATEMENT_RETURN_STATEMENT_H
#define BIBBLEC_PARSER_AST_STATEMENT_RETURN_STATEMENT_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class ReturnStatement : public ASTNode {
    public:
        ReturnStatement(scope::Scope* scope, ASTNodePtr returnValue, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mReturnValue;
    };

    using ReturnStatementPtr = std::unique_ptr<ReturnStatement>;
}

#endif //BIBBLEC_PARSER_AST_STATEMENT_RETURN_STATEMENT_H
