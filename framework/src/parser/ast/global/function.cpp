// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/global/function.h"

#include "BibbleC/parser/ast/statement/return_statement.h"

#include <BibblIR/ir/function.h>

#include <functional>

namespace bibblec::parser {
    FunctionArgument::FunctionArgument(Type* type, std::string name)
        : type(type)
        , name(std::move(name))
        , symbol(nullptr) {}

    Function::Function(
        std::vector<lexer::Token> modifierTokens,
        std::string name,
        FunctionType* type,
        std::vector<FunctionArgument> arguments,
        scope::ScopePtr ownScope,
        std::vector<ASTNodePtr> body,
        SourcePair source,
        SourcePair blockEnd)
        : ASTNode(ownScope->getParent(), source, type)
        , mName(std::move(name))
        , mArguments(std::move(arguments))
        , mBody(std::move(body))
        , mBlockEnd(blockEnd)
        , mOwnScope(std::move(ownScope)) {
        //TODO: modifiers
        constructorImpl(type);
    }

    Function::Function(
        FunctionModifiers modifiers,
        std::string name,
        FunctionType* type,
        std::vector<FunctionArgument> arguments,
        scope::ScopePtr ownScope,
        std::vector<ASTNodePtr> body,
        SourcePair source,
        SourcePair blockEnd)
        : ASTNode(ownScope->getParent(), source, type)
        , mModifiers(modifiers)
        , mName(std::move(name))
        , mArguments(std::move(arguments))
        , mBody(std::move(body))
        , mBlockEnd(blockEnd)
        , mOwnScope(std::move(ownScope)) {
        constructorImpl(type);
    }

    std::vector<ASTNode*> Function::getChildren() {
        std::vector<ASTNode*> children;
        for (ASTNodePtr& node : mBody) {
            children.push_back(node.get());
        }
        return children;
    }

    ASTNodePtr Function::cloneExternal(scope::Scope* in) {
        scope::ScopePtr ownScope = std::make_unique<scope::Scope>(in);
        auto functionType = static_cast<FunctionType*>(mType);
        return std::make_unique<Function>(mModifiers, mName, functionType, mArguments, std::move(ownScope), std::vector<ASTNodePtr>(), mSource, mBlockEnd);
    }

    bibblir::Value* Function::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        auto function = static_cast<bibblir::Function*>(mSymbol->values.front().value);

        bibblir::BasicBlock* entryBB = function->createBasicBlock("");
        builder.setInsertPoint(entryBB);

        int index = 0;
        for (FunctionArgument& argument : mArguments) {
            bibblir::Argument* arg = function->getArgument(index++);
            argument.symbol->values.emplace_back(entryBB, arg);
        }

        for (ASTNodePtr& node : mBody) {
            node->codegen(builder, module, diag);
        }

        if (!builder.getInsertPoint()->hasTerminator()) {
            Type* returnType = static_cast<FunctionType*>(mType)->getReturnType();
            if (returnType->isVoidType()) {
                builder.createReturn(nullptr);
            } else {
                diag.reportCompilerError(mSource, "not all control paths return a value");
                std::exit(1);
            }
        }

        return function;
    }

    void Function::setEmittedValue(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        std::string mangledName = mangleName();

        bibblir::Function* function = bibblir::Function::Create(module, static_cast<bibblir::FunctionType*>(mType->getBibblirType()), std::move(mangledName));

        mSymbol->values.push_back({nullptr, function});
    }

    void Function::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        FunctionType* functionType = static_cast<FunctionType*>(mType);

        if (functionType->getReturnType()->isErrorType()) {
            std::function<ReturnStatement*(ASTNode*)> process = [&process](ASTNode* node) -> ReturnStatement* {
                if (auto ret = dynamic_cast<ReturnStatement*>(node)) {
                    return ret;
                }

                for (ASTNode* child : node->getChildren()) {
                    if (auto ret = process(child)) {
                        return ret;
                    }
                }

                return nullptr;
            };

            std::vector<Type*> parameterTypes;
            Type* returnType = nullptr;

            for (FunctionArgument& argument : mArguments) {
                parameterTypes.push_back(argument.type);
            }

            for (ASTNodePtr& node : mBody) {
                if (auto ret = process(node.get())) {
                    if (ret->getChildren().empty()) {
                        returnType = Type::Get("void");
                    } else {
                        returnType = ret->getChildren()[0]->getType();
                    }
                    break;
                }
            }

            if (!returnType) {
                diag.reportCompilerError(mSource, "could not deduce return type");
                exit = true;
            } else {
                functionType = FunctionType::Create(returnType, parameterTypes);
                mType = functionType;
                mSymbol->type = functionType;
                mOwnScope->setCurrentReturnType(returnType);
            }
        }

        for (ASTNodePtr& node : mBody) {
            node->typeCheck(diag, exit);
        }
    }

    void Function::constructorImpl(FunctionType* type) {
        mOwnScope->setCurrentReturnType(type->getReturnType());
        mScope->addSymbol(std::make_unique<scope::Symbol>(mName, type));
        mSymbol = mScope->getLatestSymbol();

        for (FunctionArgument& argument : mArguments) {
            mOwnScope->addSymbol(std::make_unique<scope::Symbol>(argument.name, argument.type));
            argument.symbol = mOwnScope->getLatestSymbol();
        }
    }

    std::string Function::mangleName() {
        if (mName == "main") return ".main";
        if (mName.starts_with('.')) return mName;

        FunctionType* functionType = static_cast<FunctionType*>(mType);

        std::string mangled = mName;

        mangled += '(';
        for (Type* argument : functionType->getArgumentTypes()) {
            mangled += argument->getSymbolID(nullptr);
        }
        mangled += ')';

        mangled += functionType->getReturnType()->getSymbolID(nullptr);

        return mangled;
    }
}
