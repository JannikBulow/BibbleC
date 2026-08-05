// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/while_statement.h"

#include <BibblIR/ir/instruction/phi_instruction.h>

#include <BibblIR/ir/function.h>

namespace bibblec::parser {
    WhileStatement::WhileStatement(ASTNodePtr condition, ASTNodePtr body, scope::Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mCondition(std::move(condition))
        , mBody(std::move(body)) {}

    std::vector<ASTNode*> WhileStatement::getChildren() {
        return {mCondition.get(), mBody.get()};
    }

    bibblir::Value* WhileStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::BasicBlock* startBB = builder.getInsertPoint();
        bibblir::BasicBlock* bodyBB = builder.getInsertPoint()->getParent()->createBasicBlock("");
        bibblir::BasicBlock* mergeBB = builder.getInsertPoint()->getParent()->createBasicBlock("");

        std::vector<scope::Symbol*> symbols;
        std::vector<bibblir::PhiInstruction*> phis;
        for (const auto& scope : *mScope) {
            for (auto& symbol : scope.getSymbols()) {
                symbols.push_back(symbol.get());
            }
        }

        mCondition->ccodegen(builder, module, diag, bodyBB, mergeBB);

        bodyBB->loopEnd() = mergeBB;

        builder.setInsertPoint(bodyBB);
        for (auto* symbol : symbols) {
            auto* startBBValue = symbol->getLatestValue(startBB);
            if (!startBBValue) {
                phis.push_back(nullptr);
                continue;
            }

            bibblir::PhiInstruction* phi = builder.createPhi(symbol->type->getBibblirType());
            phi->addIncoming(startBBValue->value, startBB);
            phis.push_back(phi);

            symbol->values.emplace_back(bodyBB, phi);
        }
        mBody->codegen(builder, module, diag);
        if (!bodyBB->hasTerminator()) mCondition->ccodegen(builder, module, diag, bodyBB, mergeBB);

        for (size_t i = 0; i < phis.size(); i++) {
            if (!phis[i]) continue;

            int incoming = 1;
            auto* startBBValue = symbols[i]->getLatestValue(startBB);
            for (auto* bb : bodyBB->predecessors()) {
                auto* value = symbols[i]->getLatestValueX(bb);
                if (value && value != startBBValue && value->value != phis[i]) {
                    phis[i]->addIncoming(value->value, bb);
                    incoming++;
                }
            }

            if (incoming == 1) {
                std::erase_if(symbols[i]->values, [phi = phis[i]](auto value) {
                   return value.value == phi;
                });
                builder.getInsertPoint()->getParent()->replaceAllUsesWith(phis[i], startBBValue->value);
                phis[i]->eraseFromParent();
            }
        }

        builder.setInsertPoint(mergeBB);
        for (auto* symbol : symbols) {
            auto startBBValue = symbol->getLatestValue(startBB);
            if (!startBBValue) continue;

            std::vector<std::pair<scope::SymbolValue*, bibblir::BasicBlock*>> values;
            for (auto pred : mergeBB->predecessors()) {
                auto value = symbol->getLatestValueX(pred);
                if (value) values.emplace_back(value, pred);
            }

            if (values.size() > 1) {
                bibblir::PhiInstruction* phi = builder.createPhi(symbol->type->getBibblirType());
                for (auto& value : values) {
                    phi->addIncoming(value.first->value, value.second);
                }

                symbol->values.emplace_back(mergeBB, phi);
            }
        }

        return nullptr;
    }

    void WhileStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mCondition->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);

        if (!mCondition->getType()->isBooleanType()) {
            Type* boolType = Type::Get("bool");

            if (mCondition->canImplicitCast(diag, boolType)) {
                mCondition = CastTo(mCondition, boolType);
            } else {
                diag.reportCompilerError(mSource,
                    std::format("value of type '{}{}{}' can't be used as condition in while statement",
                        fmt::bold, mCondition->getType()->getName(), fmt::reset)
                );
                exit = true;
            }
        }
    }
}
