// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_STATEMENT_FOR_STATEMENT_H
#define BIBBLEC_PARSER_AST_STATEMENT_FOR_STATEMENT_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT ForStatement : public ASTNode {
    public:
        ForStatement(ASTNodePtr init, ASTNodePtr condition, ASTNodePtr it, ASTNodePtr body, scope::Scope* scope, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mInit;
        ASTNodePtr mCondition;
        ASTNodePtr mIt;
        ASTNodePtr mBody;
    };

    using ForStatementPtr = std::unique_ptr<ForStatement>;
}

#endif //BIBBLEC_PARSER_AST_STATEMENT_FOR_STATEMENT_H
