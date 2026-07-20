// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/boolean_literal.h"

#include <BibblIR/ir/constant/constant_boolean.h>

namespace bibblec::parser {
    BooleanLiteral::BooleanLiteral(scope::Scope* scope, bool value, SourcePair source)
        : ASTNode(scope, source, Type::Get("bool"))
        , mValue(value) {}

    bool BooleanLiteral::getValue() const {
        return mValue;
    }

    bibblir::Value* BooleanLiteral::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        return builder.createConstantBoolean(mValue);
    }

    void BooleanLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
    }

    bool BooleanLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        if (destType->isIntegerType()) {
            mType = destType;
            return true;
        }
        return false;
    }
}
