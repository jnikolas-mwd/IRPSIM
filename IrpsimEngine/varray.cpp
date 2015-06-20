// varray.cpp : implementation file
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
#include "StdAfx.h"
#include "varray.h"

#include "cmlib.h"

#include <iomanip>
#include <stdlib.h>

#include <fstream>
static wofstream sdebug("debug_varray.txt");

CMVArrayIterator::CMVArrayIterator(CMVArray* v) :
CMIrpObjectIterator(v),
iter(0),
pos(0)
{
}

CMVArrayIterator::~CMVArrayIterator()
{
	if (iter) delete iter;
}

const wchar_t* CMVArrayIterator::get_next()
{
	const wchar_t* ret;
	CMVArray* v = (CMVArray*)obj;

	if (!(v->arraystate & v->containsPolynomials)) {
		return 0;
	}
	while (pos < v->_size) {
		if (!iter) {
			//while (pos<v->array.Count() && !v->array[pos]->ContainsVariables())
			while (pos<v->_size && (v->array[pos].e==NULL || !v->array[pos].e->ContainsVariables()))
				pos++;
			if (pos >= v->_size)
				break;
			else
				iter = new CMExpressionIterator(*v->array[pos].e);
				//iter = new CMExpressionIterator(*v->array[pos]);
		}
		if (iter && (ret=(*iter)())!=0)
			return ret;
		pos++;
		if (iter) delete iter;
		iter = 0;
	}

	return 0;
}

CMVArray::CMVArray(const CMString& aName,int rows,int cols) :
CMVariable(aName)
{
	SetState(vsDontMaintain,TRUE);
	SetSize(rows,cols);
}

CMVArray::~CMVArray()
{
	//array.ResetAndDestroy(1);
	reset();
	delete[] array;
}

CMIrpObjectIterator* CMVArray::create_iterator()
{
	return (arraystate&containsPolynomials) ? new CMVArrayIterator(this) : 0;
}

void CMVArray::reset()
{
	if (array == NULL) return;
	for (size_t i = 0; i < _size; i++)
		if (array[i].e != NULL)
			delete array[i].e;
}

void CMVArray::SetSize(int rows,int cols)
{
	reset();
	if (array != NULL)
		delete[] array;
	_nrows = rows;
	_ncols = cols;
	_size = rows*cols;

	if (_size == 0)
		return;

	array = new DoubleOrExpression[_size];
	for (size_t i = 0; i < _size; i++) {
		array[i].e = NULL;
		array[i].dVal = 0;
	}
	/*
	long n = (long)nrows*ncols;
	for (long i=0;i<n;i++)
		array.AddAt(i,0);
	*/
}

double CMVArray::evaluate(CMTimeMachine* t,int index1,int index2)
{
	if (index1>0 && index1 <= _nrows && index2 > 0 && index2 <= _ncols) {
		//return array[(long)(index1 - 1)*ncols + index2 - 1]->Evaluate(t);
		size_t index = (index1 - 1)*_ncols + index2 - 1;
		if (array[index].e != NULL) {
			return array[index].e->Evaluate(t);
		}
		//sdebug << GetName() << " at " << index1 << " " << index2 << " evaluates to " << array[index].dVal << endl;
		return array[index].dVal;
	}
	return 0;
}

void CMVArray::update_variable_links()
{
	/*
	if (arraystate & containsPolynomials)
		for (long j = 0; j<array.Count(); j++)
			array[j]->UpdateVariableLinks();
    */

	if (arraystate & containsPolynomials) {
		for (size_t i = 0; i < _size; i++)
			if (array[i].e!=NULL)
				array[i].e->UpdateVariableLinks();
	}
}

void CMVArray::Set(int row,int col,const CMString& str)
{
	if (row>=0 && row<_nrows && col>=0 && col<_ncols) {
		size_t index = row*_ncols + col;
		if (isnumber(str.c_str())) {
			double d = _wtof(str.c_str());
			array[index].e = NULL;
			array[index].dVal = d;
		}
		else {
			CMExpression* e = new CMExpression(str.c_str());
			if (e->IsPolynomial()) arraystate |= containsPolynomials;
			if (e->Fail()) {
				SetState(vsFailed, 1);
				ReportError(XBadVardef, e->GetString());
			}
			array[index].e = e;
		}
		/*
		CMExpression* e = new CMExpression(str.c_str());
		if (e->IsPolynomial()) arraystate |= containsPolynomials;
		if (e->Fail()) {
  		 	SetState(vsFailed,1);
   			ReportError(XBadVardef,e->GetString());
	   }
	   array.AddAt((long)row*ncols+col,e);
	   */
	}
}

void CMVArray::read_body(wistream& s)
{
	CMString str,token,token2;
	int nr=0,nc=0;

	token=GetAssociation(L"rows");
	nr = _wtoi(token.c_str());
	token=GetAssociation(L"columns");
	nc = _wtoi(token.c_str());
	
	SetSize(nr,nc);

	nr=0;
	while(!s.eof()) {
		str.read_line(s);
		str=stripends(str);
		if (str.is_null() || str[0] == L'*')
			continue;
		if (str(0,wcslen(vardef_end)) == vardef_end)
			break;
		wchar_t* ptr = (wchar_t*)str.c_str();
		nc = 0;
		while (1) {
			while (*ptr <= L' ' && *ptr != 0) ptr++;
			if (*ptr == 0)
				break;
			int j=0;
			wchar_t* ptr2 = ptr;
			if (*ptr == L'{') {
				int bracketindex = 1;
				ptr++;
				ptr2++;
				for (;*ptr2!=0 && bracketindex;j++,ptr2++) {
					if (*ptr2==L'{') bracketindex++;
					else if (*ptr2==L'}') bracketindex--;
				}
				token = (CMString((const wchar_t*)ptr))(0, j - 1);
			}
			else {
				while (*ptr2 > L' ') {
					ptr2++;
					j++;
				}
				token = (CMString((const wchar_t*)ptr))(0, j);
			}
			ptr += j;
			Set(nr,nc++,token);
		}
		nr++;
	}
	if (nc != _ncols)
		ReportError(XIncorrectNumColumns);
	if (nr != _nrows)
		ReportError(XIncorrectNumRows);
}

void CMVArray::write_body(wostream& s)
{
	s << setiosflags(ios::left);
	for (int i=0;i<_nrows;i++) {
		for (int j=0;j<_ncols;j++) {
			//CMExpression* ex = GetExpression(i,j);
			DoubleOrExpression de = array[i*_ncols + j];
			if (de.e==NULL)
				s << setw(column_width) << de.dVal;
			else if (de.e->IsPolynomial())
				s << L'{' << *de.e << L"}  ";
			else
				s << setw(column_width) << *de.e;
		}
		s << ENDL;
	}
}

