// vmake.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Implements the "Make" method of CMVariable. Creates a variable from a
// persistent state in an input stream.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#include "StdAfx.h"
#include "vsingexp.h"
#include "vtable.h"
#include "varray.h"
#include "vperiod.h"
#include "vtimevar.h"

#include "cmlib.h"
#include "token.h"

#include <ctype.h>
#include <iostream>
using namespace std;

//#include <fstream>
//static wofstream sdebug(L"debug_vmake.txt");

CMString CMVariable::get_next_eval_type(wistream& s,CMString& aname)
{
	CMString str,token,token2,ret;
	long pos = (long)s.tellg();
	int begin = 0;

	while (!s.eof() && ret.is_null()) {
		str.read_line(s);
		if (str.is_null() || str[0] == L'*')
			continue;
		CMTokenizer next(str);
		token = next(L" \t\r\n");
		if (token == vardef_begin) {
			begin = 1;
			aname = next(L" \t\r\n");
		}
		else if (!begin)
			continue;
		else if (token(0, wcslen(vardef_end)) == vardef_end)  //                    token == vardef_end)
			ret = CMVariable::GetEvalType();
		else if (token[0] == L'#') {
			if (token(1,6) == L"period")
         		ret = CMVTimevar::GetEvalType();
			else if (token(1,6) == L"rowsel" || token(1,6) == L"colsel")
				ret = CMVTable::GetEvalType();
			else if (token(1,4) == L"rows" || token(1,6) == L"column")
				ret = CMVArray::GetEvalType();
		}
		else {
			if (!isnumber(token.c_str()))
				ret = CMVSingleExpression::GetEvalType();
			else {
				wchar_t* ptr = (wchar_t*)str.c_str();
				int pastnumber=0;
				while ((isnumber(*ptr)&&!pastnumber) || *ptr==L' ' || *ptr==L'\t') {
					if (*ptr==L' ' || *ptr==L'\t')
						pastnumber=1;
					ptr++;
				}
				if (isalgop(ptr))
					ret = CMVSingleExpression::GetEvalType();
				else if ((token=next(L" \t\r\n")).is_null())
					ret = CMVSingleExpression::GetEvalType();
				else
					ret = CMVPeriod::GetEvalType();
			}
		}
	}
	s.clear();
	s.seekg(pos,ios::beg);
	return ret;
}

CMVariable* CMVariable::Make(wistream& s)
{
	CMVariable* v = 0;
	CMString aname;

	CMString eval_type = get_next_eval_type(s,aname);

	if (eval_type == CMVSingleExpression::GetEvalType())
		v = new CMVSingleExpression(aname);
	else if (eval_type == CMVTable::GetEvalType())
		v = new CMVTable(aname);
	else if (eval_type == CMVArray::GetEvalType())
		v = new CMVArray(aname);
	else if (eval_type == CMVPeriod::GetEvalType())
		v = new CMVPeriod(aname);
	else if (eval_type == CMVTimevar::GetEvalType())
		v = new CMVTimevar(aname);
	else if (eval_type == CMVariable::GetEvalType())
		v = new CMVariable(aname);
	if (v) s >> *v;

	return v;
}


















