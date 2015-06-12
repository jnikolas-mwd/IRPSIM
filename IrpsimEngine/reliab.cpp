// reliab.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMReliability implements IRPSIM's system reliability measures.
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
#include "reliab.h"
#include "variable.h"
#include "timemach.h"

#include "token.h"
#include "cmlib.h"

#include <stdio.h>
#include <stdlib.h>

//#include <fstream.h>
//static ofstream sdebug("reliab.deb");

CMReliabilityTarget::CMReliabilityTarget() :
state(0),
shortagevar(0),
percentagevar(0),
shortage_target(0),
percent_target(0),
shortagevarname(),
percentagevarname()
{
}

CMReliabilityTarget::CMReliabilityTarget(const CMString& def)
{
	Set(def);
}

void CMReliabilityTarget::Set(const CMString& def)
{
	static const wchar_t* delims = L" (),%\t\r\n";

	state = 0;
	shortagevar = percentagevar = 0;
	shortage_target = percent_target = 0;
	shortagevarname = percentagevarname = L"";

	CMTokenizer next(def);
	CMString token = next(delims);
	if (token[0] == L'-') {
		state |= changeSignShortage;
		token = token.substr(1,token.length()-1);
	}

	shortagevarname = token;
	token = next(delims);

	if (!isnumber(token.c_str())) {
		state |= isPercent;
		if (token[0] == L'-') {
			state |= changeSignDenom;
			token = token.substr(1,token.length()-1);
		}
		percentagevarname = token;
		token = next(delims);
	}

	shortage_target = _wtof(token.c_str());

	token = next(delims);

	percent_target = _wtof(token.c_str());

	shortagevar = CMVariable::Find(shortagevarname);

	if (state & isPercent) percentagevar = CMVariable::Find(percentagevarname);

	if (shortagevar)
		shortagevar->SetState(CMVariable::vsSelected|CMVariable::vsSaveOutcomes,TRUE);
	if (percentagevar)
		percentagevar->SetState(CMVariable::vsSelected|CMVariable::vsSaveOutcomes,TRUE);
}

int CMReliabilityTarget::SameCurveAs(const CMReliabilityTarget& t)
{
	return (shortagevarname==t.shortagevarname &&
			  percentagevarname==t.percentagevarname &&
			  state==t.state);
}

CMString CMReliabilityTarget::GetString()
{
	wchar_t buffer[256];
	const wchar_t* signstr1 = (state&changeSignShortage) ? L"-" : L"";
	const wchar_t* signstr2 = (state&changeSignDenom) ? L"-" : L"";

	if (state & isPercent)
		swprintf_s(buffer, 256, L"(%s%s %s%s %g%% %g%%)",
			signstr1,shortagevarname.c_str(),signstr2,percentagevarname.c_str(),shortage_target,percent_target);
	else
		swprintf_s(buffer, 256, L"(%s%s %g %g%%)",signstr1,shortagevarname.c_str(),shortage_target,percent_target);

	return CMString(buffer);
}

int CMReliabilityTarget::Process(CMTimeMachine* t)
{
	int ret = 0;

	double shortage = shortagevar ? shortagevar->GetValue(t) : 0;
	if (state & changeSignShortage) shortage = -shortage;

	if (state & isPercent) {
		double denom = percentagevar ? percentagevar->GetValue(t) : 0;
		if (state & changeSignDenom) denom = -denom;
		if (denom <= 0)
			ret = 0;
		else
			ret = (shortage/denom <= shortage_target/100) ? 1 : 0;
	}
	else
			ret = shortage <= shortage_target ? 1 : 0;

	return ret;
}

wostream& CMReliabilityTarget::WriteBinary(wostream& s)
{
	s.write((const wchar_t*)&state,sizeof(state));
	s.write((const wchar_t*)&shortage_target, sizeof(shortage_target));
	s.write((const wchar_t*)&percent_target, sizeof(percent_target));
	writestringbinary(shortagevarname,s);
	writestringbinary(percentagevarname,s);
	return s;
}

wistream& CMReliabilityTarget::ReadBinary(wistream& s)
{
	s.read((wchar_t*)&state, sizeof(state));
	s.read((wchar_t*)&shortage_target, sizeof(shortage_target));
	s.read((wchar_t*)&percent_target, sizeof(percent_target));
	readstringbinary(shortagevarname,s);
	readstringbinary(percentagevarname,s);
	shortagevar = CMVariable::Find(shortagevarname);
	percentagevar = CMVariable::Find(percentagevarname);
	return s;
}

int CMReliabilityTarget::BinarySize()
{
	return (sizeof(state) + sizeof(shortage_target) + sizeof(percent_target) +
			  stringbinarylength(shortagevarname) + stringbinarylength(percentagevarname));
}

/*
CMReliabilityTarget::CMReliabilityTarget(const CMString& def)
{
	CMTokenizer next(def);
	CMString shortagetoken = next("(,");
	CMString pcttoken = next(",)");
	Set(shortagetoken,pcttoken);
}

void CMReliabilityTarget::Set(const CMString& stoken,const CMString& ptoken)
{
	unsigned int index;
	CMString s(stoken);
   CMString p(ptoken);

	CMString pctstring("%");

	state = 0;
	shortage = targetpct = 0;

	if (s.is_null() || p.is_null()) {
		state |= rsFail;
		return;
	}

	if ((index=s.find(pctstring)) != CM_NPOS) {
		state |= rsPct;
		s = s.substr(0,index);
	}

	shortage = atof(s.c_str());

	if ((index=p.find(pctstring)) != CM_NPOS)
		p = p.substr(0,index);

	targetpct = atof(p.c_str());
}

CMString CMReliabilityTarget::GetString()
{
	char buffer[64];
	sprintf(buffer,"(%.1f%s,%.1f%)",shortage,(state&rsPct)?"%":"",targetpct);
	return CMString(buffer);
}

*/

CMReliability::CMReliability() :
incunits(CM_MONTH),
inclength(1),
ntargets(0),
count(0),
targets(0),
hits(0),
state(0)
{
}

CMReliability::CMReliability(const CMTimeMachine& t,const CMString& def) :
beg(t[0].Begin()),
end(t[0].End()),
incunits(t.IncUnits()),
inclength(t.IncLength()),
ntargets(0),
count(0),
targets(),
hits(0),
state(0)
{
	const wchar_t* delims = L")>}";

	CMTokenizer next(def);
	CMString token;

	while (!(token = next(delims)).is_null())
		targets.Add(new CMReliabilityTarget(token));
	ntargets = targets.Count();

	periodlength = CMTime::Diff(end,beg,incunits,inclength) + 1;
	if ( end<beg || inclength<=0)
		state |= rsFail;
	else if (ntargets) {
		hits = new CMVBigArray<long>(periodlength*ntargets);
		Reset();
	}
}

CMReliability::~CMReliability()
{
	targets.ResetAndDestroy();
	if (hits) delete hits;
}

long CMReliability::index(const CMTime& t,unsigned aTarget)
{
	if (t<beg || t>end || aTarget>=ntargets)
		return -1;
	return CMTime::Diff(t,beg,incunits,inclength)*ntargets+aTarget;
}

long CMReliability::GetPeriod(CMTime& b,CMTime& e,short& units,short& length) const
{
	b = beg;
	e = end;
	units = incunits;
	length = inclength;
	return periodlength;
}

void CMReliability::Reset()
{
	count=0;
	for (long i=0;i<periodlength*ntargets;i++)
		hits->AddAt(i,0);
}

void CMReliability::Process(CMTimeMachine* t)
{
	if (!ntargets) return;
	long n = index(t->Now(),0);
	for (unsigned i=0;i<ntargets;i++,count++)
		hits->At(n+i) += targets.At(i)->Process(t);
}

long CMReliability::Success(const CMTime& t,unsigned aTarget)
{
	long i = index(t,aTarget);
	return (i<0) ? 0 : hits->At(i);
}

double CMReliability::SuccessPct(const CMTime& t,unsigned aTarget)
{
	long i = index(t,aTarget);
	long trials = Trials();
	return (i>=0 && trials) ? 100*(double)hits->At(i)/trials : 0;
}

wostream& CMReliability::WriteBinary(wostream& s)
{
	unsigned short i;
	beg.Write(s,1);
	end.Write(s,1);
	s.write((const wchar_t*)&count, sizeof(count));
	s.write((const wchar_t*)&ntargets, sizeof(ntargets));
	s.write((const wchar_t*)&incunits, sizeof(incunits));
	s.write((const wchar_t*)&inclength, sizeof(inclength));
	s.write((const wchar_t*)&state, sizeof(state));
	for (i=0;i<ntargets;i++)
		targets.At(i)->WriteBinary(s);
	for (i=0;i<periodlength*ntargets;i++)
		s.write((const wchar_t*)&hits->At(i), sizeof(long));
	return s;
}

wistream& CMReliability::ReadBinary(wistream& s)
{
	unsigned short i;
	targets.ResetAndDestroy();
	if (hits) delete hits;
	beg.Read(s,1);
	end.Read(s,1);
	s.read((wchar_t*)&count, sizeof(count));
	s.read((wchar_t*)&ntargets, sizeof(ntargets));
	s.read((wchar_t*)&incunits, sizeof(incunits));
	s.read((wchar_t*)&inclength, sizeof(inclength));
	s.read((wchar_t*)&state, sizeof(state));
	periodlength = CMTime::Diff(end,beg,incunits,inclength) + 1;
	if (ntargets) hits = new CMVBigArray<long>(periodlength*ntargets);
	for (i=0;i<ntargets;i++) {
		CMReliabilityTarget* t = new CMReliabilityTarget();
		t->ReadBinary(s);
		targets.Add(t);
	}
	for (i=0;i<periodlength*ntargets;i++) {
		long val;
		s.read((wchar_t*)&val, sizeof(long));
		hits->AddAt(i,val);
	}
	return s;
}

long CMReliability::BinarySize()
{
	long ret = 2*CMTime::BinarySize() + 4*sizeof(short) + sizeof(long);
	for (unsigned i=0;i<ntargets;i++)
		ret += targets[i]->BinarySize();
	return ret + periodlength*ntargets*sizeof(long);
}

/*
CMReliability& CMReliability::operator = (const CMReliability& r)
{
	delete targets;
	delete hits;

	beg = r.beg;
	end = r.end;
	ntargets = r.ntargets;
	incunits = r.incunits;
	inclength = r.inclength;
	state = r.state;
	count = r.count;
	periodlength = CMTime::Diff(end,beg,incunits,inclength);
	targets = new CMVSmallArray<CMReliabilityTarget>(ntargets);
	hits = new CMVBigArray<long>(periodlength*ntargets);
	for (unsigned i=0;i<ntargets;i++) {
		targets->AddAt(i,r.targets->At(i));
		for (unsigned j=0;j<periodlength;j++)
			hits->AddAt(j*ntargets+i,r.hits->At(j*ntargets+i));
	}
	return *this;
}

*/
