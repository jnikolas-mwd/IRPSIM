// vperiod.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVPeriod is an IRPSIM "period" variable defined by period-value pairs.
// E.g. 20160101 234.5
// Inherits from CMVariable base class.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#include "vperiod.h"

#include "token.h"

#include <stdlib.h>
#include <iomanip>

CMVPeriodIterator::CMVPeriodIterator(CMVPeriod* v) :
CMVNameIterator(v),
eIterator(0),
pos(0)
{
}

CMVPeriodIterator::~CMVPeriodIterator()
{
	if (eIterator) delete eIterator;
}

const wchar_t* CMVPeriodIterator::get_next()
{
	const wchar_t* ret;
	CMVPeriod* v = (CMVPeriod*) variable;

	if (!(v->periodstate & v->containsPolynomials))
		return 0;

	while (pos < v->array.Count()) {
		if (!eIterator) {
			while (pos<v->array.Count() && !v->array[pos]->Expression().ContainsVariables())
				pos++;
			if (pos >= v->array.Count())
				break;
			else
				eIterator = new CMExpressionIterator(v->array[pos]->Expression());
		}
		if (eIterator && (ret=(*eIterator)())!=0)
			return ret;
		pos++;
		if (eIterator) delete eIterator;
		eIterator = 0;
	}
	return 0;
}

CMVPeriod::CMVPeriod(const CMString& aName) :
CMVariable(aName),
array(0,32),
periodstate(0),
periodformat(CMTime::YYYY),
traceno(0)
{
}

CMVPeriod::~CMVPeriod()
{
	array.ResetAndDestroy(1);
}

CMVNameIterator* CMVPeriod::create_iterator()
{
	return (periodstate&containsPolynomials) ? new CMVPeriodIterator(this) : 0;
}

void CMVPeriod::reset(CMTimeMachine*)
{
	for (unsigned i=0;i<array.Count();i++)
		array[i]->Reset();
}

void CMVPeriod::update_variable_links()
{
	if (periodstate & containsPolynomials)
		for (unsigned i=0;i<array.Count();i++)
			array[i]->Expression().UpdateVariableLinks();
}

void CMVPeriod::SetExpression(CMTime& t, const wchar_t* str)
{
	periodstate &= ~periodSorted;
	CMTimedExpression* e = new CMTimedExpression(t,str);
	if (e->Expression().IsPolynomial()) periodstate |= containsPolynomials;
	if (e->Expression().Fail()) {
   	SetState(vsFailed,1);
      ReportError(XBadVardef,e->Expression().GetString());
   }
	array.Add(e);
}

unsigned CMVPeriod::get_array_index(CMTime& t)
{
	if (!(periodstate & periodSorted)) {
		array.Sort();
		periodstate |= periodSorted;
	}

   unsigned i;
	unsigned bottom = 0;
	unsigned top = array.Count() - 1;

//	if (t<*array[0] ||*array[top] < t)
//		throw CMXVariable(CMXVariable::XTimeOutOfRange,this,&t);

	if (t<*array[0])		return 0;
	if (*array[top] < t)  return top;

	while (top-bottom>12) {
		if (t==*array[bottom]) return bottom;
		if (t==*array[top])    return top;
		unsigned test = (bottom+top+1)/2;
		if (*array[test]<t || *array[test]==t)
			bottom=test;
		else
			top=test;
	}

	for (i=bottom;i<=top && *array[i]<t;i++);

	return (i && (i>=array.Count() || !(*array[i]==t))) ? --i : i;
}

/*
CMExpression& CMVPeriod::GetExpression(CMTime& t)
{
	return array[get_array_index(t)]->Value();
}
*/

double CMVPeriod::evaluate(CMTimeMachine* t,int,int)
{
	CMTime evaltime( (t->Cycles() > traceno) ? t->At(traceno) : t->At(0));
	if (periodformat == CMTime::MM) 
		evaltime.SetTime(2000);
	int index = get_array_index(evaltime);
	if (periodformat == CMTime::MM)
		array[index]->Reset();
	return array[index]->Evaluate(t,periodstate&forceEvaluation);
}

void CMVPeriod::read_body(wistream& s)
{
	CMString str,token,token2;
	str = GetAssociation(L"yearend");
	int yearend = str.length() ? CMTime::Month(str.c_str()) : 12;
	if (IsType(L"trace"))
		traceno = 1;
	else {
		str = GetAssociation(L"trace");
		traceno = str.length() ? _wtoi(str.c_str()) : 0;
	}
	if (IsType(L"alwaysdraw") || IsType(L"monthly")) // LEGACY
		periodstate |= forceEvaluation;
	periodformat = -1;
	array.ResetAndDestroy(1);
	while(!s.eof()) {
		str.read_line(s);
		if (str.is_null() || str[0] == L'*')
			continue;
		if (str(0,wcslen(vardef_end)) == vardef_end)
			break;
		CMTokenizer next(str);
		token = next();
		token2 = next(L"\r\n");
		switch (token.length()) {
			case 1:
			case 2:
				SetExpression(CMTime(2000,_wtoi(token.c_str())),token2.c_str());
				if (periodformat<0) periodformat = CMTime::MM;
				break;
			case 4:
				{
				int yr = _wtoi(token.c_str());
				CMTime t((yearend<12)?(yr-1):yr,yearend%12+1);
				SetExpression(t,token2.c_str());
				if (periodformat<0) periodformat = CMTime::YYYY;
				}
				break;
			case 6:
				if (periodformat<0) periodformat = CMTime::YYYYMM;
			case 8:
				if (periodformat<0) periodformat = CMTime::YYYYMMDD;
			case 10:
				if (periodformat<0) periodformat = CMTime::YYYYMMDDHH;
			case 12:
				if (periodformat<0) periodformat = CMTime::YYYYMMDDHHMM;
			default:
				if (periodformat<0) periodformat = CMTime::YYYYMMDDHHMMSS;
				SetExpression(CMTime(token),token2.c_str());
				break;
		}
	}
	array.Resize(array.Count());
}

void CMVPeriod::write_body(wostream& s)
{
	s << setiosflags(ios::left);
	if (!(periodstate & periodSorted)) {
		array.Sort();
		periodstate |= periodSorted;
	}
	int oldformat = CMTime::SetOutputFormat(periodformat);
	for (unsigned i=0;i<array.Count();i++)
		s << setw(column_width) << *array[i] << array[i]->Expression() << ENDL;
	CMTime::SetOutputFormat(oldformat);
}


