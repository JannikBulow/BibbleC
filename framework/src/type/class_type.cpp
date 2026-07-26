// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/class_type.h"

namespace bibblec {
    ClassType::ClassType(std::string name, std::vector<Field> fields)
        : Type("class " + name)
        , mName(std::move(name))
        , mFields(std::move(fields)) {

    }

    std::vector<ClassType::Field>& ClassType::fields() {
        return mFields;
    }

    ClassType::Field* ClassType::getField(std::string_view fieldName) {
        auto it = std::ranges::find_if(mFields, [&fieldName](const Field& field) {
            return field.name == fieldName;
        });
        if (it == mFields.end()) return nullptr;
        return &*it;
    }

    std::string_view ClassType::getName() const {
        return mName;
    }

    int ClassType::getSize() const {
        return 8;
    }

    bibblir::Type* ClassType::getBibblirType() const {
        std::vector<bibblir::Type*> fieldTypes;
        for (auto field : mFields) {
            fieldTypes.push_back(field.type->getBibblirType());
        }
        return bibblir::Type::GetClassType();
    }

    Type::CastLevel ClassType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string ClassType::getSymbolID(Type* thisType) const {
        return "C" + mName + ";";
    }

    bool ClassType::isClassType() const {
        return true;
    }

    static std::vector<std::unique_ptr<ClassType>> classTypes;

    ClassType* ClassType::Get(std::string name) {
        auto it = std::ranges::find_if(classTypes, [&name](const auto& type) {
            return type->getName() == name;
        });
        if (it == classTypes.end()) return nullptr;
        return it->get();
    }

    std::vector<ClassType*> ClassType::GetAllClassTypes() {
        std::vector<ClassType*> result;
        result.reserve(classTypes.size());
        for (const auto& type : classTypes) {
            result.emplace_back(type.get());
        }
        return result;
    }

    ClassType* ClassType::Create(std::string name, std::vector<Field> fields) {
        ClassType* found = Get(name);
        if (found) return found;

        classTypes.push_back(std::make_unique<ClassType>(std::move(name), std::move(fields)));
        return classTypes.back().get();
    }

    void ClassType::Reset() {
        classTypes.clear();
    }
}
