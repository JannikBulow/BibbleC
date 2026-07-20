// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/binary_expression.h"
#include "BibbleC/parser/ast/expression/variable_expression.h"

#include "BibbleC/type/integer_type.h"

#include <BibblIR/ir/instruction/binary_instruction.h>
#include <BibblIR/ir/instruction/load_instruction.h>

namespace bibblec::parser {
    BinaryExpression::BinaryExpression(scope::Scope* scope, ASTNodePtr left, lexer::Token operatorToken, ASTNodePtr right, SourcePair source)
        : ASTNode(scope, std::move(source))
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
            case lexer::TokenType::Equal:
                mOperator = Assign;
                break;
            case lexer::TokenType::PlusEqual:
                mOperator = AddAssign;
                break;
            case lexer::TokenType::MinusEqual:
                mOperator = SubAssign;
                break;
            case lexer::TokenType::StarEqual:
                mOperator = MulAssign;
                break;
            case lexer::TokenType::SlashEqual:
                mOperator = DivAssign;
                break;
            case lexer::TokenType::PercentEqual:
                mOperator = ModAssign;
                break;

            default:
                break; // maybe add an unreachable() thing here
        }
    }

    std::vector<ASTNode*> BinaryExpression::getChildren() {
        return {mLeft.get(), mRight.get()};
    }

    bibblir::Value* BinaryExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        auto createAssign = [&](bibblir::Value* left, bibblir::Value* right, bool eraseLeft = true) -> bibblir::Value* {
            if (auto variableExpression = dynamic_cast<VariableExpression*>(mLeft.get())) {
                scope::Symbol* symbol = mScope->resolveSymbol(variableExpression->getName());

                if (symbol->constant) {
                    diag.reportCompilerError(mSource, "attempted mutation of constant");
                    std::exit(1);
                }

                symbol->values.push_back({builder.getInsertPoint(), right});
                return right;
            } else if (auto load = dynamic_cast<bibblir::LoadInstruction*>(left)) {
                bibblir::Value* variableOperand = bibblir::GetVariableOperand(load);
                if (eraseLeft) load->eraseFromParent();

                builder.createStore(variableOperand, right);

                return right;
            }

            return nullptr;
        };

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

                diag.reportCompilerError(mSource, "can't divide a non-integer type");
                std::exit(1);
            case Mod:
                if (mType->isIntegerType()) {
                    if (static_cast<IntegerType*>(mType)->isSigned()) {
                        return builder.createSMod(left, right);
                    }
                    return builder.createUMod(left, right);
                }

                diag.reportCompilerError(mSource, "can't divide a non-integer type"); // this probably shouldn't say divide, but idk what the verb for modulo is
                std::exit(1);
            case Assign:
                return createAssign(left, right);
            case AddAssign: {
                auto add = builder.createAdd(left, right);
                return createAssign(left, add, false);
            }
            case SubAssign: {
                auto sub = builder.createSub(left, right);
                return createAssign(left, sub, false);
            }
            case MulAssign: {
                auto mul = builder.createMul(left, right);
                return createAssign(left, mul, false);
            }
            case DivAssign: {
                bibblir::BinaryInstruction* div = nullptr;
                if (mType->isIntegerType()) {
                    if (static_cast<IntegerType*>(mType)->isSigned()) {
                        div = builder.createSDiv(left, right);
                    } else {
                        div = builder.createUDiv(left, right);
                    }
                }
                return createAssign(left, div, false);
            }
            case ModAssign: {
                bibblir::BinaryInstruction* mod = nullptr;
                if (mType->isIntegerType()) {
                    if (static_cast<IntegerType*>(mType)->isSigned()) {
                        mod = builder.createSMod(left, right);
                    } else {
                        mod = builder.createUMod(left, right);
                    }
                }
                return createAssign(left, mod, false);
            }
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
                        std::format("no match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                            fmt::bold, mOperatorToken.getName(), fmt::reset,
                            fmt::bold, mLeft->getType()->getName(), fmt::reset,
                            fmt::bold, mRight->getType()->getName(), fmt::reset)
                    );
                    exit = true;
                }
                mType = mLeft->getType();
                break;

            case Assign:
                if (mLeft->getType() != mRight->getType()) {
                    if (mRight->canImplicitCast(diag, mLeft->getType())) {
                        mRight = CastTo(mRight, mLeft->getType());
                    } else {
                        diag.reportCompilerError(mSource,
                            std::format("no match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                                fmt::bold, mOperatorToken.getName(), fmt::reset,
                                fmt::bold, mLeft->getType()->getName(), fmt::reset,
                                fmt::bold, mRight->getType()->getName(), fmt::reset)
                        );
                        exit = true;
                    }
                }
                break;

            case AddAssign:
            case SubAssign:
            case MulAssign:
            case DivAssign:
            case ModAssign:
                if (mLeft->getType()->isIntegerType()) {
                    if (mLeft->getType() != mRight->getType()) {
                        if (mRight->canImplicitCast(diag, mLeft->getType())) {
                            mRight = CastTo(mRight, mLeft->getType());
                        } else {
                            diag.reportCompilerError(mSource,
                                std::format("no match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                                    fmt::bold, mOperatorToken.getName(), fmt::reset,
                                    fmt::bold, mLeft->getType()->getName(), fmt::reset,
                                    fmt::bold, mRight->getType()->getName(), fmt::reset)
                            );
                            exit = true;
                        }
                        mType = mLeft->getType();
                    }
                } else {
                    diag.reportCompilerError(mSource,
                                std::format("no match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                                    fmt::bold, mOperatorToken.getName(), fmt::reset,
                                    fmt::bold, mLeft->getType()->getName(), fmt::reset,
                                    fmt::bold, mRight->getType()->getName(), fmt::reset)
                            );
                    exit = true;
                    mType = Type::Get("error-type");
                }
                break;
        }
    }
}
