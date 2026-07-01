// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    bool ASTNode::canImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        if (triviallyImplicitCast(diag, destType)) {
            return true;
        }

        Type::CastLevel cast = mType->castTo(destType);
        if (cast == Type::CastLevel::Implicit || cast == Type::CastLevel::ImplicitWarning) {
            if (cast == Type::CastLevel::ImplicitWarning) {
                diag.reportCompilerWarning(mSource, "implicit", mType->getImplicitCastWarning(destType));
            }
            return true;
        }
        return false;
    }

    ASTNodePtr ASTNode::CastTo(ASTNodePtr& node, Type* destType) {
        if (node->mType == destType) {
            return std::move(node);
        }
        return nullptr; // TODO: cast expression
    }
}
