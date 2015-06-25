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
	static CMUnits defaultunits;
public:
	unsigned index;
   int state;
	CMUnits* units;
   double value;
   CMIndexAndValue(unsigned i=0,int s=0,CMUnits* vunits=0,CMUnits* cunits=0);
   CMIndexAndValue(const CMIndexAndValue& n) {index=n.index;state=n.state;units=n.units;value=n.value;}
   CMIndexAndValue& operator = (const CMIndexAndValue& n) {index=n.index;state=n.state;units=n.units;value=n.value;return *this;}
	double Translate(double v) {return (units) ? units->TranslateToLocal(v) : v;}   
   int operator == (const CMIndexAndValue& n) const {return (index==n.index);}
	int operator < (const CMIndexAndValue& n) const {return (index<n.index);}
};

class _IRPCLASS CMSaveSimulation
{
private:
	float* _aggresults;
    unsigned* _aggindex;
	int _precision = 10;

	void get_data_from_simulation();
    wofstream* open_file(const CMString& name);
	double last_timestep_in_interval(const CMTime& t);
	void output_realizations_record(const CMTime& t,long trialno,long row);
	float get_realization(const CMTime& t,unsigned var,long trial);
	CMTime get_realizations(const CMTime& t,long trial);
   CMString message_header;
protected:
	static const wchar_t* realization_header_names[];
	static const wchar_t* summary_header_names[];
	static const wchar_t* relseries_header_names[];
	static const wchar_t* reldetail_header_names[];
	wofstream* fout;
	CMIrpApplication* app;
	CMSimulation& sim;
   CMSimulationArray* array;
   CMAccumulatorArray* accum;
   CMReliability* reliability;
   class _IRPCLASS CMPSmallArray<CMIndexAndValue> accumindex;
   class _IRPCLASS CMPSmallArray<CMIndexAndValue> arrayindex;
	CMTime outbeg,outend;
	CMUnits outvolunits,outcostunits;
	int outlen;
   int trialwidth,timestepwidth,fieldwidth,maxnamelength,maxtargetlength;
	CMTIMEUNIT simincunits,outincunits;
   int simincsteps,outincsteps;
   long simtimesteps,outtimesteps;
   long outtrials;
   long trialbeg,trialend;
   unsigned* reldetailindex;
   unsigned  reldetailtargets;
   long realization_records,summary_records,relseries_records,reldetail_records;
	int state;

	virtual void output_header(int which) = 0;
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