// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/scope/symbol.h"

namespace bibblec::scope {
    Symbol::Symbol(std::string name, Type* type)
        : name(std::move(name))
        , type(type) {}

    SymbolValue* Symbol::getLatestValue(bibblir::BasicBlock* bb) {
        SymbolValue* ret = getLatestValueImpl(bb);
        mSearched.clear();
        return ret;
    }

    SymbolValue* Symbol::getLatestValueX(bibblir::BasicBlock* bb) {
        auto it = std::find_if(values.rbegin(), values.rend(), [bb](const SymbolValue& value) {
            return value.bb == bb;
        });
        if (it != values.rend()) {
            return &*it;
        }

        if (bb->predecessors().size() == 1) {
            if (SymbolValue* value = getLatestValueX(bb->predecessors()[0])) {
                return value;
            }
        }

        return nullptr;
    }

    SymbolValue* Symbol::getLatestValueImpl(bibblir::BasicBlock* bb) {
        if (!bb) {
            return &values.back();
        }

        if (std::ranges::find(mSearched, bb) != mSearched.end()) return nullptr;
        mSearched.push_back(bb);

        auto it = std::find_if(values.rbegin(), values.rend(), [bb](const SymbolValue& value) {
            return value.bb == bb;
        });
        if (it != values.rend()) {
            mSearched.clear();
            return &*it;
        }

        for (auto predecessor : bb->predecessors()) {
            if (auto value = getLatestValue(predecessor)) {
                mSearched.clear();
                return value;
            }
        }

        return nullptr;
    }
}
