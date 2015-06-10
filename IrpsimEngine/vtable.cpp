// vtable.cpp : implementation file
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
#include "vtable.h"
#include "defines.h"

#include "token.h"

#include <iomanip>

//#include <fstream.h>
//static ofstream vdebug("vtable.deb");

CMVTableIterator::CMVTableIterator(CMVTable* v) :
CMIrpObjectIterator(v),
vIterator(0),
eIterator(0),
state(iterRowSel),
pos(0)
{
}

CMVTableIterator::~CMVTableIterator()
{
	if (vIterator) delete vIterator;
	if (eIterator) delete eIterator;
}

const wchar_t* CMVTableIterator::get_next()
{
	const wchar_t* ret;
	CMVTable* v = (CMVTable*)obj;
	CMVariable* var;

//   vdebug << "got to 1 : " << v->GetName() << ENDL;

	if (state == iterRowSel) {
		if (pos < v->rowselectornames.Count()) 
			return v->rowselectornames[pos++].c_str();
		state = iterColSel;
		pos=0;
	}

//	vdebug << "  got to 2" << ENDL;

	if (state == iterColSel) {
		if (pos < v->colselectornames.Count())
			return v->colselectornames[pos++].c_str();
		state = iterRowVars;
		pos=0;
	}

//	vdebug << "  got to 3" << ENDL;

	if (state == iterRowVars) {
		while (pos < v->rowselectornames.Count()) {
			if (vIterator) {
				if ((ret = vIterator->GetNext())!=0)
					return ret;
				pos++;
				vIterator = 0;
			}
			var=0;
			while (pos<v->rowselectornames.Count() && (var=CMVariable::Find(v->rowselectornames[pos]))==0)
				pos++;
			if (var) {
				if ((vIterator = var->CreateIterator())==0)
					pos++;
			}
		}
		state = iterColVars;
		vIterator=0;
		pos=0;
	}

//	vdebug << "  got to 4" << ENDL;

	if (state == iterColVars) {
		while (pos < v->colselectornames.Count()) {
			if (vIterator) {
				if ((ret = vIterator->GetNext())!=0)
					return ret;
				pos++;
				vIterator = 0;
			}
			var=0;
			while (pos<v->colselectornames.Count() && (var=CMVariable::Find(v->colselectornames[pos]))==0)
				pos++;
			if (var) {
				if ((vIterator = var->CreateIterator())==0)
					pos++;
			}
		}
		state = iterArray;
		vIterator=0;
		pos=0;
	}

//	vdebug << "  got to 5" << ENDL;

	if (state == iterArray) {
		if (!(v->tablestate & v->containsPolynomials))
			return 0;
		while (pos < v->array.Count()) {
			if (!eIterator) {
				while (pos<v->array.Count() && !v->array[pos]->ContainsVariables())
					pos++;
				if (pos >= v->array.Count())
					break;
				else
					eIterator = new CMExpressionIterator(*v->array[pos]);
			}
			if (eIterator && (ret=(*eIterator)())!=0)
				return ret;
			pos++;
			if (eIterator) delete eIterator;
			eIterator = 0;
		}
	}

//	vdebug << "  got to 6" << ENDL;

	return 0;
}

CMVTable::CMVTable(const CMString& aName) :
CMVariable(aName),
array(0,32),
rowselectornames(0,1),
colselectornames(0,1),
rowselector(),
colselector(),
nrows(0),
ncols(0),
tablestate(0)
{
}

CMVTable::~CMVTable()
{
	array.ResetAndDestroy(1);
}

void CMVTable::update_variable_links()
{
	CMVariable* v;
	int err = 0;
   int i;
   long j;

	for (i=0;i<rowselector.GetNumVariables();i++) {
		if ((v = Find(rowselectornames[i]))==0) err=1;
		rowselector.SetVariable(i,v);
	}

	for (i=0;i<colselector.GetNumVariables();i++) {
		if ((v = Find(colselectornames[i]))==0) err=1;
		colselector.SetVariable(i,v);
	}

	if (tablestate & containsPolynomials)
		for (j=0;j<array.Count();j++)
			array[j]->UpdateVariableLinks();

	if (err)
		ReportError(XNoAssociatedVariable);
}

void CMVTable::SetRowSelector(int n,const CMString& nm)
{
	rowselectornames.AddAt(n,nm);
	rowselector.SetNumVariables(rowselectornames.Count());
}

void CMVTable::SetColSelector(int n,const CMString& nm)
{
	colselectornames.AddAt(n,nm);
	colselector.SetNumVariables(colselectornames.Count());
}

void CMVTable::SetRowValue(int row,int selno,double value)
{
	rowselector.Add(row,selno,value);
}

void CMVTable::SetColValue(int col,int selno,double value)
{
	colselector.Add(col,selno,value);
}

void CMVTable::SetExpression(int row,int col,const CMString& str)
{
	if (col>=0 && col<ncols) {
		if (row>=nrows) nrows = row+1;
		CMExpression* e = new CMExpression(str.c_str());
		if (e->IsPolynomial()) tablestate |= containsPolynomials;
		if (e->Fail()) {
   		SetState(vsFailed,1);
      	ReportError(XBadVardef,e->GetString());
	   }
		array.AddAt((long)row*ncols+col,e);
	}
}

double CMVTable::evaluate(CMTimeMachine* t,int,int)
{
	long row = rowselector.GetIndex(t);
	long col = colselector.GetIndex(t);

	if (row>=0 && row<nrows && col>=0 && col<ncols)
		return array[(long)row*ncols+col]->Evaluate(t);

	return 0;
}

void CMVTable::read_body(wistream& s)
{
	CMString str,token,token2;
	int i=0,nrsel=0,ncsel=0,processed=0,nr=0,nc;

	if ((token=GetAssociation(L"rowselector")).length()) {
		CMTokenizer next(token);
		while (!(token2=next(L" \t\r\n")).is_null())
			SetRowSelector(nrsel++,token2);
	}

	if ((token=GetAssociation(L"colselector")).length()) {
		CMTokenizer next(token);
		while (!(token2=next(L" \t\r\n")).is_null())
			SetColSelector(ncsel++,token2);
	}

	array.ResetAndDestroy(1);
	if (!ncsel) ncols = 1;

	while(!s.eof()) {
		str.read_line(s);
		if (str.is_null() || str[0] == L'*')
			continue;
		if (str(0,wcslen(vardef_end)) == vardef_end)
			break;
		CMTokenizer next(str);
		if (processed < ncsel) {
			ncols = 0;
			while (!(token = next()).is_null()) {
				double val = CMDefinitions::GetDefinition(token);
				SetColValue(ncols++,processed,val);
//				SetColValue(ncols++,processed,atof(token.c_str()));
			}
			processed++;
			continue;
		}
		for (i=0;i<nrsel;i++) {
			token = next();
			double val = CMDefinitions::GetDefinition(token);
			SetRowValue(nr,i,val);
//			SetRowValue(nr,i,atof(token.c_str()));
		}
		nc=0;
		token = next(L"\r\n");
		token2 = token;
		wchar_t* ptr = (wchar_t*)token2.c_str();
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
			SetExpression(nr,nc++,token);
		}
		if (nc != ncols)
			ReportError(XIncorrectNumColumns,token2,0);
		nr++;
	}
}

void CMVTable::write_body(wostream& s)
{
	int i,j;
	s << setiosflags(ios::left);
	for (i=0;i<colselector.GetNumVariables();i++) {
		for (j=0;j<rowselector.GetNumVariables();j++)
			s << setw(column_width) << L"";
		for (j=0;j<ncols;j++)
			s << setw(column_width) << colselector.GetValue(j,i);
		s << ENDL;
	}

	for (i=0;i<nrows;i++) {
		for (j=0;j<rowselector.GetNumVariables();j++)
			s << setw(column_width) << rowselector.GetValue(i,j);
		for (j=0;j<ncols;j++) {
			CMExpression& ex = GetExpression(i,j);
			if (ex.IsPolynomial())	s << L'{' << ex << L"}  ";
			else                  	s << setw(column_width) << ex;
		}
		s << ENDL;
	}
}

