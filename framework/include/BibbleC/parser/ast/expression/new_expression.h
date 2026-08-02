// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_NEW_EXPRESSION_H
#define BIBBLEC_PARSER_AST_EXPRESSION_NEW_EXPRESSION_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class NewExpression : public ASTNode {
    public:
        NewExpression(scope::Scope* scope, Type* allocatedType, std::vector<ASTNodePtr> parameters, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::vector<ASTNodePtr> mParameters;

        scope::Symbol* mBestViableConstructor;

        scope::Symbol* getBestViableConstructor(diagnostic::Diagnostics& diag);
    };

    using NewExpressionPtr = std::unique_ptr<NewExpression>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_NEW_EXPRESSION_H
