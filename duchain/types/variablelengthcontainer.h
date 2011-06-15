/**
    This file is part of KDevelop
    Copyright (C) 2011 Sven Brauch <svenbrauch@googlemail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/


#ifndef VARIABLELENGTHCONTAINER_H
#define VARIABLELENGTHCONTAINER_H

#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/typesystemdata.h>

#include "pythonduchainexport.h"

using namespace KDevelop;

namespace Python {
    
class KDEVPYTHONDUCHAIN_EXPORT VariableLengthContainerData : public KDevelop::StructureTypeData
{
public:
    /// Constructor
    VariableLengthContainerData()
        : KDevelop::StructureTypeData(), m_keyType(0), m_contentType(0)
    {
    }
    /// Copy constructor. \param rhs data to copy
    VariableLengthContainerData( const VariableLengthContainerData& rhs )
        : KDevelop::StructureTypeData(rhs), m_keyType(rhs.m_keyType), m_contentType(rhs.m_contentType)
    {
    }
    
    VariableLengthContainerData(const StructureTypeData& rhs)
        : KDevelop::StructureTypeData(rhs)
    {
    };
    
    AbstractType::Ptr m_keyType;
    AbstractType::Ptr m_contentType;
};


/**
* Describes something like a python list which is a list, but has a second type,
* the type of its content (for example "list of integers").
**/
class KDEVPYTHONDUCHAIN_EXPORT VariableLengthContainer : public KDevelop::StructureType
{
public:
    typedef TypePtr<VariableLengthContainer> Ptr;
    
    VariableLengthContainer();
    VariableLengthContainer(const VariableLengthContainer& rhs);
    VariableLengthContainer(const StructureType& copyFrom);
    VariableLengthContainer(StructureTypeData& data);
    void addContentType(AbstractType::Ptr typeToAdd);
    void addKeyType(AbstractType::Ptr typeToAdd);
    virtual AbstractType* clone() const;
    virtual uint hash() const;
    AbstractType::Ptr contentType() const;
    AbstractType::Ptr keyType() const;
    
    virtual bool equals(const AbstractType* rhs) const;
    
    enum {
#warning check identity value (61)
        Identity = 61
    };
    
    typedef VariableLengthContainerData Data;
    typedef KDevelop::StructureType BaseType;
    
protected:
    TYPE_DECLARE_DATA(VariableLengthContainer);
};

}

#endif // VARIABLELENGTHCONTAINER_H
