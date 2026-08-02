// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/new_expression.h"

#include "BibbleC/type/class_type.h"
#include "BibbleC/type/function_type.h"

#include <BibblIR/ir/instruction/new_instruction.h>

#include <BibblIR/ir/builder.h>

#include <algorithm>

namespace bibblec::parser {
    NewExpression::NewExpression(scope::Scope* scope, Type* allocatedType, std::vector<ASTNodePtr> parameters, SourcePair source)
        : ASTNode(scope, source, allocatedType)
        , mParameters(std::move(parameters))
        , mBestViableConstructor(nullptr) {}

    std::vector<ASTNode*> NewExpression::getChildren() {
        std::vector<ASTNode*> children;
        children.reserve(mParameters.size());
        for (auto& parameter : mParameters) {
            children.push_back(parameter.get());
        }
        return children;
    }

    bibblir::Value* NewExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Value* constructor = mBestViableConstructor->getLatestValue()->value;

        if (mType->isClassType()) {
            auto classType = static_cast<ClassType*>(mType);

            bibblir::Value* object = builder.createNew(classType->getBibblirClass());

            std::vector<bibblir::Value*> parameters;
            parameters.reserve(mParameters.size() + 1);
            parameters.push_back(object);
            for (auto& parameter : mParameters) {
                parameters.push_back(parameter->codegen(builder, module, diag));
            }

            builder.createCall(constructor, std::move(parameters));

            return object;
        }

        return nullptr;
    }

    void NewExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& parameter : mParameters) {
            parameter->typeCheck(diag, exit);
        }

        if (!mType->isClassType()) {
            diag.reportCompilerError(mSource,
                std::format("'{}operator new{}' can currently only be used on {}class{} types",
                    fmt::bold, fmt::reset, fmt::bold, fmt::reset)
            );
            exit = true;
            return;
        }

        mBestViableConstructor = getBestViableConstructor(diag);
        if (!mBestViableConstructor) {
            exit = true;
            return;
        }

        auto functionType = static_cast<FunctionType*>(mBestViableConstructor->type);

        unsigned int index = 1;
        for (auto& parameter : mParameters) {
            Type* argumentType = functionType->getArgumentTypes()[index++];

            if (parameter->getType() != argumentType) {
                if (parameter->canImplicitCast(diag, argumentType)) {
                    parameter = CastTo(parameter, argumentType);
                } else {
                    diag.reportCompilerError(mSource,
                        std::format("no matching constructor for class '{}{}{}'",
                            fmt::bold, mType->getName(), fmt::reset)
                    );
                    exit = true;
                }
            }
        }
    }

    struct ViableFunction {
        scope::Symbol* symbol;
        int score;
        bool disallowed;
    };

    scope::Symbol* NewExpression::getBestViableConstructor(diagnostic::Diagnostics& diag) {
        std::vector<scope::Symbol*> candidates = mScope->getCandidateFunctions(".init");
        std::string errorName = std::format("{}::{}", mType->getName(), static_cast<ClassType*>(mType)->getClassName());

        for (auto it = candidates.begin(); it != candidates.end();) {
            scope::Symbol* candidate = *it;
            if (!candidate->type->isFunctionType()) {
                it = candidates.erase(it);
            } else {
                auto functionType = static_cast<FunctionType*>(candidate->type);
                auto& arguments = functionType->getArgumentTypes();

                if (!arguments.empty() && arguments[0] != mType) {
                    it = candidates.erase(it);
                    continue;
                }

                if (arguments.size() != (mParameters.size() + 1)) {
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
                Type::CastLevel castLevel = mParameters[i]->getType()->castTo(functionType->getArgumentTypes()[i + 1]);
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
                std::format("no matching constructor for class '{}{}{}'",
                    fmt::bold, mType->getName(), fmt::reset)
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
}
