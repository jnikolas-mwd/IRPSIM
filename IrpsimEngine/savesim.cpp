// savesim.cpp : implementation file
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
#include "StdAfx.h"
#include "savesim.h"

#include "cmdefs.h"
#include "token.h"
#include "cmlib.h"
#include "notify.h"

#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <mem.h>
#include <ctype.h>

#include <fstream>
static wofstream sdebug(L"debug_savesim.txt");

CMSaveSimulation::CMSaveSimulation(CMSimulation& s,CMIrpApplication* a) :
fout(0),
message_header(L"Writing "),
app(a),
sim(s),
array(s.SimArray()),
_accum(s.Accumulator()),
reliability(s.Reliability()),
outbeg(),
outend(),
outlen(0),
outtimesteps(0),
trialbeg(0),
trialend(0),
accumindex(),
arrayindex(),
simincunits(CM_MONTH),
simincsteps(1),
outincunits(CM_MONTH),
outincsteps(1),
maxnamelength(1),
maxtargetlength(1),
reldetailindex(0),
reldetailtargets(0),
realization_records(0),
summary_records(0),
relseries_records(0),
reldetail_records(0),
_aggresults(0),
_aggindex(0),
state(0)
{
	_outtrials = sim.Trials();
	trialend = _outtrials-1;
	_accum->GetPeriod(outbeg,outend,simincunits,simincsteps);
	trialwidth = 1+(int)ceil(log10(cmmax(1,_outtrials)));
}

void CMSaveSimulation::get_data_from_simulation()
{
	outlen = 0;
	outtimesteps = 0;
	state = 0;
	CMString str;

	sdebug << "Output folder " << sim.GetOption(L"outputfolder") << endl;
	sdebug << "Id " << sim.GetId() << endl;

	_precision = sim.GetOptionInt(L"precision");
	_costPrecision = sim.GetOptionInt(L"costprecision");
	sdebug << "Precision = " << _precision << endl;
	sdebug << "Cost Precision = " << _costPrecision << endl;

	_accum->GetPeriod(outbeg,outend,outincunits,outincsteps);

	sdebug << "outbeg=" << outbeg << endl;
	sdebug << "outend=" << outend << endl;
	sdebug << "outincunits=" << outincunits << endl;
	sdebug << "outincsteps=" << outincsteps << endl;

	_outtrials = sim.Trials();
	trialbeg = 0;
	trialend = _outtrials - 1;
	outincunits = simincunits;
	outincsteps = 1;
	simtimesteps = 1+CMTime::Diff(outend,outbeg,simincunits,simincsteps);
    outtimesteps = 1+CMTime::Diff(outend,outbeg,outincunits,outincsteps);

	fieldwidth = outlen > 0 ? outlen : 9;
	switch (outincunits) {
		case CM_SECOND: timestepwidth=14; break;
		case CM_MINUTE: timestepwidth=12; break;
		case CM_HOUR: 	 timestepwidth=10; break;
		case CM_DAY: 	 timestepwidth=8; break;
		case CM_WEEK: 	 timestepwidth=8; break;
		case CM_YEAR: 	 timestepwidth=4; break;
		default: 		 timestepwidth=6; break;
   }

	reldetailtargets = 0;
	reldetailindex = new unsigned[reliability->Targets()>0 ? reliability->Targets() : 1];
    maxtargetlength=0;

	for (unsigned i = 0; i < reliability->Targets(); i++) {
		CMReliabilityTarget* target = reliability->Target(i);
		unsigned j = target->GetString().length();
		maxtargetlength = cmmax(maxtargetlength, (int)j);
		for (j = 0; j < i; j++)
			if (target->SameCurveAs(*reliability->Target(j)))
				break;
		if (j == i)
			reldetailindex[reldetailtargets++] = i;
	}

	realization_records = (trialend-trialbeg+1) * outtimesteps;
	summary_records = simtimesteps * accumindex.Count();
    relseries_records = reliability->Targets() * simtimesteps;
	reldetail_records = reldetailtargets * outtimesteps;

	str = sim.GetOption(L"outputsortby");
	if (str.is_null() || str==L"trial")
   	state |= rSortByTrial;
}

CMSaveSimulation::~CMSaveSimulation()
{
	arrayindex.ResetAndDestroy();
	accumindex.ResetAndDestroy();
   if (reldetailindex) delete [] reldetailindex;
	if (_aggresults) delete [] _aggresults;
	if (_aggindex) 	 delete [] _aggindex;
   if (fout) delete fout;
}

void CMSaveSimulation::AddOutputVariable(const CMString& name)
{
	maxnamelength = cmmax(maxnamelength, (int)name.length());
	unsigned n = array->VariableIndex(name);
	if (n < array->Variables())
		arrayindex.Add(new CMIndexAndValue(n, array->GetVariableState(n)));
	n = _accum->VariableIndex(name);
	if (n < _accum->Variables())
		accumindex.Add(new CMIndexAndValue(n, _accum->GetVariableState(n)));
}

float CMSaveSimulation::get_realization(const CMTime& t,unsigned var,long trial)
{
   if (!array) return 0;
   float val;
   if (state & rCalendarAggregation)
		array->Aggregate(t,trial,outincunits,&var,&val,1);
	else if (outincsteps > 1)
		array->Aggregate(t,trial,outincsteps,&var,&val,1);
	else
     	val = array->At(t,var,trial);
   return val;
}

CMTime CMSaveSimulation::get_realizations(const CMTime& tm,long trial)
{
	CMTime ret;
   if (state & rCalendarAggregation)
		ret = array->Aggregate(tm,trial,outincunits,_aggindex,_aggresults,arrayindex.Count());
	else
		ret = array->Aggregate(tm,trial,outincsteps,_aggindex,_aggresults,arrayindex.Count());
	for (unsigned i=0;i<arrayindex.Count();i++)
		arrayindex[i]->SetValue(_aggresults[i]);
   return ret;
}

/*
   if (!array) return;
   float val;
   int esc=0,steps=0;
   CMTime t=tm;
   while (!esc) {
		for (unsigned i=0;i<arrayindex.Count();i++) {
	     	val = array->At(t,arrayindex[i]->index,trial);
			if (!(arrayindex[i]->state & CMVariableDescriptor::vdSum) || t==tm)
         	arrayindex[i]->value = val;
      	else
         	arrayindex[i]->value += val;
		}
	   steps++;
      t.inc(simincsteps,simincunits);
      if (state & rCalendarAggregation) {
	      switch (outincunits) {
   	   	case CM_YEAR: esc = (t.Year() != tm.Year()); break;
	      	case CM_MONTH:	esc = (t.Month() != tm.Month()); break;
	      	case CM_WEEK: esc = ((ULONG)t >= (ULONG)tm+7); break;
	      	case CM_DAY: esc = ((ULONG)t != (ULONG)tm); break;
      		case CM_HOUR: esc = (t.Hour() != tm.Hour()); break;
	      	case CM_MINUTE: esc = (t.Minute() != tm.Minute()); break;
	         default: esc=1; break;
	      }
   	}
		else
      	esc = (steps>=outincsteps);
   }
	for (unsigned i=0;i<arrayindex.Count();i++)
		arrayindex[i]->Translate();
}
*/

wofstream* CMSaveSimulation::open_file(const CMString& fname)
{
	sdebug << "Opening " << fname << endl;
	if (fout)
		delete fout;
	fout = new wofstream(fname.c_str(), IOS_BINARY);
	if (fout->fail()) {
		CMNotifier::Notify(CMNotifier::WARNING, L"Unable to open file for output: " + fname);
		delete fout;
		fout = 0;
	}
	return fout;
}

double CMSaveSimulation::last_timestep_in_interval(const CMTime& t)
{
	wchar_t buffer[128];
	CMTime tm(t);
   tm.inc(outincsteps,outincunits);
   tm.inc(-1,simincunits);
   return _wtof(tm.GetString(buffer, 128));
}

void CMSaveSimulation::output_realizations_record(const CMTime& t,long trialno,long row)
{
	wchar_t buffer[128];

	CMTime tm = get_realizations(t,trialno);
    output_item(OutRealizations,trialno+1,row,0,fieldwidth,0);
    output_item(OutRealizations,_wtof(tm.GetString(buffer, 128)),row,1,fieldwidth,0);

	for (unsigned i = 0; i < arrayindex.Count(); i++)
		output_item(OutRealizations, arrayindex[i]->GetValue(), row, i + 2, fieldwidth, arrayindex[i]->IsCostVariable() ? _costPrecision : _precision);   //arrayindex[i]->units->Precision()); ***TODO Change how precision is obtained

   output_record_end(row);
}

int CMSaveSimulation::Outcomes(const CMString& fname)
{
	if (!open_file(fname))
		return -1;

	if (_aggresults) delete[] _aggresults;
	if (_aggindex) 	 delete[] _aggindex;

	_aggresults = new float[arrayindex.Count()];
	_aggindex = new unsigned[arrayindex.Count()];

	sdebug << "Writing outcomes to " << fname << " array size = " << arrayindex.Count() << endl;

	get_data_from_simulation();

	output_item(OutRealizations, L"Trial", 0, 0, fieldwidth, 0);
	output_item(OutRealizations, L"Period", 0, 1, fieldwidth, 0);

	long row = 0;

	for (unsigned i = 0; i < arrayindex.Count(); i++) {
		_aggindex[i] = arrayindex[i]->GetIndex();
		output_item(OutRealizations, array->GetVariableName(arrayindex[i]->GetIndex()).c_str(), 0, i + 2, fieldwidth, arrayindex[i]->IsCostVariable() ? _costPrecision : _precision);
	}

	output_record_end(row++);

	int oldformat = CMTime::SetOutputFormat(outincunits);

	long nrows = 1 + realization_records;

	long trialno;

	CMTime tm;

	if (state & rSortByTrial) {
		for (trialno = trialbeg; trialno <= trialend; trialno++) {
			for (tm = outbeg; tm <= outend; tm.inc(outincsteps, outincunits))
				output_realizations_record(tm, trialno, row++);
		}
	}
	else {
		for (tm = outbeg; tm <= outend; tm.inc(outincsteps, outincunits)) {
			for (trialno = trialbeg; trialno <= trialend; trialno++)
				output_realizations_record(tm, trialno, row++);
		}
	}

	output_footer(OutRealizations);

	delete[] _aggresults;
	delete[] _aggindex;
	_aggresults = 0;
	_aggindex = 0;
	delete fout;
	fout = 0;
	CMNotifier::Notify(CMNotifier::INFO, L"");
	CMTime::SetOutputFormat(oldformat);
	return 0;
}

int CMSaveSimulation::Summary(const CMString& fname)
{
	unsigned i;

	if (!open_file(fname))
		return -1;
	wchar_t buffer[128];
	get_data_from_simulation();

	long row = 0;

	sdebug << "Writing summary file" << endl;

	int oldformat = CMTime::SetOutputFormat(simincunits);
	long nrows = 1 + summary_records;

	for (i = 0; i < accumindex.Count(); i++) {
		double val;
		unsigned varindex = accumindex[i]->GetIndex();
		const wchar_t* vname = _accum->GetVariableName(varindex).c_str();
		sdebug << vname << " cost var=" << accumindex[i]->IsCostVariable() << endl;
		for (CMTime tm = outbeg; tm <= outend; tm.inc(simincsteps, simincunits), row++) {
			output_item(OutSummary, vname, row, 0, maxnamelength, 0);
			output_item(OutSummary, _wtof(tm.GetString(buffer, 128)), row, 1, fieldwidth, 0);
			for (int j = 0; j < 4; j++) {
				switch (j) {
				case 0: val = _accum->Mean(tm, varindex); break;
				case 1: val = _accum->StdDev(tm, varindex); break;
				case 2: val = _accum->Min(tm, varindex); break;
				case 3: val = _accum->Max(tm, varindex); break;
				}
				output_item(OutSummary, val, row, j + 2, fieldwidth, accumindex[i]->IsCostVariable() ? _costPrecision : _precision);
			}
			output_record_end(row);
		}
	}

	output_footer(OutSummary);

	CMTime::SetOutputFormat(oldformat);
	delete fout;
	fout = 0;
	return 0;
}

int CMSaveSimulation::ReliabilitySeries(const CMString& fname)
{
	unsigned i;

	if (!open_file(fname))
   	return -1;
	wchar_t buffer[128];
	get_data_from_simulation();
	int oldformat = CMTime::SetOutputFormat(simincunits);

	long row=0;

   long nrows = 1 + relseries_records;

	for (i=0;i<reliability->Targets();i++) {
		const wchar_t* tname = reliability->Target(i)->GetString().c_str();
		for (CMTime tm=outbeg;tm<=outend;tm.inc(simincsteps,simincunits),row++) {
		   output_item(OutReliabilitySeries,tname,row,0,maxtargetlength,0);
		   output_item(OutReliabilitySeries,_wtof(tm.GetString(buffer, 128)),row,1,fieldwidth,0);
		   output_item(OutReliabilitySeries,reliability->FailPct(tm,i),row,2,fieldwidth,1);
         output_record_end(row);
      }
   }

	output_footer(OutReliabilitySeries);

	CMTime::SetOutputFormat(oldformat);
   delete fout;
   fout=0;
   return 0;
}

int CMSaveSimulation::ReliabilityDetail(const CMString& fname)
{
	if (!open_file(fname))
   	return -1;
	get_data_from_simulation();
	int oldformat = CMTime::SetOutputFormat(outincunits);

   int i,j;
   int step;

	if (!reldetailtargets)
		return 0;

   long row = 0;
   CMTime tm;

	long** bins = new long*[reldetailtargets*outtimesteps];
	for (i=0;i<outtimesteps;i++)
   	bins[i] = new long[20];
	double* expected = new double[reldetailtargets*outtimesteps];
 	double* maxshortage = new double[reldetailtargets*outtimesteps];

	for (i=0;i<(int)reldetailtargets*outtimesteps;i++) {
	  	expected[i]=0;
   	maxshortage[i]=100;
      for (j=0;j<20;j++)
			bins[i][j] = 0;
	}

   step = 0;

	for (tm=outbeg;tm<=outend;tm.inc(outincsteps,outincunits),step++) {
		for (long trial=trialbeg;trial<=trialend;trial++) {
			for (i=0;i<(int)reldetailtargets;i++) {
				CMReliabilityTarget* target = reliability->Target(reldetailindex[i]);
				unsigned sindex = array->VariableIndex(target->ShortageVariable());
				unsigned pindex = array->VariableIndex(target->PercentageVariable());
				int ispercent = (target->IsPercent() && pindex<array->Variables());
				double sval;
				if (!ispercent && sindex<array->Variables()) {
					maxshortage[step*reldetailtargets+i]=0;
					for (long tr=trialbeg;tr<=trialend;tr++) {
						sval = target->ShortageSign() * get_realization(tm,sindex,tr);
            	   if (sval > maxshortage[step*reldetailtargets+i]) maxshortage[step*reldetailtargets+i] = sval;
	            }
   	      }
      	   sval=0;
				if (sindex<array->Variables()) {
					sval = target->ShortageSign() * get_realization(tm,sindex,trial);
					double pval = (ispercent) ? (target->PercentSign()*get_realization(tm,pindex,trial)) : 0;
					if (pval != 0) sval *= (100/pval);
				}
				if (sval>1e-9) {
					expected[step*reldetailtargets+i] += sval;
					int binno = ispercent ? (int)(sval/5) : (int)(20*sval/maxshortage[step*reldetailtargets+i]);
					if (binno>=20) binno=19;
					bins[step*reldetailtargets+i][binno]++;
				}
			}
      }
	}

   for (step=0;step<(int)outtimesteps;step++)
		for (i=0;i<(int)reldetailtargets;i++)
			for (j=18;j>=0;j--)
				bins[step*reldetailtargets+i][j] += bins[step*reldetailtargets+i][j+1];

	step = 0;
	for (tm=outbeg;tm<=outend;tm.inc(outincsteps,outincunits),step++) {
		for (i=0;i<(int)reldetailtargets;i++,row++) {
   		long col = 0;
		   output_item(OutReliabilityDetail,reliability->Target(reldetailindex[i])->GetString().c_str(),row,col++,maxtargetlength,0);
		   output_item(OutReliabilityDetail,last_timestep_in_interval(tm),row,col++,fieldwidth,0);
         for (j=0;j<20;j++)
				output_item(OutReliabilityDetail,100*(double)bins[step*reldetailtargets+i][j]/_outtrials,row,col++,fieldwidth,1);
			output_item(OutReliabilityDetail,100*(double)bins[step*reldetailtargets+i][0]/_outtrials,row,col++,fieldwidth,1);
			output_item(OutReliabilityDetail,expected[step*reldetailtargets+i]/_outtrials,row,col++,fieldwidth,1);
         output_record_end(row);
      }
   }

	output_footer(OutReliabilityDetail);

	delete [] expected;
	delete [] maxshortage;
	for (i=0;i<outtimesteps;i++)
   	delete [] bins[i];
	delete [] bins;
   delete fout;
   fout = 0;

   CMTime::SetOutputFormat(oldformat);
   return 0;
}


