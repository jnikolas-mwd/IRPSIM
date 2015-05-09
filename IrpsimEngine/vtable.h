// vtable.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVTable is an IRPSIM variable defined as a table of values or expressions. 
// Inherits from CMVariable.
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
#include "indexsel.h"

#include "irp.h"
#include "bgparray.h"
#include "smvarray.h"

class _IRPCLASS CMVTable;

class _IRPCLASS CMVTableIterator : public CMVNameIterator
{
	CMVNameIterator 	*vIterator;
	CMExpressionIterator *eIterator;
	int state;
	unsigned short pos;
	virtual const wchar_t* get_next();
	enum {iterRowSel,iterColSel,iterRowVars,iterColVars,iterArray};
public:
	CMVTableIterator(CMVTable* v);
	~CMVTableIterator();
};

class _IRPCLASS CMVTable : public CMVariable
{
protected:
	friend class CMVTableIterator;
	class _IRPCLASS CMPBigArray<CMExpression> array;
	class _IRPCLASS CMVSmallArray<CMString> rowselectornames;
	class _IRPCLASS CMVSmallArray<CMString> colselectornames;
	class _IRPCLASS CMIndexSelector rowselector;
	class _IRPCLASS CMIndexSelector colselector;
	int nrows;
	int ncols;
	int tablestate;
	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0);
	virtual CMVNameIterator* create_iterator() {return new CMVTableIterator(this);}
	virtual void update_variable_links();
	virtual void read_body(wistream& s);
	virtual void write_body(wostream& s);
   enum {containsPolynomials=0x0001};
public:
	CMVTable(const CMString& aName);
	~CMVTable();
	void SetRowSelector(int n,const CMString& name);
	void SetColSelector(int n,const CMString& name);
	void SetRowValue(int row,int selno,double value);
	void SetColValue(int col,int selno,double value);
	void SetExpression(int row,int col,const CMString& str);
	CMExpression& GetExpression(int row,int col) {return *array[(long)row*ncols+col];}
	virtual CMString VariableType() {return GetEvalType();}
	static const wchar_t* GetEvalType() {return L"CMVTable";}
};