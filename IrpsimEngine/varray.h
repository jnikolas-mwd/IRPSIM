// varray.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVArray is an array variable. Inherits from CMVariable base class.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#pragma once

#include "variable.h"
#include "expresn.h"

#include "bgparray.h"

class _IRPCLASS CMVArray;

class _IRPCLASS CMVArrayIterator : public CMIrpObjectIterator
{
	CMExpressionIterator *iter;
	long pos;
	virtual const wchar_t* get_next();
public:
	CMVArrayIterator(CMVArray* v);
	~CMVArrayIterator();
};

class _IRPCLASS CMVArray : public CMVariable
{
protected:
	friend class CMVArrayIterator;
	class _IRPCLASS CMPBigArray<CMExpression> array;
	int nrows;
	int ncols;
	virtual void update_variable_links();
	virtual void read_body(wistream& s);
	virtual void write_body(wostream& s);
	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0);
	virtual CMIrpObjectIterator* create_iterator();
	int arraystate;
	enum {containsPolynomials=0x0001};
public:
	CMVArray(const string& aName,int rows=0,int cols=0);
	~CMVArray();
	void SetSize(int rows,int cols);
	void Set(int row,int col,const string& str);
   CMExpression* GetExpression(int row,int col) {return array[(long)row*ncols+col];}
	virtual string VariableType() {return GetEvalType();}
	static const wchar_t* GetEvalType() { return L"CMVArray"; }
};
