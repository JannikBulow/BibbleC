// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_CLASS_TYPE_H
#define BIBBLEC_TYPE_CLASS_TYPE_H

#include "BibbleC/scope/symbol.h"

#include "BibbleC/type/type.h"

#include <BibblIR/ir/class.h>

namespace bibblec {
    class BIBBLEC_EXPORT ClassType : public Type {
    public:
        struct Member {
            Type* type;
            std::string name;
            bool isMethod;

            scope::Symbol* methodImpl; // this is ass
        };

        ClassType(std::string moduleName, std::string name);

        std::string_view getModuleName() const;
        std::string_view getClassName() const;

        const std::vector<Member>& getMembers() const;
        bool hasMember(std::string_view fieldName);
        Member* getMember(std::string_view fieldName);

        bibblir::Field* getBibblirField(std::string_view fieldName);
        bibblir::Method* getBibblirMethod(std::string_view methodName);

        bibblir::Class* getBibblirClass() const;

        void setFields(std::vector<Member> fields);
        void setBibblirClass(bibblir::Class* clas);

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

        std::optional<std::vector<Member>> mFields;
        bibblir::Class* mBibblirClass; // to get field nodes
    };
}

#endif //BIBBLEC_TYPE_CLASS_TYPE_H
