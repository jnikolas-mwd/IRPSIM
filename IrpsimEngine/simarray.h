// simarray.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMSimulationArray implements an array of simulation outcomes
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "irp.h"
#include "vardesc.h"
#include "timemach.h"

#include "cmdefs.h"
#include "bgvarray.h"
#include "smvarray.h"
#include "cmstring.h"

#include <fstream>
using namespace std;

class _IRPCLASS pointer
{
	double val;
	long   index;
public:
	pointer(double aval=0,long aindex=0) : val(aval) , index(aindex) {}
	pointer(const pointer& p) : val(p.val) , index(p.index) {}
	pointer& operator = (const pointer& p) {val=p.val;index=p.index;return *this;}
	int operator == (const pointer& p) const {return val==p.val;}
	int operator < (const pointer& p) const {return val<p.val;}
	long Index() {return index;}
};

class _IRPCLASS CMSimulationArray
{
	CMVBigArray<float>* array;
	CMVSBigArray<pointer>* key;
	CMVSmallArray<CMVariableDescriptor>* vardesc;

	long savetrials;
	CMTIMEUNIT incunits;			// e.g. CMTime::months, CMTime::years
	short inclength;			// e.g. # of months, # of years in increment
	CMTime beg;          // beginning of period
	CMTime end;          // end of period
	CMTimeMachine time;
	unsigned short nvars;      // number of variables being recorded
	long	 periodlength; // length of period (e.g. # of months)
	short	 state;
	enum {failbit=0x0001,readonly=0x0002,deletefileonclose=0x0004};
public:
	CMSimulationArray(const CMTimeMachine& t, unsigned nv, unsigned numTrials);
	~CMSimulationArray();
	int Fail() const {return (state & failbit);}
	void  Reset();
	void  AssignVariable(unsigned short var,const CMString& name,const CMString& type,int s)
		{if (vardesc) vardesc->AddAt(var,CMVariableDescriptor(name,type,s));}
	void  AddAt(const CMTime& t,unsigned var,long trial,float val);
	float At(const CMTime& t,unsigned var,long trial,int usekey=0);
	float At(const CMTime& t,const CMString& var,long trial,int usekey=0)
		{return At(t,VariableIndex(var),trial,usekey);}
	CMTime Aggregate(const CMTime& time,long trial,int resolution,unsigned* varindex,float* results,unsigned n);
	float Aggregate(const CMTime& time,const CMString& var,long trial,int resolution);
	CMTime Summary(const CMTime& time,int resolution,unsigned* varindex,float* vmean,float* vstderr,float* vmin,float* vmax,unsigned n,long begtrial=-1,long endtrial=-1);

	long Count() const {return array->Count();}
	long Trials()  const;
	CMTime BeginPeriod() const {return beg;}
	CMTime EndPeriod() const {return end;}
	long TimeSteps() const {return periodlength;}
	long GetPeriod(CMTime& b,CMTime& e,CMTIMEUNIT& units,short& length) const;
	unsigned short Variables()  const {return nvars;}
	CMTimeMachine TimeMachine()  const {return time;}
	int MakeKey(const CMTime& t,int var);
	CMString GetVariableName(unsigned n) const;
	CMString GetVariableType(unsigned n) const;
	int  GetVariableState(unsigned n) const;
	void SetVariableState(unsigned n,int aState,BOOL action);
   unsigned VariableIndex(const CMString& name) const;
	void WriteBinary();
	void WriteBinary(wostream& s);
	void ReadBinary();
	long BinarySize() const;
};

