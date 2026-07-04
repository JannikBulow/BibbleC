// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_SCOPE_SYMBOL_H
#define BIBBLEC_SCOPE_SYMBOL_H

#include "BibbleC/type/type.h"

#include "BibbleC/api.h"

#include <BibblIR/ir/basicblock.h>
#include <BibblIR/ir/value.h>

#include <memory>
#include <string>
#include <vector>

namespace bibblec::scope {
    struct SymbolValue {
        bibblir::BasicBlock* bb;
        bibblir::Value* value;
    };

    class BIBBLEC_EXPORT Symbol {
    public:
        std::string name;
        Type* type;
        std::vector<SymbolValue> values;

        bool removed = false;
        bool constant = false;

        Symbol(std::string name, Type* type);

        SymbolValue* getLatestValue(bibblir::BasicBlock* bb = nullptr);
        SymbolValue* getLatestValueX(bibblir::BasicBlock* bb);

    private:
        std::vector<bibblir::BasicBlock*> mSearched;

        SymbolValue* getLatestValueImpl(bibblir::BasicBlock* bb = nullptr);
    };

    using SymbolPtr = std::unique_ptr<Symbol>;
}

#endif //BIBBLEC_SCOPE_SYMBOL_H
