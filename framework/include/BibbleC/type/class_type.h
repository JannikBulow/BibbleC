// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_CLASS_TYPE_H
#define BIBBLEC_TYPE_CLASS_TYPE_H

#include "BibbleC/type/type.h"

namespace bibblec {
    class BIBBLEC_EXPORT ClassType : public Type {
    public:
        struct Field {
            std::string name;
            Type* type;
        };

        ClassType(std::string name, std::vector<Field> fields);

        std::vector<Field>& fields();
        bool hasField(std::string_view fieldName) { return getField(fieldName) != nullptr; }
        Field* getField(std::string_view fieldName);

        std::string_view getName() const override;
        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isClassType() const override;

        static ClassType* Get(std::string name);
        static std::vector<ClassType*> GetAllClassTypes();
        static ClassType* Create(std::string name, std::vector<Field> fields);
        static void Reset();

    private:
        std::string mName;
        std::vector<Field> mFields;
    };
}

#endif //BIBBLEC_TYPE_CLASS_TYPE_H
