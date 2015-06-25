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

#include <stdio.h>
#include <stdlib.h>
#include <iomanip>

//#include <fstream>
//static wofstream sdebug("debug_options.txt");

CMOptions::_def CMOptions::defaults[] = {
{L"simbegin",L"2016"},
{L"simend",L"2025"},
{L"siminterval",L"monthly"},
{L"numtrials",L"1000"},
{0,0}
};

/*
traceBegin               1922
traceEnd                 1930
traceStart               1922
traceMode                Dynamic
focusPeriod              201907
yearEnd                  Jun
updateInterval           1
precision                1
volumeUnits              AFX1000
costUnits                DX1000
supplyRule               Rank
transferRule             Rank
storageRule              MaxTake
defaultDivisible         notDivisible
defaultTake              partialTake
defaultDraw              dontDraw
costFocus                <default>
supplyCost               Net
grossReliability         (10%,1%)  (5%,5.5%)  (0%,10%)
netReliability           (10%,1%)  (5%,5.5%)  (0%,10%)
discountRates            (1995,6%)
defaultDiscount          5
valuationYear            1994
valuationMode            Begin
graphType                TimeSeries
graphPrimary             _supply
graphSecondary           _demand 
graphCostFormat          SingleYear  Nominal  
graphShortageFormat      Percent
graphBins                20
graphPeriod              1995  2020
summaryFile              Test.sum
summaryOpenMode          Replace
outcomeFile              Test.out
outcomeOpenMode          Replace
outputVariables	   aggregate user
outputCostFormat         SingleYear  Nominal
outputSortBy             None
outputSortOrder          Ascending
outputFormat             ByTrial
ouputPercentile          100
outputHeader             No
outputPeriod             0
outputResolution         monthly
outputDelimiter          S1
outputReliability        Yes
saveIncrementSize        1
simulationFile           sim.sim
autosummary	  yes
pbTier			3
pbSupplyVar		swp_3
pbContractorConveyanceVar  contractor_conveyance
pbContractorAllocationVar 	contractor_allocation
pbMWDAllocationVar	mwd_allocation
*/

/*
CMOption::CMOption(const CMString& n,const CMString& v , int app_id) :
name(n)
{
  name.to_lower();
   name = stripends(name);
	if (name.length() && name[0]==L'#')
		name = name.substr(1,name.length()-1);
	SetValue(v);
}
*/

void CMOption::SetValue(const CMString& v)
{
	value = stripends(v);
}

/*
wostream& CMOption::WriteBinary(wostream& s)
{
	writestringbinary(name,s);
	writestringbinary(value,s);
	return s;
}

wistream& CMOption::ReadBinary(wistream& s)
{
	name = readstringbinary(s);
	value = readstringbinary(s);
	return s;
}

short CMOption::BinarySize()
{
	return (stringbinarylength(name) + stringbinarylength(value));
}
*/

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

/*
void CMOptions::SetDefaults()
{
	options.ResetAndDestroy(1);
	maxwidth=0;
	for (int i=0;defaults[i].name;i++) {
		int len = wcslen(defaults[i].name);
		if (len>maxwidth) maxwidth = len;
		options.Add(new CMOption(defaults[i].name,defaults[i].value,-1));
	}
}
*/
CMString CMOptions::GetOption(const CMString& option)
{
	CMString ret;
	CMOption* op = options.Find(option);
	if (op)
		ret = op->GetValue();
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
		while (line.length() && line[line.length()-1] == L'\\') {
			line = line(0,line.length()-1);
			CMString cont;
			cont.read_line(s);
			line += stripends(cont);
		}
		CMOption* poption = o.SetOption(line, o.GetApplicationId());
		if (poption) poption->SetApplicationIndex(o.GetApplicationIndex());
	}

	o.options.Sort();

	return s;
}

/*
wostream& CMOptions::WriteBinary(wostream& s)
{
	unsigned short len = options.Count();
	s.write((const wchar_t*)&len,sizeof(len));
	for (unsigned short i=0;i<len;i++)
		options[i]->WriteBinary(s);
	return s;
}

wistream& CMOptions::ReadBinary(wistream& s)
{
	unsigned short len;
	s.read((wchar_t*)&len,sizeof(len));
	maxwidth = 0;
	for (unsigned i=0;i<len;i++) {
		CMOption* o = new CMOption;
		o->ReadBinary(s);
		unsigned short len = o->GetName().length();
		if (len>maxwidth) maxwidth=len;
		options.Add(o);
	}
	options.Sort();
	return s;
}

long CMOptions::BinarySize()
{
	long ret = 0;
	for (unsigned i=0;i<options.Count();i++)
		ret += options[i]->BinarySize();
	return ret;
}

*/