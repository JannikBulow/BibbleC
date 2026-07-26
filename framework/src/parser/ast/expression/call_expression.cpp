// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/call_expression.h"
#include "BibbleC/parser/ast/expression/variable_expression.h"

#include "BibbleC/type/function_type.h"

#include <BibblIR/ir/instruction/call_instruction.h>

#include <algorithm>

namespace bibblec::parser {
    CallExpression::CallExpression(scope::Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters, SourcePair source)
        : ASTNode(scope, source)
        , mCallee(std::move(callee))
        , mParameters(std::move(parameters))
        , mBestViableFunction(nullptr) {}

    std::vector<ASTNode*> CallExpression::getChildren() {
        std::vector<ASTNode*> children;
        children.push_back(mCallee.get());
        children.reserve(mParameters.size());
        for (auto& parameter : mParameters) {
            children.push_back(parameter.get());
        }
        return children;
    }

    bibblir::Value* CallExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Value* callee = mBestViableFunction->getLatestValue()->value;

        std::vector<bibblir::Value*> parameters;
        parameters.reserve(mParameters.size());
        for (auto& parameter : mParameters) {
            parameters.push_back(parameter->codegen(builder, module, diag));
        }

        return builder.createCall(callee, std::move(parameters));
    }

    void CallExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mCallee->typeCheck(diag, exit);
        for (auto& parameter : mParameters) {
            parameter->typeCheck(diag, exit);
        }

        mBestViableFunction = getBestViableFunction(diag);
        if (!mBestViableFunction) {
            exit = true;
            mType = Type::Get("error-type");
            return;
        }

        auto functionType = static_cast<FunctionType*>(mBestViableFunction->type);
        mType = functionType->getReturnType();

        unsigned int index = 0;
        for (auto& parameter : mParameters) {
            auto argumentType = functionType->getArgumentTypes()[index++];
            if (parameter->getType() != argumentType) {
                if (parameter->canImplicitCast(diag, argumentType)) {
                    parameter = CastTo(parameter, argumentType);
                } else {
                    diag.reportCompilerError(mSource,
                        std::format("no matching function for call to '{}{}(){}'",
                            fmt::bold, mBestViableFunction->name, fmt::reset)
                    );
                }
            }
        }
    }

    struct ViableFunction {
        scope::Symbol* symbol;
        int score;
        bool disallowed;
    };

    scope::Symbol* CallExpression::getBestViableFunction(diagnostic::Diagnostics& diag) {
        if (auto var = dynamic_cast<VariableExpression*>(mCallee.get())) {
            std::vector<scope::Symbol*> candidates = mScope->getCandidateFunctions(var->getName());
            std::string errorName(var->getName());

            for (auto it = candidates.begin(); it != candidates.end();) {
                scope::Symbol* candidate = *it;
                if (!candidate->type->isFunctionType()) {
                    it = candidates.erase(it);
                } else {
                    auto functionType = static_cast<FunctionType*>(candidate->type);
                    if (functionType->getArgumentTypes().size() != mParameters.size()) {
                        it = candidates.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            std::vector<ViableFunction> viableFunctions;
            for (auto candidate : candidates) {
                auto functionType = static_cast<FunctionType*>(candidate->type);
                int score = 0;
                bool disallowed = false;
                size_t i = 0;

                for (; i < mParameters.size(); i++) {
                    Type::CastLevel castLevel = mParameters[i]->getType()->castTo(functionType->getArgumentTypes()[i]);
                    int multiplier = 0;

                    if (mParameters[i]->getType() == functionType->getArgumentTypes()[i]) multiplier = 0;
                    else if (castLevel == Type::CastLevel::Implicit) multiplier = 1;
                    else if (castLevel == Type::CastLevel::ImplicitWarning) multiplier = 2;
                    else disallowed = true;

                    score += multiplier * static_cast<int>(mParameters.size()) - static_cast<int>(i);
                }

                if (!disallowed) {
                    viableFunctions.emplace_back(candidate, score, false);
                }
            }

            if (viableFunctions.empty()) {
                diag.reportCompilerError(mSource,
                    std::format("no matching function for call to '{}{}(){}'",
                        fmt::bold, errorName, fmt::reset)
                );
                return nullptr;
            }

            std::ranges::sort(viableFunctions, [](const ViableFunction& lhs, const ViableFunction& rhs) {
                return lhs.score < rhs.score;
            });
            if (viableFunctions.size() >= 2) {
                if (viableFunctions[0].score == viableFunctions[1].score) {
                    diag.reportCompilerError(mSource,
                        std::format("call to '{}{}(){}' is ambiguous",
                            fmt::bold, errorName, fmt::reset)
                    );
                    return nullptr;
                }
            }

            return viableFunctions.front().symbol;
        }

        return nullptr;
    }
}
