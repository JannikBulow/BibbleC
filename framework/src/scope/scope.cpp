// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/scope/scope.h"

namespace bibblec::scope {
    Scope::Scope(Scope* parent)
        : mParent(parent) {}

    Scope* Scope::getParent() const {
        return mParent;
    }

    Type* Scope::getCurrentReturnType() const {
        for (const Scope& scope : *this) {
            if (scope.mCurrentReturnType) return scope.mCurrentReturnType;
        }
        return nullptr;
    }

    void Scope::setCurrentReturnType(Type* type) {
        mCurrentReturnType = type;
    }
}
