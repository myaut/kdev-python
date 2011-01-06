/*****************************************************************************
 * Copyright 2010 (c) Miquel Canes Gonzalez <miquelcanes@gmail.com>          *
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

#ifndef EXPRESSIONVISITOR_H
#define EXPRESSIONVISITOR_H

#include <astdefaultvisitor.h>
#include <language/duchain/types/abstracttype.h>
#include <QHash>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/typesystemdata.h>

#include "pythonduchainexport.h"
#include <language/duchain/types/typeregister.h>
#include <KLocalizedString>

namespace KDevelop {
class Identifier;
}

using namespace KDevelop;

namespace Python
{
    
typedef KDevelop::IntegralTypeData IntegralTypeExtendedData;
class KDEVPYTHONDUCHAIN_EXPORT IntegralTypeExtended : public KDevelop::IntegralType {
public:
    typedef TypePtr<IntegralTypeExtended> Ptr;
    enum PythonIntegralTypes {
        TypeList = KDevelop::IntegralType::TypeLanguageSpecific,
        TypeDict = KDevelop::IntegralType::TypeLanguageSpecific + 1
    };
    IntegralTypeExtended(uint type = TypeNone) : IntegralType(createData<IntegralTypeExtended>()) {
        setDataType(type);
        setModifiers(ConstModifier);
    };
    IntegralTypeExtended(IntegralTypeExtendedData& data) : IntegralType(data) { }
    
    virtual QString toString() const {
        switch ( d_func()->m_dataType ) {
            case TypeList: return i18n("list");
            case TypeDict: return i18n("dictionary");
            default: break;
        }
        
        return KDevelop::IntegralType::toString();
    };
    
    enum {
        Identity = 60 // TODO ok?
    };
    
    typedef KDevelop::IntegralTypeData Data;
    typedef KDevelop::IntegralType BaseType;
protected:
    TYPE_DECLARE_DATA(IntegralTypeExtended);
};

class ExpressionVisitor : public AstDefaultVisitor
{
    public:
        ExpressionVisitor(KDevelop::DUContext* ctx);
        
        virtual void visitBinaryOperation(BinaryOperationAst* node);
        virtual void visitUnaryOperation(UnaryOperationAst* node);
        virtual void visitBooleanOperation(BooleanOperationAst* node);
        
        virtual void visitString(StringAst* node);
        virtual void visitNumber(NumberAst* node);
        virtual void visitName(NameAst* node);
        virtual void visitList(ListAst* node);
        virtual void visitDict(DictAst* node);
        virtual void visitSubscript(SubscriptAst* node);
        virtual void visitCall(CallAst* node);
        
        KDevelop::AbstractType::Ptr lastType() const { return m_lastType; }
    private:
        static QHash<KDevelop::Identifier, KDevelop::AbstractType::Ptr> s_defaultTypes;
        
        KDevelop::AbstractType::Ptr m_lastType;
        KDevelop::DUContext* m_ctx;
        
        void unknownTypeEncountered();
};

}

#endif // EXPRESSIONVISITOR_H
