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
#include "varray.h"

#include "cmlib.h"

#include <iomanip>
#include <stdlib.h>

CMVArrayIterator::CMVArrayIterator(CMVArray* v) :
CMVNameIterator(v),
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
	CMVArray* v = (CMVArray*)variable;

	if (!(v->arraystate & v->containsPolynomials))
		return 0;
	while (pos < v->array.Count()) {
		if (!iter) {
			while (pos<v->array.Count() && !v->array[pos]->ContainsVariables())
				pos++;
			if (pos >= v->array.Count())
				break;
			else
				iter = new CMExpressionIterator(*v->array[pos]);
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
CMVariable(aName),
array()
{
	SetState(vsDontMaintain,TRUE);
	SetSize(rows,cols);
}

CMVArray::~CMVArray()
{
	array.ResetAndDestroy(1);
}

CMVNameIterator* CMVArray::create_iterator()
{
	return (arraystate&containsPolynomials) ? new CMVArrayIterator(this) : 0;
}

void CMVArray::SetSize(int rows,int cols)
{
	array.ResetAndDestroy(1);
	nrows = rows;
	ncols = cols;
	long n = (long)nrows*ncols;
	for (long i=0;i<n;i++)
		array.AddAt(i,0);
}

double CMVArray::evaluate(CMTimeMachine* t,int index1,int index2)
{
	if (index1>0 && index1<=nrows && index2>0 && index2<=ncols)
		return array[(long)(index1-1)*ncols+index2-1]->Evaluate(t);
	return 0;
}

void CMVArray::update_variable_links()
{
	if (arraystate & containsPolynomials)
		for (long j=0;j<array.Count();j++)
			array[j]->UpdateVariableLinks();
}

void CMVArray::Set(int row,int col,const CMString& str)
{
	if (row>=0 && row<nrows && col>=0 && col<ncols) {
		CMExpression* e = new CMExpression(str.c_str());
		if (e->IsPolynomial()) arraystate |= containsPolynomials;
		if (e->Fail()) {
  		 	SetState(vsFailed,1);
   	   ReportError(XBadVardef,e->GetString());
	   }
		array.AddAt((long)row*ncols+col,e);
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
		if (nc != ncols)
			ReportError(XIncorrectNumColumns,token2,0);
		nr++;
	}
	if (nr != nrows)
		ReportError(XIncorrectNumColumns,token2,0);
}

void CMVArray::write_body(wostream& s)
{
	s << setiosflags(ios::left);
	for (int i=0;i<nrows;i++) {
		for (int j=0;j<ncols;j++) {
			CMExpression* ex = GetExpression(i,j);
			if (!ex)
				s << setw(column_width) << L"0";
			else if (ex->IsPolynomial())
				s << L'{' << *ex << L"}  ";
			else
				s << setw(column_width) << *ex;
		}
		s << ENDL;
	}
}

