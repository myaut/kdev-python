/*****************************************************************************
 * Copyright (c) 2011 Sven Brauch <svenbrauch@googlemail.com>                *
 *                                                                           *
 * This program is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU General Public License as            *
 * published by the Free Software Foundation; either version 2 of            *
 * the License, or (at your option) any later version.                       *
 *                                                                           *           
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *   
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************
 */
#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/duchainutils.h>

#include <KTextEditor/View>
#include <KTextEditor/Document>

#include "implementfunctioncompletionitem.h"

using namespace KDevelop;
using namespace KTextEditor;

namespace Python {

ImplementFunctionCompletionItem::ImplementFunctionCompletionItem(const QString& name, const QString& arguments, const QString& writeArguments, const QString& previousIndent)
    : m_arguments(arguments), m_writeArguments(writeArguments), m_name(name), m_previousIndent(previousIndent)
{
    
}

void ImplementFunctionCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    const QString finalText = m_name + "(" + m_writeArguments + "):";
    document->replaceText(word, finalText);
    document->insertLine(word.start().line() + 1, m_previousIndent + "    "); // 4 spaces is indentation for python. everyone does it like this. you must, too.
    if ( View* view = document->activeView() ) {
        view->setCursorPosition(Cursor(word.end().line() + 1, m_previousIndent.length() + 4));
    }
}

QVariant ImplementFunctionCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            switch ( index.column() ) {
                case KDevelop::CodeCompletionModel::Name:
                    return m_name + "(" + m_arguments + ")";
                case KDevelop::CodeCompletionModel::Postfix:
                    return "";
                case KDevelop::CodeCompletionModel::Prefix:
                    return "implement method";
                default:
                    return "";
            }
        case Qt::DecorationRole:
            if( index.column() == KDevelop::CodeCompletionModel::Icon ) {
                KDevelop::CodeCompletionModel::CompletionProperties p(KDevelop::CodeCompletionModel::Function);
                return DUChainUtils::iconForProperties(p);
            }
        default: return CompletionTreeItem::data(index, role, model);
    }
}

} // namespace Python