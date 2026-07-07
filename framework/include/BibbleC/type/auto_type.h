// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_AUTO_TYPE_H
#define BIBBLEC_TYPE_AUTO_TYPE_H

#include "BibbleC/type/type.h"

namespace bibblec {
    class BIBBLEC_EXPORT AutoType : public Type {
    public:
        AutoType();

        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isAutoType() const override;
    };
}

#endif //BIBBLEC_TYPE_AUTO_TYPE_H
