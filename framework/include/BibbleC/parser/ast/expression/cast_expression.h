// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_CAST_EXPRESSION_H
#define BIBBLEC_PARSER_AST_EXPRESSION_CAST_EXPRESSION_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class CastExpression : public ASTNode {
    public:
        CastExpression(scope::Scope* scope, ASTNodePtr value, Type* destType, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mValue;
    };

    using CastExpressionPtr = std::unique_ptr<CastExpression>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_CAST_EXPRESSION_H
