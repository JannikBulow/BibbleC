// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/cast_expression.h"

#include "BibbleC/type/integer_type.h"

namespace bibblec::parser {
    CastExpression::CastExpression(scope::Scope* scope, ASTNodePtr value, Type* destType, SourcePair source)
        : ASTNode(scope, source, destType)
        , mValue(std::move(value)) {}

    std::vector<ASTNode*> CastExpression::getChildren() {
        return {mValue.get()};
    }

    bibblir::Value* CastExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Value* value = mValue->codegen(builder, module, diag);
        return value; //TODO: this function must create a bibblir value of the destination type, but realistically, values at runtime don't need any special casting instructions as they're all 64-bit integers
    }

    void CastExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mValue->typeCheck(diag, exit);

        if (mValue->getType()->castTo(mType) == Type::CastLevel::Disallowed) {
            diag.reportCompilerError(mSource,
                std::format("can't cast from '{}{}{}' to '{}{}{}'",
                    fmt::bold, mValue->getType()->getName(), fmt::reset,
                    fmt::bold, mType->getName(), fmt::red)
            );
            exit = true;
        }
    }

    bool CastExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return mValue->triviallyImplicitCast(diag, destType);
    }
}
