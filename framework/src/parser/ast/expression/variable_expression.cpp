// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/expression/variable_expression.h"

namespace bibblec::parser {
    VariableExpression::VariableExpression(scope::Scope* scope, std::string name, SourcePair source)
        : ASTNode(scope, source)
        , mName(std::move(name)) {}

    ASTNodePtr VariableExpression::cloneExternal(scope::Scope* in) {
        return std::make_unique<VariableExpression>(in, mName, mSource);
    }

    bibblir::Value* VariableExpression::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        scope::Symbol* symbol = mScope->resolveSymbol(mName);

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
        scope::Symbol* symbol = mScope->resolveSymbol(mName);

        if (!symbol) {
            diag.reportCompilerError(mSource,
                std::format("undeclared identifier '{}{}{}'",
                    fmt::bold, mName, fmt::reset)
            );
            exit = true;
            mType = Type::Get("error-type");
        } else {
            mType = symbol->type;
        }
    }
}
