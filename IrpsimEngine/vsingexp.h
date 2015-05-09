// vsingexp.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVSingleExpression is a variable defined by a single expression.
// Inherits from CMVariable base class.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#pragma once

#include "expresn.h"
#include "variable.h"

class _IRPCLASS CMVSingleExpression;

class _IRPCLASS CMVSingleExpressionIterator : public CMVNameIterator
{
	CMExpressionIterator iter;
	virtual const wchar_t* get_next();
public:
	CMVSingleExpressionIterator(CMVSingleExpression* v);
};

class _IRPCLASS CMVSingleExpression : public CMVariable
{
protected:
	friend class CMVSingleExpressionIterator;
	CMExpression expression;

	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0) {return expression.Evaluate(t);}
	virtual CMVNameIterator* create_iterator();
	virtual void update_variable_links() {expression.UpdateVariableLinks();}
	virtual void read_body(wistream& s);
	virtual void write_body(wostream& s);
public:
	CMVSingleExpression(const CMString& aName, const wchar_t* aExp = 0) : CMVariable(aName), expression(aExp) {}
	void SetExpression(const wchar_t* exp) { expression = exp; }
	virtual CMString VariableType() {return GetEvalType();}
	static const wchar_t* GetEvalType() {return L"CMVSingleExpression";}
};