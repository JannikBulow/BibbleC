// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_BINARY_EXPRESSION_H
#define BIBBLEC_PARSER_AST_EXPRESSION_BINARY_EXPRESSION_H

#include "BibbleC/lexer/token.h"

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT BinaryExpression : public ASTNode {
    public:
        enum Operator {
            Add,
            Sub,
            Mul,
            Div,
            Mod,

            Assign,
            AddAssign,
            SubAssign,
            MulAssign,
            DivAssign,
            ModAssign,
        };

        BinaryExpression(scope::Scope* scope, ASTNodePtr left, lexer::Token operatorToken, ASTNodePtr right, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mLeft;
        ASTNodePtr mRight;
        Operator mOperator;
        lexer::Token mOperatorToken;
    };

    using BinaryExpressionPtr = std::unique_ptr<BinaryExpression>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_BINARY_EXPRESSION_H
