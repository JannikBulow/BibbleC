// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_ARRAY_TYPE_H
#define BIBBLEC_TYPE_ARRAY_TYPE_H

#include "BibbleC/type/type.h"

namespace bibblec {
    class BIBBLEC_EXPORT ArrayType : public Type {
    public:
        explicit ArrayType(Type* elementType);

        Type* getElementType() const;

        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isArrayType() const override;

        static ArrayType* Get(Type* elementType);
        static void Reset();

    private:
        Type* mElementType;
    };
}

#endif //BIBBLEC_TYPE_ARRAY_TYPE_H
