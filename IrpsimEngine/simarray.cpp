// simarray.cpp : implementation file
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
#include "StdAfx.h"
#include "simarray.h"

#include "cmlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <fstream>
static wofstream sdebug("debug_simarray.txt");

static const wchar_t* simid = L"simarray0001";

CMSimulationArray::CMSimulationArray(const CMTimeMachine& t, unsigned nv, unsigned numTrials) :
time(t),
beg(t[0].Begin()),
end(t[0].End()),
incunits(t.IncUnits()),
inclength(t.IncLength()),
nvars(nv),
array(0),
key(0),
vardesc(0),
state(0)
{
	periodlength = CMTime::Diff(end,beg,incunits,inclength) + 1;
	vardesc = nvars ? new CMVSmallArray<CMVariableDescriptor>(nvars) : 0;

	if ((state&readonly) || !nvars || !periodlength || Fail())
		return;

	array = new CMVBigArray<float>(numTrials * periodlength * nvars);
	if (!array)
		state |= failbit;
}

/*
CMSimulationArray::CMSimulationArray(const wchar_t* name) :
time(CM_MONTH,1),
array(0),
key(0),
vardesc(0),
fbuffer(0),
savetrials(0),
savesize(0),
begindex(-1),
state(0)
{
	state |= readonly;
	//file = new wfstream(name,ios::in|ios::out|IOS_BINARY);
	if (file->fail())
		state |= failbit;
	else
		ReadBinary();
}
*/

/*
int CMSimulationArray::open_file()
{
	if (file)
		delete file;
	file = 0;

	if (filename.length()) {
		file = new wfstream(filename.c_str(),ios::in|ios::out|ios::trunc|IOS_BINARY);
		if (file && !file->fail()) {
			file->write((const wchar_t*)head, sizeof(head));
			file->write((const wchar_t*)&filecount, sizeof(filecount));
			file->write((const wchar_t*)&savesize, sizeof(savesize));
			file->write((const wchar_t*)&nvars, sizeof(nvars));
			time.WriteBinary(*file);
		}
		else {
			state |= failbit;
		}
	}
	else {
		state |= failbit;
	}
	if (time[0].End() < time[0].Begin()) {
		state |= failbit;
	}
	return (state & failbit);
}
*/

CMSimulationArray::~CMSimulationArray()
{
	if (array) delete array;
	if (key) delete key;
	if (vardesc) delete vardesc;
}

void CMSimulationArray::Reset()
{
	if (state & readonly)
		return;
	if (array) array->Reset(0);

	if (key) delete key;
	key = 0;
}

/*
void CMSimulationArray::WriteBinary(wostream& s)
{
	if (file && !file->fail() && !s.fail()) {
		long n = filecount + array->Count();
		s.write((const wchar_t*)head, sizeof(head));
		s.write((const wchar_t*)&n, sizeof(n));
		s.write((const wchar_t*)&savesize, sizeof(savesize));
		s.write((const wchar_t*)&nvars, sizeof(nvars));
		time.WriteBinary(s);
		if (nvars) {
      	float * buffer = new float[nvars];
         unsigned bufsize = nvars*sizeof(float);
			file->seekg(HeaderSize(),ios::beg);
			for (n=0;n<filecount;n+=nvars) {
				file->read((wchar_t*)buffer, bufsize);
				s.write((const wchar_t*)buffer, bufsize);
         }
         for (n=0;n<array->Count();n++)
			 s.write((const wchar_t*)&(array->At(n)), sizeof(float));
         delete [] buffer;
		}
		for (unsigned short i=0;i<nvars && vardesc;i++)
			vardesc->At(i).WriteBinary(s);
   }
}

void CMSimulationArray::WriteBinary()
{
	if (!file || Fail())
		return;
	save_array_to_file();
   file->seekp(HeaderSize()+filecount*sizeof(float));
	for (unsigned short i=0;i<nvars && vardesc;i++)
		vardesc->At(i).WriteBinary(*file);
}

void CMSimulationArray::ReadBinary()
{
	if (array) delete array;
	if (key) delete key;
	if (vardesc) delete vardesc;
	if (fbuffer) delete [] fbuffer;
	array=0;	key = 0;	vardesc=0; fbuffer=0;
	if (file && !file->fail()) {
		file->seekg(0L,ios::beg);
		file->read((wchar_t*)head, sizeof(head));
      if (wcscmp(simid,head)) {
      	state |= failbit;
         return;
      }
	  file->read((wchar_t*)&filecount, sizeof(filecount));
	  file->read((wchar_t*)&savesize, sizeof(savesize));
	  file->read((wchar_t*)&nvars, sizeof(nvars));
		time.ReadBinary(*file);
		beg = time[0].Begin();
		end = time[0].End();
		incunits = time.IncUnits();
		inclength = time.IncLength();

		if (filecount<0 || savesize<0 || incunits>0 || inclength<0) {
      	state |= failbit;
         return;
      }

		file->seekg(HeaderSize()+filecount*sizeof(float),ios::beg);
		if (nvars) vardesc = new CMVSmallArray<CMVariableDescriptor>(nvars);
		for (unsigned i=0;i<nvars && vardesc;i++) {
			CMVariableDescriptor vd;
			vd.ReadBinary(*file);
			vardesc->AddAt(i,vd);
		}
	}
	else
		state |= failbit;
	if (end<beg)
		state |= failbit;
	periodlength = CMTime::Diff(end,beg,incunits,inclength) + 1;
	if (nvars) fbuffer = new float[nvars];
	SetSaveSize(savesize);
}

void CMSimulationArray::SetSaveSize(long ss)
{
	if ((state&readonly) || !nvars || !periodlength || Fail())
		return;
	save_array_to_file();
	if (ss>2000000L) ss = 2000000L;
	if (ss<periodlength*nvars) ss=periodlength*nvars;
	savetrials = ss/(periodlength*nvars);
	savesize=savetrials*periodlength*nvars;
	if (array) delete array;
	array = new CMVBigArray<float>(savesize);
	if (!array)
		state |= failbit;
}
*/

unsigned CMSimulationArray::VariableIndex(const CMString& name) const
{
	for (unsigned i=0;i<vardesc->Count();i++)
		if (vardesc->At(i)==name)
			return i;
	return nvars;
}

CMString CMSimulationArray::GetVariableName(unsigned n) const
{
	if (n<vardesc->Count())
	   return vardesc->At(n).Name();
	return CMString();
}

CMString CMSimulationArray::GetVariableType(unsigned n) const
{
	if (n<vardesc->Count())
	   return vardesc->At(n).Type();
	return CMString();
}

int CMSimulationArray::GetVariableState(unsigned n) const
{
	return (n<vardesc->Count()) ? vardesc->At(n).GetState() : 0;
}

void CMSimulationArray::SetVariableState(unsigned n,int aState,BOOL action)
{
	if (n<vardesc->Count())
		vardesc->At(n).SetState(aState,action);
}

/*
void CMSimulationArray::save_array_to_file()
{
	if (!array || !array->Count() || Fail() || !file || (state&readonly))
		return;
	file->clear();
	long count = array->Count();
	filecount+=count;
	file->seekp(0L,ios::beg);
	file->write((const wchar_t*)head, sizeof(head));
	file->write((const wchar_t*)&filecount, sizeof(filecount));
	file->seekp(HeaderSize()+(filecount-count)*sizeof(float),ios::beg);
	for (long i=0;i<count;i++)
		file->write((const wchar_t*)&(array->At(i)), sizeof(float));
	array->Reset();
}
*/
/*
void CMSimulationArray::Add(float val)
{
	if (!array || Fail()) return;
	array->Add(val);
	if (array->Count() >= savesize)
		save_array_to_file();
}
*/

void CMSimulationArray::AddAt(const CMTime& t,unsigned var,long trial,float val)
{
	if (!array || Fail()) return;
	long per = CMTime::Diff(t,beg,incunits,inclength);
	long index = trial*nvars*periodlength + per*nvars + var;
	array->AddAt(index,val);
	//if (array->Count() >= savesize)
		//save_array_to_file();
}

float CMSimulationArray::At(const CMTime& t,unsigned var,long trial,int usekey)
{
	if (Fail() || var>=nvars) return 0;
	float ret=0;
	long per = CMTime::Diff(t,beg,incunits,inclength);
	trial = (usekey && key) ? (key->At(trial)).Index() : trial;
	long index = trial*nvars*periodlength + per*nvars + var;
	/*
	if (index < filecount) {
		if (begindex<0 || index<begindex || index>=begindex+nvars) {
			file->clear();
			begindex = index-var;
			file->seekg(HeaderSize()+begindex*sizeof(float),ios::beg);
			file->read((wchar_t*)fbuffer, nvars*sizeof(float));
		}
		ret = fbuffer[var];
	}
	else if (array)
	*/
	ret = array->At(index);
	return ret;
}

/*
float CMSimulationArray::Sum(const CMTime& time,unsigned var,long trial,int timesteps)
{
	if (Fail() || var>=nvars || !timesteps) return 0;
	float ret=0;
	int incstep = (timesteps>0) ? 1 : -1;
	int i=0;
   int first=1;
	int issum = (GetVariableState(var) & CMVariableDescriptor::vdSum);
	for (CMTime tm(time);i!=timesteps;i+=incstep,tm.inc(incstep*inclength,incunits)) {
		float val = At(tm,var,trial,0);
		if (issum)
      	ret += val;
		else if (timesteps > 0 || first)
      	ret = val;
		first=0;
   }
	return ret;
}
*/

float CMSimulationArray::Aggregate(const CMTime& time,const CMString& var,long trial,int resolution)
{
   unsigned index = VariableIndex(var);
   float ret;
   Aggregate(time,trial,resolution,&index,&ret,1);
   return ret;
}

CMTime CMSimulationArray::Aggregate(const CMTime& time, long trial, int resolution, unsigned* varindex, float* results, unsigned n)
{
	if (Fail()) return time;

	sdebug << "Calling Aggregate: time=" << time << " trial = " << trial << " resolution = " << resolution << " n = " << n << endl;

	CMTime begt(time);
	int nstep = 1;
	int esc = 0;
	CMTime tm(time);
	CMTime ret;
	while (!esc) {
		ret = tm;
		for (unsigned i = 0; i < n; i++) {
			int issum = (GetVariableState(varindex[i]) & CMVariableDescriptor::vdSum);
			float val = At(tm, varindex[i], trial, 0);
			if (nstep == 1 || !issum) results[i] = val;
			else results[i] += val;
		}
		if (resolution < 0 && resolution >= incunits)
			break;
		tm.inc(inclength, incunits);
		if (resolution > 0)
			esc = (nstep >= resolution);
		else {
			switch (resolution) {
			case CM_YEAR: 	 esc = (tm.Year() != begt.Year()); break;
			case CM_MONTH:  esc = (tm.Month() != begt.Month()); break;
			case CM_WEEK: 	 esc = abs((long)tm - (long)begt) >= 7; break;
			case CM_DAY: 	 esc = ((ULONG)tm != (ULONG)begt); break;
			case CM_HOUR: 	 esc = (tm.Hour() != begt.Hour());	break;
			case CM_MINUTE: esc = (tm.Minute() != begt.Minute()); break;
			default: esc = 1; break;
			}
		}
		nstep++;
	}
	return ret;
}

CMTime CMSimulationArray::Summary(const CMTime& time,int resolution,unsigned* varindex,float* vmean,float* vstderr,float* vmin,float* vmax,unsigned n,long begtrial,long endtrial)
{
	unsigned i;

	begtrial = (begtrial>=0) ? begtrial : 0L;
	endtrial = (endtrial>=0 && endtrial<Trials()) ? endtrial : Trials()-1;
   CMTime ret(time);
   if (n==0)
   	return ret;
   float* scratch = new float[n];
	for (i=0;i<n;i++) {
   	if (vmean) vmean[i] = 0;
		if (vstderr) vstderr[i] = 0;
		if (vmin) vmin[i] = 1e10;
		if (vmax) vmax[i] = -1e10;
	}
   for (long trial=begtrial;trial<endtrial;trial++) {
   	ret = Aggregate(time,trial,resolution,varindex,scratch,n);
      for (i=0;i<n;i++) {
	   	if (vmean) vmean[i] += scratch[i];
			if (vstderr) vstderr[i] += scratch[i]*scratch[i];
			if (vmin && scratch[i]<vmin[i]) vmin[i] = scratch[i];
			if (vmin && scratch[i]>vmax[i]) vmax[i] = scratch[i];
      }
   }
	long count = (endtrial-begtrial+1);
	for (i=0;i<n;i++) {
   	if (vmean) vmean[i]/=count;
		if (vstderr) vstderr[i] = (float)sqrt(vstderr[i]/count - vmean[i]*vmean[i]);
   }
   delete [] scratch;
   return ret;
}

long CMSimulationArray::Trials() const
{
	if (Fail() || !nvars || !periodlength) return 0;
	return ( ((array ? array->Count() : 0)) / (periodlength*nvars) );
}

long CMSimulationArray::GetPeriod(CMTime& b,CMTime& e,short& units,short& length) const
{
	b = beg;
	e = end;
	units = incunits;
	length = inclength;
	return periodlength;
}

int CMSimulationArray::MakeKey(const CMTime& t,int var)
{
	if (var<0 || var>=nvars || t<beg || t>end || !array || Fail())
		return 0;

	if (key) delete key;
	long ntrials = Trials();
	key = new CMVSBigArray<pointer>(ntrials,1);
	for (long i=0;key && i<ntrials;i++)
		key->Add(pointer(At(t,var,i),i));
	key->Sort();
	return (key != 0);
}

/*
long CMSimulationArray::BinarySize() const
{
	long ret = HeaderSize() + filecount*sizeof(float);
	for (unsigned i=0;i<nvars && vardesc;i++)
		ret += vardesc->At(i).BinarySize();
	return ret;
}
*/
