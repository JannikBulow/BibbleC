// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/variable_declaration.h"

namespace bibblec::parser {
    VariableDeclaration::VariableDeclaration(scope::Scope* scope, std::string name, Type* type, ASTNodePtr initialValue, SourcePair source)
        : ASTNode(scope, source, type)
        , mName(std::move(name))
        , mInitialValue(std::move(initialValue)) {
        mScope->addSymbol(std::make_unique<scope::Symbol>(mName, mType));
        mSymbol = mScope->getLatestSymbol();
    }

    std::vector<ASTNode*> VariableDeclaration::getChildren() {
        if (mInitialValue) return {mInitialValue.get()};
        return {};
    }

    bibblir::Value* VariableDeclaration::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        if (mInitialValue) {
            bibblir::Value* value = mInitialValue->codegen(builder, module, diag);
            mSymbol->values.push_back({builder.getInsertPoint(), value});
        }

        return nullptr;
    }

    void VariableDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        if (mInitialValue) mInitialValue->typeCheck(diag, exit);

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

            mType = mInitialValue->getType();
            mSymbol->type = mType;
        }

        if (mInitialValue) {
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
