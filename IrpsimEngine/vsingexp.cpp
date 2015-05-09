// vsingexp.cpp : implementation file
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
#include "vsingexp.h"
//#include <fstream>
//static wofstream sdebug("debug_vsingexp.txt");

CMVSingleExpressionIterator::CMVSingleExpressionIterator(CMVSingleExpression* v) :
CMVNameIterator(v),
iter(v->expression)
{
}

const wchar_t* CMVSingleExpressionIterator::get_next()
{
	return iter();
}

CMVNameIterator* CMVSingleExpression::create_iterator()
{
	return expression.IsPolynomial() ? new CMVSingleExpressionIterator(this) : 0;
}

void CMVSingleExpression::read_body(wistream& s)
{
	CMString str;
	CMString expr;

	while(!s.eof()) {
		str.read_line(s);
		if (str.is_null() || str[0] == L'*')
			continue;
		if (str(0,wcslen(vardef_end)) == vardef_end)
			break;
		expr += str;
	}
	SetExpression(expr.c_str());
	if (expression.Fail()) {
		SetState(vsFailed,1);
     	ReportError(XBadVardef,expression.GetString());
	}
}

void CMVSingleExpression::write_body(wostream& s)
{
	s << expression << ENDL;
}

