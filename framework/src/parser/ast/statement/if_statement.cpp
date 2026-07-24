// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/if_statement.h"
#include "BibbleC/parser/ast/statement/variable_declaration.h"

#include <BibblIR/ir/instruction/phi_instruction.h>

#include <BibblIR/ir/function.h>

namespace bibblec::parser {
    IfStatement::IfStatement(ASTNodePtr condition, ASTNodePtr body, ASTNodePtr elseBody, scope::Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mCondition(std::move(condition))
        , mBody(std::move(body))
        , mElseBody(std::move(elseBody)) {}

    std::vector<ASTNode*> IfStatement::getChildren() {
        if (mElseBody) return {mCondition.get(), mBody.get(), mElseBody.get()};
        else return {mCondition.get(), mBody.get()};
    }

    bibblir::Value* IfStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::BasicBlock* startBB = builder.getInsertPoint();
        bibblir::BasicBlock* trueBB = builder.getInsertPoint()->getParent()->createBasicBlock("");
        bibblir::BasicBlock* falseBB = mElseBody ? builder.getInsertPoint()->getParent()->createBasicBlock("") : nullptr;
        bibblir::BasicBlock* mergeBB = builder.getInsertPoint()->getParent()->createBasicBlock("");

        bibblir::Value* condition;
        if (mElseBody) {
            falseBB->loopEnd() = mergeBB;
            condition = mCondition->ccodegen(builder, module, diag, trueBB, falseBB);
        } else {
            condition = mCondition->ccodegen(builder, module, diag, trueBB, mergeBB);
        }

        builder.setInsertPoint(trueBB);
        mBody->codegen(builder, module, diag);
        if (!builder.getInsertPoint()->hasTerminator()) {
            builder.createBr(mergeBB);
        }

        if (mElseBody) {
            builder.setInsertPoint(falseBB);
            mElseBody->codegen(builder, module, diag);
            if (!builder.getInsertPoint()->hasTerminator()) {
                builder.createBr(mergeBB);
            }
        }

        builder.setInsertPoint(mergeBB);

        std::vector<scope::Symbol*> symbols;
        for (scope::Scope& scope : *mScope) {
            for (const auto& symbol : scope.getSymbols()) {
                symbols.push_back(symbol.get());
            }
        }

        for (scope::Symbol* symbol : symbols) {
            auto startBBValue = symbol->getLatestValue(startBB);
            if (!startBBValue) continue;

            std::vector<std::pair<scope::SymbolValue*, bibblir::BasicBlock*>> values;
            for (auto pred : mergeBB->predecessors()) {
                auto value = symbol->getLatestValueX(pred);
                if (value) values.emplace_back(value, pred);
            }

            if (values.size() > 1) {
                auto phi = builder.createPhi(symbol->type->getBibblirType());
                for (auto& value : values) {
                    phi->addIncoming(value.first->value, value.second);
                }

                symbol->values.emplace_back(mergeBB, phi);
            }
        }

        return nullptr;
    }

    void IfStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mCondition->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);
        if (mElseBody) {
            mElseBody->typeCheck(diag, exit);
        }

        if (!mCondition->getType()->isBooleanType()) {
            auto boolType = Type::Get("bool");

            if (mCondition->canImplicitCast(diag, boolType)) {
                mCondition = CastTo(mCondition, boolType);
            } else {
                diag.reportCompilerError(mSource,
                    std::format("value of type '{}{}{}' can't be used as condition in if statement",
                        fmt::bold, mCondition->getType()->getName(), fmt::reset)
                );
                exit = true;
            }
        }
    }
}
