// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/variable_expression.h"

namespace bibblec::parser {
    VariableExpression::VariableExpression(scope::Scope* scope, std::string name, SourcePair source, std::string module)
        : ASTNode(scope, source)
        , mModule(std::move(module))
        , mName(std::move(name)) {}

    std::string_view VariableExpression::getModule() const {
        return mModule;
    }

    std::string_view VariableExpression::getName() const {
        return mName;
    }

    bool VariableExpression::isQualified() const {
        return !mModule.empty();
    }

    scope::Symbol* VariableExpression::resolveSymbol() const {
        if (isQualified()) return mScope->resolveQualifiedSymbol(mModule, mName);
        return mScope->resolveSymbol(mName);
    }

    ASTNodePtr VariableExpression::cloneExternal(scope::Scope* in) {
        return std::make_unique<VariableExpression>(in, mName, mSource, isQualified() ? mModule : std::string(mScope->getModuleName()));
    }

    bibblir::Value* VariableExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        scope::Symbol* symbol = resolveSymbol();

        if (symbol->constant) {
            if (symbol->values.size() != 1) {
                diag.reportCompilerError(mSource, "this error should not be reached");
                std::exit(1);
            }

            return symbol->values[0].value;
        }

        scope::SymbolValue* latestValue = symbol->getLatestValue(builder.getInsertPoint());
        if (!latestValue) {
            return nullptr; // there should probably be an error here
        }

        return latestValue->value;
    }

    void VariableExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        scope::Symbol* symbol = resolveSymbol();

        if (!symbol) {
            if (isQualified()) {
                diag.reportCompilerError(mSource,
                    std::format("module '{}{}{}' has no symbol named '{}{}{}'",
                        fmt::bold, mModule, fmt::reset,
                        fmt::bold, mName, fmt::reset));
            } else {
                diag.reportCompilerError(mSource,
                    std::format("undeclared identifier '{}{}{}'",
                        fmt::bold, mName, fmt::reset)
                );
            }
            exit = true;
            mType = Type::Get("error-type");
        } else if (!symbol->type) {
            diag.reportCompilerError(mSource,
                std::format("'{}{}{}' names a type, not a value",
                    fmt::bold, mName, fmt::reset));
            exit = true;
        } else {
            mType = symbol->type;
        }
    }
}
