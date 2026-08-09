// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/for_statement.h"

#include <BibblIR/ir/instruction/phi_instruction.h>

#include <BibblIR/ir/function.h>

namespace bibblec::parser {
    ForStatement::ForStatement(ASTNodePtr init, ASTNodePtr condition, ASTNodePtr it, ASTNodePtr body, scope::Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mInit(std::move(init))
        , mCondition(std::move(condition))
        , mIt(std::move(it))
        , mBody(std::move(body)){}

    std::vector<ASTNode*> ForStatement::getChildren() {
        return {mInit.get(), mCondition.get(), mIt.get(), mBody.get()};
    }

    bibblir::Value* ForStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::BasicBlock* startBB = builder.getInsertPoint();
        bibblir::BasicBlock* bodyBB = builder.getInsertPoint()->getParent()->createBasicBlock("");
        bibblir::BasicBlock* itBB = builder.getInsertPoint()->getParent()->createBasicBlock("");
        bibblir::BasicBlock* mergeBB = builder.getInsertPoint()->getParent()->createBasicBlock("");

        mScope->continueBB() = itBB;
        mScope->breakBB() = mergeBB;

        std::vector<scope::Symbol*> symbols;
        std::vector<bibblir::PhiInstruction*> phis;
        for (const auto& scope : *mScope) {
            for (auto& symbol : scope.getSymbols()) {
                symbols.push_back(symbol.get());
            }
        }

        mInit->codegen(builder, module, diag);
        mCondition->ccodegen(builder, module, diag, bodyBB, mergeBB);

        bodyBB->loopEnd() = mergeBB;
        itBB->loopEnd() = mergeBB;

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
        if (!builder.getInsertPoint()->hasTerminator()) builder.createBr(itBB);

        builder.getInsertPoint()->loopEnd() = mergeBB;

        if (builder.getInsertPoint()->successors().back() == itBB) {
            builder.setInsertPoint(itBB);
            mIt->codegen(builder, module, diag);
            mCondition->ccodegen(builder, module, diag, bodyBB, mergeBB);
        }

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

    void ForStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mInit->typeCheck(diag, exit);
        mCondition->typeCheck(diag, exit);
        mIt->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);

        if (!mCondition->getType()->isBooleanType()) {
            Type* boolType = Type::Get("bool");

            if (mCondition->canImplicitCast(diag, boolType)) {
                mCondition = CastTo(mCondition, boolType);
            } else {
                diag.reportCompilerError(mSource,
                    std::format("value of type '{}{}{}' can't be used as condition in for statement",
                        fmt::bold, mCondition->getType()->getName(), fmt::reset)
                );
                exit = true;
            }
        }
    }
}
