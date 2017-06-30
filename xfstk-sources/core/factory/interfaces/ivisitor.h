/*
    Copyright (C) 2014  Intel Corporation

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef IVISITOR_H
#define IVISITOR_H

class IVisitor
{
public:
    virtual ~IVisitor(){}
};

template <class T, typename R = void>
class Visitor
{
public:
    typedef R ReturnType; //Available for Client
    virtual ReturnType Visit(T&) = 0;
};

template <typename R = void>
class IBaseVisitable
{
public:
    typedef R ReturnType;
    virtual ~IBaseVisitable() {}
    virtual R Accept(IVisitor&) = 0;
protected:
    template <class T>
    static ReturnType AcceptImpl(T& visited, IVisitor& guest)
    {
        //Apply the Acyclic Visitor
        if (Visitor<T, R>* p = dynamic_cast<Visitor<T, R>*>(&guest))
        {
            return p->Visit(visited);
        }
        return ReturnType();
    }
};

#define ADD_ACCEPT_FUNC() \
        virtual ReturnType Accept(IVisitor& guest) \
        { return AcceptImpl(*this, guest);}

#endif //IVISITOR_H
