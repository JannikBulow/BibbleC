// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_GLOBAL_FUNCTION_H
#define BIBBLEC_PARSER_AST_GLOBAL_FUNCTION_H

#include "BibbleC/lexer/Token.h"

#include "BibbleC/parser/ast/node.h"

#include "BibbleC/type/function_type.h"

#include <bitset>
#include <string>
#include <vector>

namespace bibblec::parser {
    enum class FunctionModifier {
        Native,

        // Not a modifier
        Count
    };

    using FunctionModifiers = std::bitset<static_cast<size_t>(FunctionModifier::Count)>;

    struct BIBBLEC_EXPORT FunctionArgument {
        Type* type;
        std::string name;

        scope::Symbol* symbol;

        FunctionArgument(Type* type, std::string name);
    };

    class BIBBLEC_EXPORT Function : public ASTNode {
    public:
        Function(
            std::vector<lexer::Token> modifierTokens,
            std::string name,
            FunctionType* type,
            std::vector<FunctionArgument> arguments,
            scope::ScopePtr ownScope,
            std::vector<ASTNodePtr> body,
            SourcePair source,
            SourcePair blockEnd
        );

        Function(
            FunctionModifiers modifiers,
            std::string name,
            FunctionType* type,
            std::vector<FunctionArgument> arguments,
            scope::ScopePtr ownScope,
            std::vector<ASTNodePtr> body,
            SourcePair source,
            SourcePair blockEnd
        );

        std::vector<ASTNode*> getChildren() override;

        ASTNodePtr cloneExternal(scope::Scope* in) override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;
        void setEmittedValue(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        FunctionModifiers mModifiers;
        std::string mName;
        std::vector<FunctionArgument> mArguments;
        std::vector<ASTNodePtr> mBody;
        SourcePair mBlockEnd;

        scope::ScopePtr mOwnScope;
        scope::Symbol* mSymbol = nullptr; // assigned in constructorImpl

        void constructorImpl(FunctionType* type);
        std::string mangleName();
    };

    using FunctionPtr = std::unique_ptr<Function>;
}

#endif //BIBBLEC_PARSER_AST_GLOBAL_FUNCTION_H
