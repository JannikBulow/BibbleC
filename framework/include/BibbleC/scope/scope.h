// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_SCOPE_SCOPE_H
#define BIBBLEC_SCOPE_SCOPE_H

#include "BibbleC/type/type.h"

#include "BibbleC/api.h"

namespace bibblec::scope {
    class BIBBLEC_EXPORT Scope {
    public:
        explicit Scope(Scope* parent = nullptr);

        Scope* getParent() const;

        Type* getCurrentReturnType() const;
        void setCurrentReturnType(Type* type);

    private:
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

        Scope* mParent;

        Type* mCurrentReturnType = nullptr;

        Iterator begin() { return this; }
        Iterator end() { return nullptr; }
        ConstIterator begin() const { return this; }
        ConstIterator end() const { return nullptr; }
    };
}

#endif //BIBBLEC_SCOPE_SCOPE_H
