// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/statement/return_statement.h"

#include <BibblIR/ir/instruction/return_instruction.h>

namespace bibblec::parser {
    ReturnStatement::ReturnStatement(scope::Scope* scope, ASTNodePtr returnValue, SourcePair source)
        : ASTNode(scope, source)
        , mReturnValue(std::move(returnValue)) {}

    std::vector<ASTNode*> ReturnStatement::getChildren() {
        return {mReturnValue.get()};
    }

    bibblir::Value* ReturnStatement::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        if (!mReturnValue) {
            return builder.createReturn(nullptr);
        }
        return builder.createReturn(mReturnValue->codegen(builder, module, diag));
    }

    void ReturnStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        if (mReturnValue) {
            mReturnValue->typeCheck(diag, exit);
        }

        Type* returnType = mScope->getCurrentReturnType();
        if (returnType->isVoidType()) {
            if (mReturnValue && !mReturnValue->getType()->isVoidType()) {
                diag.reportCompilerError(mReturnValue->getSource(),
                    std::format("function returning '{}void{}' should not return a value",
                        fmt::bold, fmt::reset)
                );
                exit = true;
            }
        } else {
            if (!mReturnValue) {
                diag.reportCompilerError(mSource,
                    std::format("non-void function returning '{}{}{}' cannot return '{}void{}'",
                        fmt::bold, returnType->getName(), fmt::reset,
                        fmt::bold, fmt::reset)
                );
                exit = true;
            } else if (returnType != mReturnValue->getType()) {
                if (mReturnValue->canImplicitCast(diag, returnType)) {
                    mReturnValue = CastTo(mReturnValue, returnType);
                } else {
                    diag.reportCompilerError(mSource,
                        std::format("value of type '{}{}{}' is not compatible with return type '{}{}{}'",
                            fmt::bold, mReturnValue->getType()->getName(), fmt::reset,
                            fmt::bold, returnType->getName(), fmt::reset)
                    );
                    exit = true;
                }
            }
        }
    }
}
