// timedexp.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Implements the CTimedExpression class used in polynomial evaluation.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#if !defined (__TIMEVAL_H)
#define __TIMEVAL_H

#include "expresn.h"

#include "cmdefs.h"
#include "cmtime.h"

class CMTimedExpression : public CMTime
{
	CMExpression expression;
	double value;
public:
	CMTimedExpression() : CMTime() , expression() , value(CM_NODOUBLE) {}
	CMTimedExpression(const CMTime& t,const CMExpression& exp) : CMTime(t) , expression(exp) , value(CM_NODOUBLE) {}
	CMTimedExpression(const CMTime& t, const wchar_t* str) : CMTime(t), expression(str), value(CM_NODOUBLE) {}
	CMTimedExpression(const CMTimedExpression& t) : CMTime((const CMTime&)t) , expression(t.expression) , value(t.value) {}
	void Reset() {value=CM_NODOUBLE;}
	CMTimedExpression& operator = (const CMTimedExpression& t)
		{CMTime::operator = (t); expression=t.expression;value=t.value; return *this;}
	CMExpression& Expression() {return expression;}
	double Evaluate(CMTimeMachine* t,int force)
		{if (value==CM_NODOUBLE || force) value=expression.Evaluate(t);return value;}
};

#endif




