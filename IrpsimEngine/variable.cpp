// variable.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVariable is the virtual base class for simulation variables.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#include "variable.h"
#include "varcol.h"
#include "vartypes.h"
#include "notify.h"

#include "token.h"
#include "cmlib.h"

#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <iomanip>
//#include <fstream>
//static wofstream sdebug("debug.variable.txt", ios::binary);

using namespace std;

const unsigned long CMVariable::vsFailed 				= 0x00000001L;
const unsigned long CMVariable::vsStochastic 		= 0x00000002L;
const unsigned long CMVariable::vsSystem 				= 0x00000004L;
const unsigned long CMVariable::vsDontDestroy		= 0x00000008L;
const unsigned long CMVariable::vsAggregate			= 0x00000010L;
const unsigned long CMVariable::vsRegional			= 0x00000020L;
const unsigned long CMVariable::vsEvaluating 		= 0x00000040L;
const unsigned long CMVariable::vsIterating 			= 0x00000080L;
const unsigned long CMVariable::vsCircularity 		= 0x00000100L;
const unsigned long CMVariable::vsDontMaintain 		= 0x00000200L;
const unsigned long CMVariable::vsAlwaysEvaluate   = 0x00000400L;
const unsigned long CMVariable::vsLinksUpdated 		= 0x00000800L;
const unsigned long CMVariable::vsSelected 			= 0x00001000L;
const unsigned long CMVariable::vsLinked 				= 0x00002000L;
const unsigned long CMVariable::vsSaveSummary 		= 0x00004000L;
const unsigned long CMVariable::vsSaveOutcomes 		= 0x00008000L;
const unsigned long CMVariable::vsOutput		 		= 0x00010000L;
const unsigned long CMVariable::vsGraph		 		= 0x00020000L;
const unsigned long CMVariable::vsSum					= 0x00040000L;
const unsigned long CMVariable::vsInteger				= 0x00080000L;
const unsigned long CMVariable::vsMoney	  			= 0x00100000L;
const unsigned long CMVariable::vsNoUnits  			= 0x00200000L;
const unsigned long CMVariable::vsResetRequired 	= 0x00400000L;
const unsigned long CMVariable::vsCarryForward 		= 0x00800000L;
const unsigned long CMVariable::vsInUse				= 0x01000000L;
const unsigned long CMVariable::vsAutoCreated 		= 0x02000000L;
const unsigned long CMVariable::vsMonitor		 		= 0x04000000L;
const unsigned long CMVariable::vsAccumulate	 		= 0x08000000L;
const unsigned long CMVariable::vsDontEdit	 		= 0x10000000L;
const unsigned long CMVariable::vsFlag			 		= 0x20000000L;

CMVNameIterator::CMVNameIterator(CMVariable* v) :
variable(v),
varray(),
vpos(-2)
{
}

CMVNameIterator::~CMVNameIterator()
{
	varray.ResetAndDestroy(1);
}

int CMVNameIterator::reset()
{
	if (vpos>=-1) {
		vpos=0;
		return 1;
	}
	return 0;
}

const wchar_t* CMVNameIterator::GetNext()
{
	const wchar_t* ret = 0;
	if (!(variable->GetState() & CMVariable::vsIterating)) {
		variable->SetState(CMVariable::vsIterating,TRUE);
		if (vpos==-2) {
			if ((ret=get_next())!=0)
				varray.Insert(ret);
			else
				vpos=-1;
		}
		else if (vpos>-1)
			ret = (vpos<varray.Count()) ? varray[vpos++]->c_str() : 0;
		variable->SetState(CMVariable::vsIterating,FALSE);
	}
	return ret;
}

// Static variables

wchar_t* CMVariable::errorstrings[] =
{
L"variable already exists",
L"illegal variable name",
L"circularity encountered",
L"misplaced #ENDVAR",
L"incorrect number of row entries",
L"incorrect number of column entries",
L"bad variable definition",
L"illegal time lag",
L"time out of range",
L"can't find one or more associated variables"
};

const wchar_t* 	CMVariable::vardef_begin 		= L"#VARDEF";
const wchar_t* 	CMVariable::vardef_end   		= L"#ENDVAR";
int 				CMVariable::output_precision 	= 10;
int 				CMVariable::column_width 		= 15;
int				CMVariable::sort_method 		= CMVariable::byType;
int				CMVariable::static_state 		= 0;
CMVariableCollection* CMVariable::variables = 0;

const wchar_t* CMVariable::IsA() { return L"CMVariable"; }

CMVariable::CMVariable(const CMString& aName,ULONG astate,int id) :
CMIrpObject(aName,id),
//name(aName),
iterator(0),
type(NOTYPE),
region(-1),
state(astate),
//app_id(id),
errorcode(0),
types(0,1),
realized(0),
associations(0,16)
{
	if (Find(aName)) {
	  	ReportError(XAlreadyExists);
   	SetState(vsFailed,TRUE);
   }
   else {
		for (int i=0;i<(int)aName.length();i++) {
			if (!(isalnum(aName[i]) || aName[i]==L'_' || aName[i]==L'@' || aName[i]==L'.')) {
			  	ReportError(XIllegalName);
   			SetState(vsFailed,TRUE);
            break;
         }
      }
   }
}

CMVariable::~CMVariable()
{
	types.ResetAndDestroy(1);
	associations.ResetAndDestroy(1);
	if (iterator) delete iterator;
	if (realized) delete realized;
}

void CMVariable::ReportError(int c,CMTime* t)
{
	static wchar_t* separator = L" : ";
	CMString ret(errorstrings[c]);
	ret += separator;
	ret += GetName();
	if (t) {
		ret += separator;
		ret += t->GetString();
	}
	CMNotifier::Notify(CMNotifier::ERROR, ret);
}

void CMVariable::ReportError(int c,const CMString& str,CMTime* t)
{
	static wchar_t* separator = L" : ";

	CMString ret(errorstrings[c]);
	ret += separator;
	ret += GetName();
	if (t) {
		ret += separator;
		ret += t->GetString();
	}
	if (str.length()) {
		ret += separator;
		ret += str;
	}
	CMNotifier::Notify(CMNotifier::ERROR, ret);
}

void CMVariable::reset_trial(CMTimeMachine* t)
{
	if (realized)
		realized->Reset();
	else if (!(state&vsDontMaintain))
		set_value_queue((state&vsCarryForward) ? 2 : 1);
	reset(t);
	state &= ~vsResetRequired;
}

ULONG CMVariable::SetState(ULONG aState,BOOL action)
{
	if (action == TRUE)	state |= aState;
	else               	state &= ~aState;
	return state;
}

BOOL CMVariable::ToggleState(ULONG aState)
{
	state ^= aState;
	return (state&aState) ? TRUE : FALSE;
}

/*
void CMVariable::DestroyAllVariables()
{
	static_state |= ssDestroying;
	variables.Reset(1);
	regions.Reset(1);
	foci.Reset(1);
	static_state &= ~ssDestroying;
}

void CMVariable::DestroyVariables(ULONG stateflag)
{
	CMVariableIterator iter;
	static_state |= ssDestroying;
	CMVariable* v;
	CMVSmallArray<CMString> varnames;
	while ((v=iter())!=0)
		if ( (v->GetState()&stateflag) == stateflag)
			varnames.Add(v->GetName());
	for (unsigned i=0;i<varnames.Count();i++)
		variables.Detach(varnames[i],1);
	static_state &= ~ssDestroying;
}

long CMVariable::VariableCount()
{
	return variables.Count();
}

*/

void CMVariable::UpdateVariableLinks()
{
	if (state&vsLinksUpdated)
		return;
	state|=vsLinksUpdated;
	if (iterator)
		delete iterator;
	iterator = 0;
	update_variable_links();
}

void CMVariable::UpdateLinkStatus()
{
	if (state&vsLinked)
		return;

   SetState(vsLinked,1);
	CMVariable* vfound;
	for (unsigned short j=0;j<associations.Count();j++) {
		if ((vfound=Find(associations.GetValue(j)))!=0)
         vfound->UpdateLinkStatus();
   }

	CMVNameIterator* ni = CreateIterator();
	if (ni) {
		const wchar_t* vname;
		while ((vname=ni->GetNext())!=0)
			if ((vfound=Find(vname))!=0)
				vfound->UpdateLinkStatus();
	}
}

void CMVariable::ResetTrial()
{
	if (variables) variables->SetStateAll(vsResetRequired,TRUE);
}

CMVNameIterator* CMVariable::create_iterator()
{
	return 0;
//	return new CMVNameIterator(this);
}

void CMVariable::SetType(const CMString& aName)
{
	int aType;
	if (aName == L"carryforward")
		SetState(vsCarryForward,TRUE);
   else if (aName == L"accumulator")
		SetState(vsCarryForward|vsAccumulate,TRUE);
	else if (aName == L"sum")
   	SetState(vsSum,TRUE);
	else if (aName == L"cost" || aName == L"money")
   	SetState(vsMoney,TRUE);
	else if (aName == L"nounits")
   	SetState(vsNoUnits,TRUE);
	else if (aName == L"alwaysevaluate")
   	SetState(vsAlwaysEvaluate,TRUE);
	if ((aType = CMVariableTypes::VarIntFromString(aName.c_str(),1)) >= 0) {
   	type = aType;
		if (CMVariableTypes::IsVarSum(type)) SetState(vsSum,TRUE);
	}
	else
		types.Add(new CMString(aName));
}

void CMVariable::SetType(int val)
{
//	if (val<0)
		type=val;
}

int CMVariable::IsType(const CMString& aName)
{
	int ret=0;
	if (type!=NOTYPE && type>=0 && (type%100)==0)
		ret = (aName == CMVariableTypes::VarStringFromInt(type,1)) ? 1 : 0;
	if (!ret)
		ret = types.Contains(aName);
	return ret;
}

CMString CMVariable::GetSpecialType()
{
	const wchar_t* stype = CMVariableTypes::VarStringFromInt(type, 1);
	if (stype) return CMString(stype);
   return CMString();
}

void CMVariable::AddAssociation(const CMString& aName,const CMString& val)
{
	associations.AddAssociation(aName,val);
}

CMString CMVariable::GetAssociation(const CMString& aName)
{
	return associations.GetValue(aName);
}

int CMVariable::GetAssociation(int n,CMString& s1,CMString& s2)
{
	return associations.GetAssociation(n,s1,s2);
}

CMVNameIterator* CMVariable::CreateIterator()
{
	if (GetState() & vsIterating)
		return 0;

	if (iterator) {
		if (!iterator->reset()) {
			delete iterator;
			iterator=0;
		}
	}

	if (!iterator)
		iterator = create_iterator();

	return iterator;
}

CMVariable* CMVariable::Find(const CMString& aName)
{
	return variables ? variables->Find(aName) : 0;
}

double CMVariable::GetValue(CMTimeMachine* t,int which,int index1,int index2,int force_evaluation)
{
	double ret = 0;

	int atbeginning = (t->Now() == t->At(0).Begin());
	int iscurrent = (t->Now() == t->At(0).Current());

	// Reset the variable if it hasn't been reset

	if (state&vsResetRequired)
		reset_trial(t);

	int contains = realized ? realized->ContainsAt(*t,t->Now(),t->IncUnits(),t->IncLength()) : 0;

	// if this is a carryforward variable AND we are not at the beginning
	// of the period AND we have not already retrieved the value,
   // get the value from the previous period & add it to the queue

	if ((state&vsCarryForward) && realized && !atbeginning && !contains) {
		ret = realized->At(t->Now().Plus(-t->IncLength(),t->IncUnits()),t->IncUnits(),t->IncLength(),1);
		realized->AddAt(t->Now(),t->Now(),t->IncUnits(),t->IncLength(),ret);
		if (!(state&vsAccumulate))
			contains = realized->ContainsAt(*t,t->Now(),t->IncUnits(),t->IncLength());
	}

	if (!realized || !contains || (((state&vsAlwaysEvaluate)||force_evaluation)&&iscurrent)) {
		if ( (state & vsEvaluating) ) {
			// If we are trying to access the same variable, same timestep
   			// OR if there is no queue in which to store past values -> ERROR
			if (iscurrent || !realized)
				SetState(vsCircularity,TRUE);
			return 0;
   		}
		else {
			SetState(vsEvaluating,TRUE);
			UpdateVariableLinks();
			ret = evaluate(t,index1,index2) + ((state&vsAccumulate) ? ret : 0);
			SetState(vsEvaluating,FALSE);
		}
	}

	if (realized) {
		if ((((state&vsAlwaysEvaluate) || force_evaluation) && iscurrent) || !contains) {
			// Add value to queue and report error if there is a problem
			if (!realized->AddAt(*t,t->Now(),t->IncUnits(),t->IncLength(),ret))
				ReportError(XIllegalTimeLag,&((CMTime&)*t));
		}
		// Time lag is positive
		else if (contains < 0)
			ReportError(XIllegalTimeLag,&((CMTime&)*t));
		else
			ret = realized->At(*t,t->IncUnits(),t->IncLength(),which);
	}

	if (GetState() & vsCircularity) {
		ReportError(XCircularity,&((CMTime&)*t));
      SetState(vsCircularity,FALSE);
   }

	return ret;
}

void CMVariable::SetValue(CMTimeMachine* t,double val)
{
	if (state&vsResetRequired)
		reset_trial(t);

	if (realized) {
		if (!realized->AddAt(*t,t->Now(),t->IncUnits(),t->IncLength(),val))
			ReportError(XIllegalTimeLag,&((CMTime&)*t));
	}
}

void CMVariable::set_value_queue(int sz)
{
	if (realized)
		delete realized;
	realized = 0;

	if ( sz>0 && !(state&vsDontMaintain)) {
		realized = new CMValueQueue(sz);
		realized->Reset();
	}
}

wistream& CMVariable::read(wistream& s)
{
	CMString str,token,token2;
	if (state&vsDontEdit)
   	return s;
	long pos = (long)s.tellg();
	int begin = 0;

	type = NOTYPE;
	while(!s.eof()) {
		pos = (long)s.tellg();
		str.read_line(s);
		if (str.is_null() || str[0] == L'*')
			continue;
		else if (str(0,wcslen(vardef_begin)) == vardef_begin) {
			begin = 1;
			continue;
		}
		else if (str(0,wcslen(vardef_end)) == vardef_end) {
			if (!begin)
				continue;
			else
				break;
		}
		else if (!begin)
			continue;
		else if (str[0] != L'#')
			break;
		CMTokenizer next(str.substr(1,str.length()-1));
		token = next(L" \t\r\n");

		if (token==L"type") {
			while (!(token2=next(L" \t\r\n")).is_null())
				SetType(token2);
		}
		else {
			token2 = stripends(next(L"\r\n"));
			AddAssociation(token,token2);
		}
	}
	s.seekg(pos,ios::beg);

	read_body(s);

	associations.Compact();
	return s;
}

void CMVariable::read_body(wistream& s)
{
	CMString str;
	while(!s.eof()) {
		str.read_line(s);
		if (str(0,wcslen(vardef_end)) == vardef_end)
			break;
	}
}

wostream& CMVariable::write(wostream& s)
{
	unsigned short i;
	s << setprecision(output_precision);
	s << vardef_begin << L"  " << name;

	if ((type!=NOTYPE&&type>=0) || types.Count())
		s << ENDL << L"#type";
	if (type!=NOTYPE && type>=0 && (type%100)==0)
		s << L"  " << CMVariableTypes::VarStringFromInt(type,1);
	for (i=0;i<types.Count();i++)
		s << L"  " << *types[i];
	for (i=0;i<associations.Count();i++) {
		if (i && associations.GetName(i)==associations.GetName(i-1))
			s << L"  " << associations.GetValue(i);
		else
			s << ENDL << L'#' << associations.GetName(i) << L"  " << associations.GetValue(i);
	}
	s << ENDL;
	write_body(s);
	return s << vardef_end << ENDL;
}

int CMVariable::is_less_than(const CMIrpObject& o2) const
{
	CMVariable& v2 = (CMVariable&)o2;
	int ret=0;
	if (CMVariable::sort_method == CMVariable::byName)
		ret = (name < v2.name);
	else if (CMVariable::sort_method == CMVariable::byType) {
		int t1 = type<0 ? type : (int)type/100;
		int t2 = v2.type<0 ? v2.type : (int)v2.type/100;
		if (!GetState(CMVariable::vsSystem) && v2.GetState(CMVariable::vsSystem)) ret=0;
		else if (GetState(CMVariable::vsSystem) && !v2.GetState(CMVariable::vsSystem)) ret=1;
		else if (GetState(CMVariable::vsSystem) && v2.GetState(CMVariable::vsSystem)) ret=(name < v2.name);
		else if (*this==v2 || t1 > t2)	ret = 0;
		else if (t1 < t2)    	ret = 1;
		else                   	ret = (name < v2.name);
	}
	return ret;
}

