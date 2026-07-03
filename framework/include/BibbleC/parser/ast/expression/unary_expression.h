// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_UNARY_EXPRESSION_H
#define BIBBLEC_PARSER_AST_EXPRESSION_UNARY_EXPRESSION_H

#include "BibbleC/lexer/Token.h"

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class UnaryExpression : public ASTNode {
    public:
        enum Operator {
            Neg,
        };

        UnaryExpression(scope::Scope* scope, ASTNodePtr operand, lexer::Token operatorToken, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mOperand;
        Operator mOperator;
        lexer::Token mOperatorToken;
    };

    using UnaryExpressionPtr = std::unique_ptr<UnaryExpression>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_UNARY_EXPRESSION_H
