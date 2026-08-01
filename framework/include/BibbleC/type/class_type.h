// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_CLASS_TYPE_H
#define BIBBLEC_TYPE_CLASS_TYPE_H

#include "BibbleC/type/type.h"

namespace bibblec {
    class BIBBLEC_EXPORT ClassType : public Type {
    public:
        ClassType(std::string moduleName, std::string name);

        std::string_view getModuleName() const;
        std::string_view getClassName() const;

        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isClassType() const override;

        static std::vector<ClassType*> GetAllClassTypes();
        static ClassType* Get(const std::string& moduleName, const std::string& name);
        static ClassType* Create(std::string moduleName, std::string name);
        static void Reset();

    private:
        std::string mModuleName;
        std::string mClassName;
    };
}

#endif //BIBBLEC_TYPE_CLASS_TYPE_H
