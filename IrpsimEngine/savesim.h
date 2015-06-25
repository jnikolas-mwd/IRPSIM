// savesim.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMSaveSimulation is a virtual base class for saving simulation outcomes.
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
#include "irpapp.h"
#include "reliab.h"
#include "simulat.h"

#include "cmdefs.h"
#include "units.h"
#include "smparray.h"
#include "cmstring.h"

#include <fstream>
using namespace std;

class _IRPCLASS CMIndexAndValue {
	unsigned _index;
	int _state;
	double _value;
public:
	CMIndexAndValue(unsigned i, int s) { _index = i; _state = s; }
	CMIndexAndValue(const CMIndexAndValue& n) { _index = n.GetIndex(); _state = n.GetState(); }
	CMIndexAndValue& operator = (const CMIndexAndValue& n) { _index = n.GetIndex(); _state = n.GetState(); return *this; }

   int GetIndex() const { return _index; }
   void SetIndex(int value) { _index = value; }

   int GetState() const { return _state; }
   BOOL IsCostVariable() const {return (_state&CMVariableDescriptor::vdMoney) ? true : false; }

   double GetValue() const { return _value; }
   void SetValue(double value) { _value = value; }

   int operator == (const CMIndexAndValue& n) const {return (_index==n._index);}
   int operator < (const CMIndexAndValue& n) const {return (_index<n._index);}
};

class _IRPCLASS CMSaveSimulation
{
private:
	float* _aggresults;
    unsigned* _aggindex;
	int _costPrecision = 0;
	int _precision = 10;

	void get_data_from_simulation();
    wofstream* open_file(const CMString& name);
	double last_timestep_in_interval(const CMTime& t);
	void output_realizations_record(const CMTime& t,long trialno,long row);
	float get_realization(const CMTime& t,unsigned var,long trial);
	CMTime get_realizations(const CMTime& t,long trial);
   CMString message_header;
protected:
	wofstream* fout;
	CMIrpApplication* app;
	CMSimulation& sim;
   CMSimulationArray* array;
   CMAccumulatorArray* _accum;
   CMReliability* reliability;
   class _IRPCLASS CMPSmallArray<CMIndexAndValue> accumindex;
   class _IRPCLASS CMPSmallArray<CMIndexAndValue> arrayindex;
	CMTime outbeg,outend;
	int outlen;
   int trialwidth,timestepwidth,fieldwidth,maxnamelength,maxtargetlength;
	CMTIMEUNIT simincunits,outincunits;
   int simincsteps,outincsteps;
   long simtimesteps,outtimesteps;
   long _outtrials;
   long trialbeg,trialend;
   unsigned* reldetailindex;
   unsigned  reldetailtargets;
   long realization_records,summary_records,relseries_records,reldetail_records;
	int state;

	virtual void output_item(int which, const wchar_t* val, long row, long col, int width, int prec) = 0;
   virtual void output_item(int which,double val,long row,long col,int width,int prec) = 0;

	virtual void output_record_end(long row) {}
	virtual void output_footer(int which) {}

	enum {OutRealizations,OutSummary,OutReliabilitySeries,OutReliabilityDetail};
   enum {rHeader=0x0001,rCalendarAggregation=0x0002,rSortByTrial=0x0004};
public:
	CMSaveSimulation(CMSimulation& s,CMIrpApplication* a=0);
	~CMSaveSimulation();
   void AddOutputVariable(const CMString& name);
   int Outcomes(const CMString& fname);
   int Summary(const CMString& fname);
   int ReliabilitySeries(const CMString& fname);
   int ReliabilityDetail(const CMString& fname);
};