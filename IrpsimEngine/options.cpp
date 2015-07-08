// options.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMOption implemets an option (string-value pair) used to control a
// simulation.
// CMOptions implements a collection of options
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "options.h"

#include "cmlib.h"
#include "token.h"
#include "notify.h"

#include <stdio.h>
#include <stdlib.h>
#include <iomanip>

//#include <fstream>
//static wofstream sdebug("debug_options.txt");

const wchar_t* allowedOptions[] = {
	L"simbegin",
	L"simend",
	L"siminterval",	L"simulationname",	L"costprecision",	L"numtrials",	L"outputfolder",	L"precision",	L"randomseed",	L"tracebegin",	L"traceend",	L"tracemode",	L"tracestart",	L"yearend",
	0
};

CMOption::CMOption(const CMString& n, const CMString& v, int app_id) : CMIrpObject(n, app_id) 
{ 
	BOOL bFound = FALSE;
	const wchar_t* name = n.c_str();
	for (unsigned i = 0; allowedOptions[i] != 0; i++)
	{
		if (!_wcsicmp(name, allowedOptions[i]))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
		CMNotifier::Notify(CMNotifier::WARNING, L"Option " + n + L" is not recognized or is deprecated");

	SetValue(v); 
}

void CMOption::SetValue(const CMString& v)
{
	value = stripends(v);
}

CMOptions::CMOptions() :
options(),
maxwidth(0)
{
	//SetDefaults();
}

CMOptions::CMOptions(const CMOptions& opt) :
options(),
maxwidth(0)
{
	for (unsigned i=0;i<opt.Count();i++) {
		int len = opt.At(i)->GetName().length();
		if (len>maxwidth) maxwidth=len;
			options.Add(new CMOption(*opt.At(i)));
	}
}

CMString CMOptions::GetOption(const CMString& name)
{
	CMString ret;
	CMString _name(name);
	_name.to_lower();
	CMOption* op = options.Find(_name);
	if (op) ret = op->GetValue();
	return ret;
}

const wchar_t* CMOptions::GetOptionString(const CMString& option)
{
	CMString op = GetOption(option);
	return op.c_str();
}

double CMOptions::GetOptionDouble(const CMString& option)
{
	const wchar_t* str = GetOption(option).c_str();
	return isnumber(str) ? _wtof(str) : 0;
}

short CMOptions::GetOptionInt(const CMString& option)
{
	const wchar_t* str = GetOption(option).c_str();
	return isnumber(str) ? _wtoi(str) : 0;
}

long CMOptions::GetOptionLong(const CMString& option)
{
	const wchar_t* str = GetOption(option).c_str();
	return isnumber(str) ? _wtol(str) : 0;
}

CMOption* CMOptions::SetOption(const CMString& line, int id)
{
	CMTokenizer next(line);
	CMString name  = next(L" \t\r\n");
	CMString value = next(L"\r\n");
	return SetOption(name, value, id);
}

CMOption* CMOptions::SetOption(const CMString& name, const CMString& value, int id)
{
	CMString nm = stripends(name);
	nm.to_lower();
	int len = nm.length();
	if (len>maxwidth) maxwidth=len;
	CMOption* op = options.Find(nm);
	if (op) 	op->SetValueAndAppId(value, id);
	else {
		op = new CMOption(CMOption(nm, value, id));
		options.Add(op);
	}
	return op;
}

CMOption* CMOptions::SetOption(const CMString& name, double option, int id)
{
	wchar_t buffer[64];
	swprintf_s(buffer, 64, L"%.12g",option);
	return SetOption(name,buffer,id);
}

CMOptions& CMOptions::operator = (const CMOptions& op)
{
	options.ResetAndDestroy(1);
	for (unsigned i=0;i<op.Count();i++)
		SetOption(*op.At(i));
	return *this;
}

wostream& operator << (wostream& s, CMOptions& o)
{
	s << setiosflags(ios::left) << L"#OPTIONS" << ENDL;
	o.options.Sort();
	s << setiosflags(ios::left);
	for (unsigned i=0;i<o.Count();i++)
		s << setw(o.maxwidth+2) << o.At(i)->GetName() << o.At(i)->GetValue() << ENDL;
	return s << L"#END";
}

wistream& operator >> (wistream& s, CMOptions& o)
{
	static wchar_t* header = L"#options";
	static wchar_t* footer = L"#end";
	o.options.Reset(1);
	CMString line;

	int begin = 0;

	while (!s.eof()) {
		line.read_line(s);
		line = stripends(line);

		if (line.is_null() || line[0] == L'*') {
			continue;
		}
		if (line(0,wcslen(header)) == header) {
			begin = 1;
			continue;
		}
		else if (line(0,wcslen(footer)) == footer) {
			if (!begin) { continue; }
			else       	{ break; }
		}
		else if (!begin) {
			continue;
		}
		/*
		while (line.length() && line[line.length()-1] == L'\\') {
			line = line(0,line.length()-1);
			CMString cont;
			cont.read_line(s);
			line += stripends(cont);
		}
		*/
		CMOption* poption = o.SetOption(line, o.GetApplicationId());
		if (poption) poption->SetApplicationIndex(o.GetApplicationIndex());
	}

	o.options.Sort();

	return s;
}
