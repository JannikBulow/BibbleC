// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/binary_expression.h"

#include "BibbleC/type/integer_type.h"

#include <BibblIR/ir/instruction/binary_instruction.h>

namespace bibblec::parser {
    BinaryExpression::BinaryExpression(scope::Scope* scope, ASTNodePtr left, lexer::Token operatorToken, ASTNodePtr right, SourcePair source)
        : ASTNode(scope, source)
        , mLeft(std::move(left))
        , mRight(std::move(right))
        , mOperatorToken(std::move(operatorToken)) {
        switch (mOperatorToken.getTokenType()) {
            case lexer::TokenType::Plus:
                mOperator = Add;
                break;
            case lexer::TokenType::Minus:
                mOperator = Sub;
                break;
            case lexer::TokenType::Star:
                mOperator = Mul;
                break;
            case lexer::TokenType::Slash:
                mOperator = Div;
                break;
            case lexer::TokenType::Percent:
                mOperator = Mod;
                break;

            default:
                break; // maybe add an unreachable() thing here
        }
    }

    std::vector<ASTNode*> BinaryExpression::getChildren() {
        return {mLeft.get(), mRight.get()};
    }

    bibblir::Value* BinaryExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Value* left = mLeft->codegen(builder, module, diag);
        bibblir::Value* right = mRight->codegen(builder, module, diag);

        switch (mOperator) {
            case Add:
                return builder.createAdd(left, right);
            case Sub:
                return builder.createSub(left, right);
            case Mul:
                return builder.createMul(left, right);
            case Div:
                if (mType->isIntegerType()) {
                    if (static_cast<IntegerType*>(mType)->isSigned()) {
                        return builder.createSDiv(left, right);
                    }
                    return builder.createUDiv(left, right);
                }

                diag.reportCompilerError(mSource, "Can't divide a non-integer type");
                std::exit(1);
            case Mod:
                if (mType->isIntegerType()) {
                    if (static_cast<IntegerType*>(mType)->isSigned()) {
                        return builder.createSMod(left, right);
                    }
                    return builder.createUMod(left, right);
                }

                diag.reportCompilerError(mSource, "Can't divide a non-integer type"); // this probably shouldn't say divide, but idk what the verb for modulo is
                std::exit(1);
        }

        return nullptr;
    }

    void BinaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mLeft->typeCheck(diag, exit);
        mRight->typeCheck(diag, exit);

        switch (mOperator) {
            case Add:
            case Sub:
            case Mul:
            case Div:
            case Mod:
                if (mLeft->getType() != mRight->getType() && mLeft->getType()->isIntegerType() && mRight->getType()->isIntegerType()) {
                    if (mLeft->getType()->getSize() > mRight->getType()->getSize()) {
                        if (mRight->canImplicitCast(diag, mLeft->getType())) {
                            mRight = CastTo(mRight, mLeft->getType());
                        }
                    } else {
                        if (mLeft->canImplicitCast(diag, mRight->getType())) {
                            mLeft = CastTo(mLeft, mRight->getType());
                        }
                    }
                }
                if (mLeft->getType() != mRight->getType() || !mLeft->getType()->isIntegerType()) {
                    diag.reportCompilerError(mSource,
                        std::format("No match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                            fmt::bold, mOperatorToken.getName(), fmt::reset,
                            fmt::bold, mLeft->getType()->getName(), fmt::reset,
                            fmt::bold, mRight->getType()->getName(), fmt::reset)
                    );
                    exit = true;
                }
                mType = mLeft->getType();
                break;
        }
    }
}
