// vperiod.h : header file
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
#pragma once

#include "expresn.h"
#include "variable.h"
#include "timedexp.h"

#include "smparray.h"

class _IRPCLASS CMVPeriod;

class _IRPCLASS CMVPeriodIterator : public CMIrpObjectIterator
{
	CMExpressionIterator *eIterator;
	unsigned pos;
	virtual const wchar_t* get_next();
public:
	CMVPeriodIterator(CMVPeriod* v);
	~CMVPeriodIterator();
};

class _IRPCLASS CMVPeriod : public CMVariable
{
protected:
	// force evaluation for example when period list is annual eg 1995, 1996, 1997
   // but we wish to have a different evaluation every month
	enum {periodSorted=0x0001,containsPolynomials=0x0002,forceEvaluation=0x0004};

	int periodstate;
	int periodformat;
	int traceno; // 0 if main clock, 1 if first trace, etc.
	friend class CMVPeriodIterator;

	class _IRPCLASS CMPSSmallArray<CMTimedExpression> array;
	unsigned get_array_index(CMTime& t);
	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0);
	virtual CMIrpObjectIterator* create_iterator();
	virtual void reset(CMTimeMachine*);
	virtual void update_variable_links();
	virtual void read_body(wistream& s);
	virtual void write_body(wostream& s);
public:
	CMVPeriod(const CMString& aName);
	~CMVPeriod();
	void SetExpression(CMTime& t, const wchar_t* str);
	virtual CMString VariableType() {return GetEvalType();}
	static const wchar_t* GetEvalType() { return L"CMVPeriod"; }
//	CMExpression& GetExpression(CMTime& t);
};
