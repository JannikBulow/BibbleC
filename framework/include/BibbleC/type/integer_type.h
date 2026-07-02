// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_INTEGER_TYPE_H
#define BIBBLEC_TYPE_INTEGER_TYPE_H

#include "BibbleC/type/type.h"

namespace bibblec {
    class BIBBLEC_EXPORT IntegerType : public Type {
    public:
        enum Size : int {
            Byte = 1,
            Short = 2,
            Int = 4,
            Long = 8
        };

        IntegerType(Size size, bool isSigned);

        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getImplicitCastWarning(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isIntegerType() const override;

        bool isSigned() const;

    private:
        Size mSize;
        bool mSigned;
    };
}

#endif //BIBBLEC_TYPE_INTEGER_TYPE_H
