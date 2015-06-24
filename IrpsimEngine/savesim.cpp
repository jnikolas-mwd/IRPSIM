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

const wchar_t* CMSaveSimulation::realization_header_names[] =
	{L"trial",L"period",0};
const wchar_t* CMSaveSimulation::summary_header_names[] =
	{L"name",L"period",L"mean",L"stderr",L"min",L"max",0};
const wchar_t* CMSaveSimulation::relseries_header_names[] =
	{L"target",L"period",L"failpct",0};
const wchar_t* CMSaveSimulation::reldetail_header_names[] =
	{L"target",L"period",L"0%",L"5%",L"10%",L"15%",L"20%",L"25%",L"30%",L"35%",L"40%",L"45%",
    L"50%",L"55%",L"60%",L"65%",L"70%",L"75%",L"80%",L"85%",L"90%",L"95%",L"P(S)",L"E(S)",0};

CMUnits CMIndexAndValue::defaultunits;

CMIndexAndValue::CMIndexAndValue(unsigned i,int s,CMUnits* vunits,CMUnits* cunits) :
index(i),
state(s),
units(vunits)
{
	if (state&CMVariableDescriptor::vdMoney)
		units=cunits;
	else if (state & CMVariableDescriptor::vdNoUnits)
		units=&defaultunits;
}

CMSaveSimulation::CMSaveSimulation(CMSimulation& s,CMIrpApplication* a) :
fout(0),
message_header(L"Writing "),
app(a),
sim(s),
array(s.SimArray()),
accum(s.Accumulator()),
reliability(s.Reliability()),
outbeg(),
outend(),
outvolunits(),
outcostunits(),
outlen(0),
outtimesteps(0),
outtrials(0),
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
aggresults(0),
aggindex(0),
state(0)
{
	outtrials = sim.Trials();
	trialend = outtrials-1;
	accum->GetPeriod(outbeg,outend,simincunits,simincsteps);
	trialwidth = 1+(int)ceil(log10(cmmax(1,outtrials)));
}

void CMSaveSimulation::get_data_from_simulation()
{
	static const wchar_t* delims = L" \t,-";
	outlen = 0;
	outtimesteps = 0;
	state = 0;

	outvolunits.Set(sim.GetOption(L"outputvolumeunits"));
	outcostunits.Set(sim.GetOption(L"outputcostunits"));
   if (sim.GetOption(L"outputheader")==L"yes")
   	state |= rHeader;

	CMString str = sim.GetOption(L"outputdelimiter");
	if (str[0] == L's' || str[0] == L'S') {
		outch = L' ';outlen = _wtoi(((CMString)str.substr(1)).c_str());
	}
	else if (iswdigit(str[0])) {
		outch = L'\0';outlen = _wtoi(str.c_str());
	}
	else {
		outch = str[0];outlen = 0;
	}

	CMTokenizer next(sim.GetOption(L"outputperiod"));
	CMString begstr(next(delims));
	CMString endstr(next(delims));

	accum->GetPeriod(outbeg,outend,outincunits,outincsteps);

	if (begstr.length() && begstr != L"entire") {
		if (!endstr.length())
      	endstr = begstr;
		CMTime beg,end;
      int lastmonth = CMTime::Month(sim.GetOption(L"yearend").c_str());
		CMTime::GetInterval(begstr.c_str(),endstr.c_str(),simincunits,simincsteps,lastmonth,beg,end);
		if (beg>=outbeg && beg<=outend) outbeg = beg;
		if (end>=outbeg && end<=outend) outend = end;
   }

	next.Reset(sim.GetOption(L"outputtrials"));
	begstr = next(delims);
	endstr = next(delims);

	trialbeg = 0;
   outtrials = sim.Trials();
	trialend = outtrials-1;

	if (begstr.length() && begstr!=L"all") {
		if (!endstr.length())
      	endstr = begstr;
		trialbeg = _wtol(begstr.c_str()) - 1;
		trialend = _wtol(endstr.c_str()) - 1;
      if (trialbeg<0 || trialbeg>=outtrials)
      	trialbeg = 0;
      if (trialend<trialbeg)
      	trialend = trialbeg;
      if (trialend>=outtrials)
      	trialend = outtrials-1;
		outtrials = trialend-trialbeg+1;
   }

   str = sim.GetOption(L"outputResolution");
   if (iswdigit(str[0])) {
	   outincsteps = _wtoi(str.c_str());
     	if (outincsteps<=0) outincsteps = 1;
		outincunits = simincunits;
   }
   else {
   	state |= rCalendarAggregation;
	   outincunits = (int)CMTime::StringToTimeUnit(str);
      outincsteps=1;
   }

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

	for (unsigned i=0;i<reliability->Targets();i++) {
		CMReliabilityTarget* target = reliability->Target(i);
		unsigned j = target->GetString().length();
      maxtargetlength = cmmax(maxtargetlength,(int)j);
		for (j=0;j<i;j++)
			if (target->SameCurveAs(*reliability->Target(j)))
				break;
		if (j==i)
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
	if (aggresults) delete [] aggresults;
	if (aggindex) 	 delete [] aggindex;
   if (fout) delete fout;
}

void CMSaveSimulation::AddOutputVariable(const CMString& name)
{
   maxnamelength = cmmax(maxnamelength,(int)name.length());
	unsigned n = array->VariableIndex(name);
   if (n < array->Variables())
   	arrayindex.Add(new CMIndexAndValue(n,array->GetVariableState(n),&outvolunits,&outcostunits));
	n = accum->VariableIndex(name);
   if (n < accum->Variables())
   	accumindex.Add(new CMIndexAndValue(n,accum->GetVariableState(n),&outvolunits,&outcostunits));
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
		ret = array->Aggregate(tm,trial,outincunits,aggindex,aggresults,arrayindex.Count());
	else
		ret = array->Aggregate(tm,trial,outincsteps,aggindex,aggresults,arrayindex.Count());
	for (unsigned i=0;i<arrayindex.Count();i++)
		arrayindex[i]->value = arrayindex[i]->Translate(aggresults[i]);
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
	if (fout)
	  	delete fout;
	fout = new wofstream(fname.c_str(),IOS_BINARY);
   if (fout->fail()) {
   	delete fout;
      fout=0;
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

	for (unsigned i=0;i<arrayindex.Count();i++)
      output_item(OutRealizations,arrayindex[i]->value,row,i+2,fieldwidth,arrayindex[i]->units->Precision());

   output_record_end(row);
}

int CMSaveSimulation::Outcomes(const CMString& fname)
{
	if (!open_file(fname))
   	return -1;

	if (aggresults) delete [] aggresults;
	if (aggindex) 	 delete [] aggindex;

	aggresults = new float[arrayindex.Count()];
   aggindex = new unsigned[arrayindex.Count()];

   sdebug << "Writing outcomes to " << fname << endl;

	get_data_from_simulation();

	output_header(OutRealizations);

	long row=0;

	output_item(OutRealizations,realization_header_names[0],0,0,fieldwidth,0);
	output_item(OutRealizations,realization_header_names[1],0,1,fieldwidth,0);

	for (unsigned i=0;i<arrayindex.Count();i++) {
   		aggindex[i] = arrayindex[i]->index;
		output_item(OutRealizations,array->GetVariableName(arrayindex[i]->index).c_str(),0,i+2,fieldwidth,arrayindex[i]->units->Precision());
   }

	output_record_end(row++);

	int oldformat = CMTime::SetOutputFormat(outincunits);

	long nrows = 1 + realization_records;

	long trialno;

	CMTime tm;
	if (state & rSortByTrial) {
		for (trialno=trialbeg;trialno<=trialend;trialno++) {
			for (tm=outbeg;tm<=outend;tm.inc(outincsteps,outincunits))
				output_realizations_record(tm,trialno,row++);
		}
	}
	else {
		for (tm=outbeg;tm<=outend;tm.inc(outincsteps,outincunits)) {
			for (trialno=trialbeg;trialno<=trialend;trialno++)
				output_realizations_record(tm,trialno,row++);
		}
	}

	output_footer(OutRealizations);

	delete [] aggresults;
   delete [] aggindex;
   aggresults = 0;
   aggindex = 0;
	delete fout;
	fout=0;
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

	output_header(OutSummary);

	long row=0;

	for (i=0;summary_header_names[i]!=0;i++)
		output_item(OutSummary,summary_header_names[i],0,i,i==0?maxnamelength:fieldwidth,i<2?0:outvolunits.Precision());

	output_record_end(row++);

	int oldformat = CMTime::SetOutputFormat(simincunits);
	long nrows = 1 + summary_records;

	for (i=0;i<accumindex.Count();i++) {
		double val;
      unsigned varindex = accumindex[i]->index;
	  const wchar_t* vname = accum->GetVariableName(varindex).c_str();
		for (CMTime tm=outbeg;tm<=outend;tm.inc(simincsteps,simincunits),row++) {
		   output_item(OutSummary,vname,row,0,maxnamelength,0);
		   output_item(OutSummary,_wtof(tm.GetString(buffer, 128)),row,1,fieldwidth,0);
			for (int j=0;j<4;j++) {
				switch (j) {
					case 0: val = accum->Mean(tm,varindex); break;
					case 1: val = accum->StdDev(tm,varindex); break;
					case 2: val = accum->Min(tm,varindex); break;
					case 3: val = accum->Max(tm,varindex); break;
				}
				output_item(OutSummary,accumindex[i]->units->TranslateToLocal(val),row,j+2,fieldwidth,outvolunits.Precision());
         }
			output_record_end(row);
		}
	}

	output_footer(OutSummary);

	CMTime::SetOutputFormat(oldformat);
	delete fout;
   fout=0;
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

   output_header(OutReliabilitySeries);

	long row=0;

	for (i=0;relseries_header_names[i]!=0;i++)
		output_item(OutReliabilitySeries,relseries_header_names[i],0,i,i==0?maxtargetlength:fieldwidth,i<2?0:1);

	output_record_end(row++);

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

   output_header(OutReliabilityDetail);

   long row = 0;
   CMTime tm;

	for (i=0;reldetail_header_names[i]!=0;i++)
		output_item(OutReliabilityDetail,reldetail_header_names[i],0,i,i==0?maxtargetlength:fieldwidth,i<2?0:1);

	output_record_end(row++);

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
				output_item(OutReliabilityDetail,100*(double)bins[step*reldetailtargets+i][j]/outtrials,row,col++,fieldwidth,1);
			output_item(OutReliabilityDetail,100*(double)bins[step*reldetailtargets+i][0]/outtrials,row,col++,fieldwidth,1);
			output_item(OutReliabilityDetail,expected[step*reldetailtargets+i]/outtrials,row,col++,fieldwidth,1);
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


