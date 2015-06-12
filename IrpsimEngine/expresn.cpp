// expresn.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMExpression implements IRPSIM's expression evaluation functionality
// An expression may be either a constant or a polynomial (formula).
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include <stdlib.h>

#include "expresn.h"
#include "polynom.h"

#include "cmdefs.h"
#include "cmlib.h"

//#include <fstream>
//static wofstream sdebug(L"debug_expression.txt");

CMExpression::CMExpression(const wchar_t* str) :
type(Constant)
{
	translate(str);
}

CMExpression::CMExpression(const CMExpression& ex) :
type(ex.type)
{
	if (type == Constant)
		u.v = ex.u.v;
	else
		u.p = new CMPolynomial(*ex.u.p);
}

CMExpression::~CMExpression()
{
	if (type==Polynomial && u.p)
		delete u.p;
}

void CMExpression::translate(const wchar_t* str)
{
	if (type==Polynomial && u.p)
		delete u.p;
	type = Constant;
	if (!str)
		u.v = 0;
	else if (isnumber(str))
		u.v = _wtof(str);
	else {
		type = Polynomial;
		u.p = new CMPolynomial(str);
	}
}

void CMExpression::UpdateVariableLinks()
{
	if (type==Polynomial && u.p)
		u.p->UpdateVariableLinks();
}

double CMExpression::Evaluate(CMTimeMachine* t)
{
	if (type==Constant)
		return u.v;
	else
		return u.p->Evaluate(t);
}

int CMExpression::ContainsVariables()
{
	return (type==Constant) ? 0 :u.p->ContainsVariables();
}

int CMExpression::Fail()
{
	return (type==Constant) ? 0 : u.p->Fail();
}

CMExpression& CMExpression::operator = (const wchar_t* str)
{
	translate(str);
	return *this;
}

CMExpression& CMExpression::operator = (const CMExpression& ex)
{
	if (type==Polynomial && u.p)
		delete u.p;
	if ((type=ex.type) == Constant)
		u.v = ex.u.v;
	else
		u.p = new CMPolynomial(*ex.u.p);
	return *this;
}

int _IRPFUNC operator == (const CMExpression& e1,const CMExpression& e2)
{
	if (e1.type != e2.type)
		return 0;
	if (e1.type == e1.Constant)
		return (e1.u.v == e2.u.v);
	else
		return (*e1.u.p == *e2.u.p);
}

wostream& operator << (wostream& os,CMExpression& ex)
{
	if (ex.type==ex.Constant) os << ex.u.v;
	else os << *ex.u.p;
	return os;
}

CMString CMExpression::GetString()
{
	if (type==Constant) {
		CMString ret;
      ret += u.v;
		return ret;
	}
	else
		return CMString(u.p->GetString());
}

void CMExpression::IgnoreMissingVariables(int action)
{
	CMPolynomial::IgnoreMissingVariables(action);
}

CMExpressionIterator::CMExpressionIterator(CMExpression& ex) :
pi(0)
{
	if (ex.type==ex.Polynomial && !ex.Fail())
		pi = new CMPolynomialIterator(*ex.u.p);
}


CMExpressionIterator::~CMExpressionIterator()
{
	if (pi) delete pi;
}

const wchar_t* CMExpressionIterator::operator ()()
{
	return pi ? pi->GetNext() : 0;
}


