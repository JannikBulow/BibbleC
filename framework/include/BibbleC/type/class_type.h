// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_CLASS_TYPE_H
#define BIBBLEC_TYPE_CLASS_TYPE_H

#include "BibbleC/type/type.h"

namespace bibblec {
    class BIBBLEC_EXPORT ClassType : public Type {
    public:
        explicit ClassType(std::string name);

        std::string_view getName() const override;
        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isClassType() const override;

        static std::vector<ClassType*> GetAllClassTypes();
        static ClassType* Create(std::string name);
        static void Reset();

    private:
        std::string mName;
    };
}

#endif //BIBBLEC_TYPE_CLASS_TYPE_H
