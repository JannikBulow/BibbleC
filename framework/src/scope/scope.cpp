// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/scope/scope.h"

namespace bibblec::scope {
    Scope::Scope(Scope* parent)
        : mParent(parent) {
        if (parent) parent->mChildren.push_back(this);
    }

    Scope* Scope::getParent() const {
        return mParent;
    }

    const std::vector<Scope*>& Scope::getChildren() const {
        return mChildren;
    }

    Symbol* Scope::resolveSymbol(std::string_view name) const {
        for (const Scope& scope : *this) {
            auto it = std::ranges::find_if(scope.mSymbols, [name](const SymbolPtr& symbol) {
                return symbol->name == name;
            });

            if (it != scope.mSymbols.end()) {
                return it->get();
            }
        }
        return nullptr;
    }

    std::vector<Symbol*> Scope::getCandidateFunctions(std::string_view name) const {
        std::vector<Symbol*> candidates;
        for (const Scope& scope : *this) {
            for (const SymbolPtr& symbol : scope.mSymbols) {
                if (symbol->name == name) {
                    candidates.push_back(symbol.get());
                }
            }
        }
        return candidates;
    }

    void Scope::addSymbol(SymbolPtr symbol) {
        mSymbols.push_back(std::move(symbol));
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
