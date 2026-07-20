// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/integer_literal.h"

#include <BibblIR/ir/constant/constant_int.h>

#include <cmath>

namespace bibblec::parser {
    IntegerLiteral::IntegerLiteral(scope::Scope* scope, uintmax_t value, Type* type, SourcePair source)
        : ASTNode(scope, std::move(source), type)
        , mValue(value) {}

    uintmax_t IntegerLiteral::getValue() const {
        return mValue;
    }

    ASTNodePtr IntegerLiteral::cloneExternal(scope::Scope* in) {
        return std::make_unique<IntegerLiteral>(in, mValue, mType, mSource);
    }

    bibblir::Value* IntegerLiteral::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        return builder.createConstantInt(static_cast<intmax_t>(mValue), mType->getBibblirType());
    }

    void IntegerLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
    }

    bool IntegerLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        if (destType->isIntegerType()) {
            unsigned srcBits = mType->getSize() * 8;
            unsigned destBits = destType->getSize() * 8;

            if (destBits < srcBits) {
                diag.reportCompilerWarning(mSource, "implicit",
                    std::format("integer literal '{}{}{}' is being narrowed to '{}{}{}'",
                        fmt::bold, mValue, fmt::reset,
                        fmt::bold, mValue & ((static_cast<uintmax_t>(1) << destBits) - 1), fmt::reset)
                );

                mValue = mValue & ((static_cast<uintmax_t>(1) << destBits) - 1);
            }

            mType = destType;
            return true;
        }
        return false;
    }
}
