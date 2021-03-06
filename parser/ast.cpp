/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2010-2012 Sven Brauch <svenbrauch@googlemail.com>           *
 *   Copyright 2012 Patrick Spendrin <ps_ml@gmx.de>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "ast.h"
#include "astbuilder.h"
#include <language/duchain/problem.h>

namespace Python
{

// We never need actual constructors for AST nodes, but it seems to be required, at least for some platforms
// so we provide pseudo implementations
// there's nothing happening here, don't bother reading the code
    
Ast::Ast( Ast* parent, Ast::AstType type ) : parent(parent), astType( type ), startCol(0), startLine(-99999), endCol(0), endLine(0), hasUsefulRangeInformation(false), context(nullptr) { }
Ast::Ast() :  parent(nullptr), startCol(0), startLine(-5), endCol(0), endLine(0), hasUsefulRangeInformation(false), context(nullptr) { }

ArgumentsAst::ArgumentsAst(Ast* parent): Ast(parent, Ast::ArgumentsAstType)
{
    
}

ArgAst::ArgAst(Ast* parent): Ast(parent, Ast::ArgAstType), argumentName(nullptr), annotation(nullptr)
{

}

AssertionAst::AssertionAst(Ast* parent): StatementAst(parent, Ast::AssertionAstType) 
{
    
}

AssignmentAst::AssignmentAst(Ast* parent): StatementAst(parent, Ast::AssignmentAstType), value(nullptr)
{
    
}

AttributeAst::AttributeAst(Ast* parent): ExpressionAst(parent, Ast::AttributeAstType), value(nullptr), depth(0)
{
    
}

AugmentedAssignmentAst::AugmentedAssignmentAst(Ast* parent): StatementAst(parent, Ast::AugmentedAssignmentAstType), value(nullptr)
{
    
}

AnnotationAssignmentAst::AnnotationAssignmentAst(Ast* parent): StatementAst(parent, Ast::AnnotationAssignmentAstType), target(nullptr), value(nullptr), annotation(nullptr)
{

}

BinaryOperationAst::BinaryOperationAst(Ast* parent): ExpressionAst(parent, Ast::BinaryOperationAstType), lhs(nullptr), rhs(nullptr)
{

}

BooleanOperationAst::BooleanOperationAst(Ast* parent): ExpressionAst(parent, Ast::BooleanOperationAstType)
{
    
}

BreakAst::BreakAst(Ast* parent): StatementAst(parent, Ast::BreakAstType)
{
    
}

CallAst::CallAst(Ast* parent): ExpressionAst(parent, Ast::CallAstType), function(nullptr)
{
    
}

ClassDefinitionAst::ClassDefinitionAst(Ast* parent): StatementAst(parent, Ast::ClassDefinitionAstType), name(nullptr)
{
    
}

CodeAst::CodeAst() : Ast(nullptr, Ast::CodeAstType), name(nullptr)
{
}

CodeAst::~CodeAst()
{
    free_ast_recursive(this);
}

CompareAst::CompareAst(Ast* parent): ExpressionAst(parent, Ast::CompareAstType), leftmostElement(nullptr)
{
    
}

ComprehensionAst::ComprehensionAst(Ast* parent): Ast(parent, Ast::ComprehensionAstType), target(nullptr), iterator(nullptr)
{
    
}

ContinueAst::ContinueAst(Ast* parent): StatementAst(parent, Ast::ContinueAstType)
{
    
}

DeleteAst::DeleteAst(Ast* parent): StatementAst(parent, Ast::DeleteAstType)
{
    
}

DictAst::DictAst(Ast* parent): ExpressionAst(parent, Ast::DictAstType)
{

}

IndexAst::IndexAst(Ast* parent): SliceAstBase(parent, Ast::IndexAstType), value(nullptr)
{

}

SliceAst::SliceAst(Ast* parent): SliceAstBase(parent, Ast::SliceAstType), lower(nullptr), upper(nullptr), step(nullptr)
{

}

DictionaryComprehensionAst::DictionaryComprehensionAst(Ast* parent): ExpressionAst(parent, Ast::DictionaryComprehensionAstType), key(nullptr), value(nullptr)
{
    
}

EllipsisAst::EllipsisAst(Ast* parent): SliceAstBase(parent, Ast::EllipsisAstType)
{
    
}

ExceptionHandlerAst::ExceptionHandlerAst(Ast* parent): Ast(parent, Ast::ExceptionHandlerAstType), type(nullptr), name(nullptr)
{
    
}

ListComprehensionAst::ListComprehensionAst(Ast* parent): ExpressionAst(parent, Ast::ListComprehensionAstType), element(nullptr)
{

}

ExpressionAst::ExpressionAst(Ast* parent, AstType type): Ast(parent, type), value(nullptr)
{
    
}

YieldFromAst::YieldFromAst(Ast* parent) : ExpressionAst(parent, Ast::YieldFromAstType)
{

}

ExtendedSliceAst::ExtendedSliceAst(Ast* parent): SliceAstBase(parent, Ast::ExtendedSliceAstType)
{
    
}

ForAst::ForAst(Ast* parent): StatementAst(parent, Ast::ForAstType), target(nullptr), iterator(nullptr)
{
    
}

FunctionDefinitionAst::FunctionDefinitionAst(Ast* parent): StatementAst(parent, Ast::FunctionDefinitionAstType), name(nullptr), arguments(nullptr), async(false)
{
    
}

GeneratorExpressionAst::GeneratorExpressionAst(Ast* parent): ExpressionAst(parent, Ast::GeneratorExpressionAstType), element(nullptr)
{
    
}

GlobalAst::GlobalAst(Ast* parent): StatementAst(parent, Ast::GlobalAstType)
{
    
}

Identifier::Identifier(QString value) : Ast(nullptr, Ast::IdentifierAstType), value(value)
{
    
}

IfAst::IfAst(Ast* parent): StatementAst(parent, Ast::IfAstType), condition(nullptr)
{
    
}

IfExpressionAst::IfExpressionAst(Ast* parent): ExpressionAst(parent, Ast::IfExpressionAstType), condition(nullptr)
{
    
}

ImportAst::ImportAst(Ast* parent): StatementAst(parent, Ast::ImportAstType)
{
    
}

ImportFromAst::ImportFromAst(Ast* parent): StatementAst(parent, Ast::ImportFromAstType), module(nullptr), level(0)
{
    
}

KeywordAst::KeywordAst(Ast* parent): Ast(parent, Ast::KeywordAstType), argumentName(nullptr), value(nullptr)
{
    
}

LambdaAst::LambdaAst(Ast* parent): ExpressionAst(parent, Ast::LambdaAstType), arguments(nullptr)
{
    
}

ListAst::ListAst(Ast* parent): ExpressionAst(parent, Ast::ListAstType)
{
    
}

NameAst::NameAst(Ast* parent): ExpressionAst(parent, Ast::NameAstType), identifier(nullptr)
{
    
}

AwaitAst::AwaitAst(Ast* parent): ExpressionAst(parent, Ast::AwaitAstType), value(nullptr)
{

}

NameConstantAst::NameConstantAst(Ast* parent): ExpressionAst(parent, Ast::NameConstantAstType), value(Invalid)
{

}

NumberAst::NumberAst(Ast* parent): ExpressionAst(parent, Ast::NumberAstType), value(0)
{
    
}

PassAst::PassAst(Ast* parent): StatementAst(parent, Ast::PassAstType)
{
    
}

NonlocalAst::NonlocalAst(Ast* parent): StatementAst(parent, Ast::NonlocalAstType)
{
    
}

RaiseAst::RaiseAst(Ast* parent): StatementAst(parent, Ast::RaiseAstType), type(nullptr)
{
    
}

ReturnAst::ReturnAst(Ast* parent): StatementAst(parent, Ast::ReturnAstType), value(nullptr)
{
    
}

SetAst::SetAst(Ast* parent): ExpressionAst(parent, Ast::SetAstType)
{
    
}

SetComprehensionAst::SetComprehensionAst(Ast* parent): ExpressionAst(parent, Ast::SetComprehensionAstType), element(nullptr)
{
    
}

SliceAstBase::SliceAstBase(Ast* parent, AstType type): Ast(parent, type)
{
    
}

StatementAst::StatementAst(Ast* parent, AstType type): Ast(parent, type)
{
    
}

StringAst::StringAst(Ast* parent): ExpressionAst(parent, Ast::StringAstType), value(""), usedAsComment(false)
{

}

JoinedStringAst::JoinedStringAst(Ast* parent): ExpressionAst(parent, Ast::JoinedStringAstType), values()
{

}

FormattedValueAst::FormattedValueAst(Ast* parent): ExpressionAst(parent, Ast::FormattedValueAstType), value(nullptr), conversion(0), formatSpec(nullptr)
{

}

BytesAst::BytesAst(Ast* parent): ExpressionAst(parent, Ast::BytesAstType), value("")
{
    
}

SubscriptAst::SubscriptAst(Ast* parent): ExpressionAst(parent, Ast::SubscriptAstType), value(nullptr), slice(nullptr)
{
    
}

StarredAst::StarredAst(Ast* parent): ExpressionAst(parent, Ast::StarredAstType)
{
    
}

TupleAst::TupleAst(Ast* parent): ExpressionAst(parent, Ast::TupleAstType)
{
    
}

UnaryOperationAst::UnaryOperationAst(Ast* parent): ExpressionAst(parent, Ast::UnaryOperationAstType), operand(nullptr)
{
    
}

TryAst::TryAst(Ast* parent): StatementAst(parent, Ast::TryAstType)
{

}

WhileAst::WhileAst(Ast* parent): StatementAst(parent, Ast::WhileAstType), condition(nullptr)
{
    
}

WithAst::WithAst(Ast* parent): StatementAst(parent, Ast::WithAstType)
{
    
}

WithItemAst::WithItemAst(Ast* parent): Ast(parent, Ast::WithItemAstType)
{

}

YieldAst::YieldAst(Ast* parent): ExpressionAst(parent, Ast::YieldAstType), value(nullptr)
{
    
}

AliasAst::AliasAst(Ast* parent): Ast(parent, Ast::AliasAstType), name(nullptr), asName(nullptr)
{
    
}



}
