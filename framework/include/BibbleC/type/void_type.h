// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_VOID_TYPE_H
#define BIBBLEC_TYPE_VOID_TYPE_H

#include "BibbleC/type/type.h"

namespace bibblec {
    class VoidType : public Type {
    public:
        VoidType();

        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isVoidType() const override;
    };
}

#endif //BIBBLEC_TYPE_VOID_TYPE_H
