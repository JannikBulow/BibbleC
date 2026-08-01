// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/member_access.h"

#include "BibbleC/type/function_type.h"

#include <BibblIR/ir/instruction/field_instruction.h>
#include <BibblIR/ir/instruction/load_instruction.h>

#include <BibblIR/ir/instruction.h>

namespace bibblec::parser {
    MemberAccess::MemberAccess(scope::Scope* scope, ASTNodePtr object, std::string id, SourcePair source)
        : ASTNode(scope, source)
        , mObject(std::move(object))
        , mId(std::move(id))
        , mClassType(nullptr) {}

    std::vector<ASTNode*> MemberAccess::getChildren() {
        return {mObject.get()};
    }

    bibblir::Value* MemberAccess::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Value* object = mObject->codegen(builder, module, diag);

        bibblir::Value* getfield = builder.createGetField(object, mClassType->getBibblirField(mId));
        return builder.createLoad(getfield);
    }

    void MemberAccess::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mObject->typeCheck(diag, exit);

        if (!mObject->getType()->isClassType()) {
            diag.reportCompilerError(mSource,
                std::format("'{}operator.{}' used on non-object value",
                    fmt::bold, fmt::reset)
            );
            exit = true;
            mType = Type::Get("error-type");
            return;
        }
        mClassType = static_cast<ClassType*>(mObject->getType());

        ClassType::Field* field = mClassType->getField(mId);
        if (field) {
            mType = field->type;
        } else {
            auto functions = mScope->getCandidateFunctions(mId);
            if (!functions.empty()) {
                for (scope::Symbol* function : functions) {
                    if (auto* functionType = dynamic_cast<FunctionType*>(function->type)) {
                        if (functionType->getArgumentTypes()[0] == mClassType) {
                            return;
                        }
                    }
                }
            }

            diag.reportCompilerError(mSource,
                std::format("class '{}{}{}' has no member named '{}{}{}'",
                    fmt::bold, mClassType->getName(), fmt::reset,
                    fmt::bold, mId, fmt::reset)
            );
            exit = true;
            mType = Type::Get("error-type");
        }
    }
}
