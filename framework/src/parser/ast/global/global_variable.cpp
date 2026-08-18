// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/global/global_variable.h"

namespace bibblec::parser {
    GlobalVariable::GlobalVariable(scope::Scope* scope, std::string name, Type* type, ASTNodePtr initialValue, bool constant, SourcePair source)
        : ASTNode(scope, source, type)
        , mName(std::move(name))
        , mInitialValue(std::move(initialValue)) {
        mScope->addSymbol(std::make_unique<scope::Symbol>(mName, type));
        mSymbol = mScope->getLatestSymbol();
        mSymbol->constant = constant;
    }

    std::vector<ASTNode*> GlobalVariable::getChildren() {
        if (mInitialValue) return {mInitialValue.get()};
        return {};
    }

    ASTNodePtr GlobalVariable::cloneExternal(scope::Scope* in) {
        bool constant = mSymbol->constant;
        if (!constant) return nullptr;

        ASTNodePtr newInitialValue = mInitialValue ? mInitialValue->cloneExternal(in) : nullptr;
        return std::make_unique<GlobalVariable>(in, mName, mType, std::move(newInitialValue), constant, mSource);
    }

    bibblir::Value* GlobalVariable::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        if (!mSymbol->constant) {
            diag.reportCompilerError(mSource, "global variables can only be constant");
            std::exit(1);
        }

        if (!mInitialValue) {
            diag.reportCompilerError(mSource, "constant variable with no value");
            std::exit(1);
        }

        mSymbol->values.emplace_back(nullptr, mInitialValue->codegen(builder, module ,diag));

        return nullptr;
    }

    void GlobalVariable::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        bool typeCheckedInitialValue = false;

        if (mType->isAutoType()) {
            if (!mInitialValue) {
                diag.reportCompilerError(mSource,
                    std::format("variable '{}{}{}' has unknown type",
                        fmt::bold, mName, fmt::reset)
                );
                exit = true;
                mType = Type::Get("error-type");
                return;
            }

            mInitialValue->typeCheck(diag, exit);
            typeCheckedInitialValue = true;
            mType = mInitialValue->getType();
            mSymbol->type = mType;
        }

        if (mInitialValue) {
            if (!typeCheckedInitialValue) mInitialValue->typeCheck(diag, exit);

            if (mInitialValue->getType() != mType) {
                if (mInitialValue->canImplicitCast(diag, mType)) {
                    mInitialValue = CastTo(mInitialValue, mType);
                } else {
                    diag.reportCompilerError(mSource,
                        std::format("value of type '{}{}{}' cannot be assigned to variable of type '{}{}{}'",
                            fmt::bold, mInitialValue->getType()->getName(), fmt::reset,
                            fmt::bold, mType->getName(), fmt::reset)
                    );
                    exit = true;
                }
            }
        }
    }
}
