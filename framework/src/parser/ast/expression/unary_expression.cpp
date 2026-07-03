// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/unary_expression.h"

#include <BibblIR/ir/instruction/unary_instruction.h>

namespace bibblec::parser {
    UnaryExpression::UnaryExpression(scope::Scope* scope, ASTNodePtr operand, lexer::Token operatorToken, SourcePair source)
        : ASTNode(scope, source)
        , mOperand(std::move(operand))
        , mOperatorToken(std::move(operatorToken)) {
        switch (mOperatorToken.getTokenType()) {
            case lexer::TokenType::Minus:
                mOperator = Neg;

            default:
                break;
        }
    }

    std::vector<ASTNode*> UnaryExpression::getChildren() {
        return {mOperand.get()};
    }

    bibblir::Value* UnaryExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Value* operand = mOperand->codegen(builder, module, diag);

        switch (mOperator) {
            case Neg:
                return builder.createNeg(operand);
        }

        return nullptr;
    }

    void UnaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mOperand->typeCheck(diag, exit);

        switch (mOperator) {
            case Neg:
                if (!mOperand->getType()->isIntegerType()) {
                    diag.reportCompilerError(mSource,
                            std::format("No match for '{}operator{}{}' with type '{}{}{}'",
                                fmt::bold, mOperatorToken.getName(), fmt::reset,
                                fmt::bold, mOperand->getType()->getName(), fmt::reset)
                        );
                    exit = true;
                    mType = Type::Get("error-type");
                } else {
                    mType = mOperand->getType();
                }
                break;
        }
    }
}
