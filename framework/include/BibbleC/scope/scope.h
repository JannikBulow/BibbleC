// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_SCOPE_SCOPE_H
#define BIBBLEC_SCOPE_SCOPE_H

#include "BibbleC/scope/symbol.h"

#include "BibbleC/type/type.h"

#include "BibbleC/api.h"

namespace bibblec::scope {
    class BIBBLEC_EXPORT Scope {
    public:
        template<class T>
        struct BaseIterator {
            T* current;

            BaseIterator(T* current) : current(current) {}

            T& operator*() const { return *current; }
            bool operator!=(const BaseIterator& other) const { return other.current != current; }
            BaseIterator& operator++() {
                current = current->mParent;
                return *this;
            }
        };

        using Iterator = BaseIterator<Scope>;
        using ConstIterator = BaseIterator<const Scope>;

        explicit Scope(std::optional<std::string> moduleName = std::nullopt, Scope* parent = nullptr);

        Iterator begin() { return this; }
        Iterator end() { return nullptr; }
        ConstIterator begin() const { return this; }
        ConstIterator end() const { return nullptr; }

        Scope* getParent() const;
        const std::vector<Scope*>& getChildren() const;
        const std::vector<SymbolPtr>& getSymbols() const;

        std::string_view getModuleName() const;

        Symbol* getLatestSymbol() const;
        Symbol* resolveSymbol(std::string_view name) const;
        std::vector<Symbol*> getCandidateFunctions(std::string_view name) const;
        void addSymbol(SymbolPtr symbol);

        Type* getCurrentReturnType() const;
        void setCurrentReturnType(Type* type);

    private:
        Scope* mParent;
        std::optional<std::string> mModuleName;
        std::vector<Scope*> mChildren;

        std::vector<SymbolPtr> mSymbols;

        Type* mCurrentReturnType = nullptr;
    };

    using ScopePtr = std::unique_ptr<Scope>;
}

#endif //BIBBLEC_SCOPE_SCOPE_H
