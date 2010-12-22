/*****************************************************************************
 * Copyright (c) 2007 Piyush verma <piyush.verma@gmail.com>                  *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/
#include "usebuilder.h"
#include "pythoneditorintegrator.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <parsesession.h>
#include <usebuilder.h>
#include "ast.h"
#include <language/duchain/types/structuretype.h>

using namespace KTextEditor;
using namespace KDevelop;

namespace Python {

UseBuilder::UseBuilder (PythonEditorIntegrator* editor) : m_editor(editor)
{
}

void UseBuilder::buildUses(Ast* node)
{
    UseBuilderBase::buildUses(node);
}


void UseBuilder::visitName(NameAst* node)
{
    DUChainWriteLocker lock(DUChain::lock());
    QList<Declaration*> declarations = currentContext()->findDeclarations(identifierForNode(node->identifier), editorFindRange(node, node).start);
    QList<Declaration*> localDeclarations = currentContext()->findLocalDeclarations(identifierForNode(node->identifier).last(), editorFindRange(node, node).end);
    Declaration* declaration;
    if ( localDeclarations.length() ) {
        declaration = localDeclarations.last();
        kDebug() << "Using local declaration";
    }
    else if ( declarations.length() ) {
        declaration = declarations.last();
        kDebug() << "Using global declaration";
    }
    else declaration = 0;
    kDebug() << currentContext()->type() << currentContext()->scopeIdentifier() << currentContext()->range().castToSimpleRange();
    
    Q_ASSERT(node->identifier);
    Q_ASSERT(node->hasUsefulRangeInformation); // TODO remove this!
    RangeInRevision useRange(node->identifier->startLine, node->identifier->startCol, node->identifier->endLine, node->identifier->endCol + 1);
    
    if ( declaration && declaration->range() == useRange ) return;
    
    kDebug() << " Registering use for " << node->identifier->value << " at " << useRange.castToSimpleRange() << "with dec" << declaration;
    UseBuilderBase::newUse(node, useRange, DeclarationPointer(declaration));
}

void UseBuilder::visitAttribute(AttributeAst* node)
{
    kDebug() << "VisitAttribute start";
    UseBuilderBase::visitAttribute(node);
    kDebug() << "Visit Attribute base end";
    NameAst* accessingAttributeOf = dynamic_cast<NameAst*>(node->value);
    if ( ! accessingAttributeOf ) {
        kWarning() << "Accessing attributes of non-names not implemented, aborting";
        return;
    }
    QList<Declaration*> availableDeclarations = currentContext()->findDeclarations(identifierForNode(accessingAttributeOf->identifier), editorFindRange(node, node).start);
    Declaration* accessingAttributeOfDeclaration;
    if ( availableDeclarations.length() > 0 ) accessingAttributeOfDeclaration = availableDeclarations.last();
    else {
        kWarning() << "No declaration found to look up type of attribute in! This is probably something wrong in YOUR code. :)";
        return; // TODO report error
    }
    
    TypePtr<StructureType> accessingAttributeOfType = accessingAttributeOfDeclaration->type<StructureType>();
    DUContext* searchAttrInContext = accessingAttributeOfType.unsafeData()->declaration(currentContext()->topContext())->internalContext();
    QList<Declaration*> foundDecls = searchAttrInContext->findDeclarations(identifierForNode(node->attribute), CursorInRevision::invalid(), 
                                                                           KDevelop::AbstractType::Ptr(), searchAttrInContext->topContext());
    
    RangeInRevision useRange(node->attribute->startLine, node->attribute->startCol, node->attribute->endLine, node->attribute->endCol);
    
    if ( foundDecls.length() > 0 ) {
        kDebug() << "Creating a new attribute declaration:" << useRange.castToSimpleRange() << ", Declaration:" << foundDecls.last()->range();
        UseBuilderBase::newUse(node, useRange, DeclarationPointer(foundDecls.last()));
    }
    else {
        kWarning() << "No declaration found for attribute";
        UseBuilderBase::newUse(node, useRange, DeclarationPointer(0));
    }
    kDebug() << "VisitAttribute end";
}

// void UseBuilder::openContext(DUContext * newContext)
// {
//   UseBuilderBase::openContext(newContext);
//   m_nextUseStack.push(0);
// }
// 
// void UseBuilder::closeContext()
// {
//   UseBuilderBase::closeContext();
//   m_nextUseStack.pop();
// }


ParseSession *UseBuilder::parseSession() const
{
    return m_session;
}

}
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
