// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/string_literal.h"

#include <BibblIR/ir/constant/constant_string.h>

namespace bibblec::parser {
    StringLiteral::StringLiteral(scope::Scope* scope, std::string value, SourcePair source)
        : ASTNode(scope, std::move(source), Type::Get("string"))
        , mValue(std::move(value)) {}

    std::string_view StringLiteral::getValue() const {
        return mValue;
    }

    ASTNodePtr StringLiteral::cloneExternal(scope::Scope* in) {
        return std::make_unique<StringLiteral>(in, mValue, mSource);
    }

    bibblir::Value* StringLiteral::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        return builder.createConstantString(mValue);
    }

    void StringLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {

    }
}
