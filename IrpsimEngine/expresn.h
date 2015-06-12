// expresn.h : header file
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
#pragma once

#include "irp.h"
#include "cmdefs.h"
#include "string.h"
#include <iostream>
using namespace std;

class _IRPCLASS CMVariableIterator;
class _IRPCLASS CMPolynomial;
class _IRPCLASS CMPolynomialIterator;
class _IRPCLASS CMTimeMachine;

class _IRPCLASS CMExpression
{
	friend _IRPFUNC wostream& operator << (wostream& os,CMExpression& ex);
	friend _IRPFUNC int operator == (const CMExpression& e1,const CMExpression& e2);
	friend class _IRPCLASS CMExpressionIterator;
	enum {Constant,Polynomial};

	BYTE type;
	union {
		double v;
		CMPolynomial* p;
	} u;

	void translate(const wchar_t* str);
public:
	CMExpression(const wchar_t* str = 0);
	CMExpression(const CMExpression& ex);
	~CMExpression();
	int IsPolynomial() {return (type==Polynomial);}
	int ContainsVariables();
	void UpdateVariableLinks();
	double Evaluate(CMTimeMachine* t);
	int Fail();
	string GetString();
	CMExpression& operator = (const wchar_t* str);
	CMExpression& operator = (const CMExpression& ex);
   static void IgnoreMissingVariables(int action);
};

class _IRPCLASS CMExpressionIterator
{
	CMPolynomialIterator* pi;
public:
	CMExpressionIterator(CMExpression& aExp);
	~CMExpressionIterator();
	const wchar_t* operator ()();
};