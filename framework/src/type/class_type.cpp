// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/class_type.h"

#include <cassert>
#include <format>

namespace bibblec {
    ClassType::ClassType(std::string moduleName, std::string name)
        : Type(std::format("class {}::{}", moduleName, name))
        , mModuleName(std::move(moduleName))
        , mClassName(std::move(name))
        , mBibblirClass(nullptr) {}

    std::string_view ClassType::getModuleName() const {
        return mModuleName;
    }

    std::string_view ClassType::getClassName() const {
        return mClassName;
    }

    const std::vector<ClassType::Field>& ClassType::getFields() const {
        assert(mFields.has_value());
        return mFields.value();
    }

    bool ClassType::hasField(std::string_view fieldName) {
        assert(mFields.has_value());
        return getField(fieldName) != nullptr;
    }

    ClassType::Field* ClassType::getField(std::string_view fieldName) {
        assert(mFields.has_value());

        auto it = std::ranges::find_if(*mFields, [&fieldName](const Field& field) {
            return field.name == fieldName;
        });
        if (it == mFields->end()) return nullptr;

        return &*it;
    }

    bibblir::Field* ClassType::getBibblirField(std::string_view fieldName) {
        Field* field = getField(fieldName);
        if (field == nullptr) return nullptr;

        return mBibblirClass->addField(field->type->getBibblirType(), field->name);
    }

    void ClassType::setFields(std::vector<Field> fields) {
        assert(!mFields.has_value());
        mFields = std::move(fields);
    }

    void ClassType::setBibblirClass(bibblir::Class* clas) {
        mBibblirClass = clas;
    }

    int ClassType::getSize() const {
        return 8;
    }

    bibblir::Type* ClassType::getBibblirType() const {
        return bibblir::Type::GetClassType(mModuleName, mClassName);
    }

    Type::CastLevel ClassType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string ClassType::getSymbolID(Type* thisType) const {
        return std::format("C{}/{};", mModuleName, mClassName);
    }

    bool ClassType::isClassType() const {
        return true;
    }

    static std::vector<std::unique_ptr<ClassType>> classTypes;

    std::vector<ClassType*> ClassType::GetAllClassTypes() {
        std::vector<ClassType*> result;
        result.reserve(classTypes.size());
        for (const auto& type : classTypes) {
            result.emplace_back(type.get());
        }
        return result;
    }

    ClassType* ClassType::Get(const std::string& moduleName, const std::string& name) {
        auto it = std::ranges::find_if(classTypes, [&name, &moduleName](const auto& type) { return type->getClassName() == name && type->getModuleName() == moduleName; });
        if (it != classTypes.end()) return it->get();
        return nullptr;
    }

    ClassType* ClassType::Create(std::string moduleName, std::string name) {
        ClassType* found = Get(moduleName, name);
        if (found) return found;

        classTypes.push_back(std::make_unique<ClassType>(std::move(moduleName), std::move(name)));
        return classTypes.back().get();
    }

    void ClassType::Reset() {
        classTypes.clear();
    }
}
