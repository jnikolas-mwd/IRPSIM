// accum.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// accumulator class maintains basic summary statistics of a variable over
// the course of a simulation trial.
// CMAccumulatorArray implements an array of accumulators.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "accum.h"
#include "cmlib.h"
#include <math.h>

//#include <fstream.h>
//static ofstream sdebug("accum.deb");

	void accumulator::Reset()
	{
		sum = sumsq = 0;
		vmin = 1e20;
		vmax = -1e20;
	}

	accumulator& accumulator::operator += (double val)
	{
		sum += val;
		sumsq += (val*val);
		if (val < vmin) vmin = val;
		if (val > vmax) vmax = val;
		return *this;
	}

	double accumulator::Mean(long count) const
	{
		return (count > 0) ? (sum / count) : 0;
	}

	double accumulator::Variance(long count) const
	{
		double val = (count > 0) ? (sumsq / count - (sum / count)*(sum / count)) : 0;
		return (val < 0) ? 0 : val;
	}

	double accumulator::StdDev(long count) const
	{
		return sqrt(Variance(count));
	}

	wostream& accumulator::WriteBinary(wostream& s)
	{
		s.write((const wchar_t*)&sum, sizeof(sum));
		s.write((const wchar_t*)&sumsq, sizeof(sumsq));
		s.write((const wchar_t*)&vmin, sizeof(vmin));
		s.write((const wchar_t*)&vmax, sizeof(vmax));
		return s;
	}

	wistream& accumulator::ReadBinary(wistream& s)
	{
		s.read((wchar_t*)&sum, sizeof(sum));
		s.read((wchar_t*)&sumsq, sizeof(sumsq));
		s.read((wchar_t*)&vmin, sizeof(vmin));
		s.read((wchar_t*)&vmax, sizeof(vmax));
		return s;
	}

	CMAccumulatorArray::CMAccumulatorArray() :
		beg(),
		end(),
		incunits(CM_MONTH),
		inclength(1),
		nvars(0),
		count(0),
		array(0),
		vardesc(0),
		state(0)
	{
	}

	CMAccumulatorArray::CMAccumulatorArray(const CMTimeMachine& t, unsigned nv) :
		beg(t[0].Begin()),
		end(t[0].End()),
		incunits(t.IncUnits()),
		inclength(t.IncLength()),
		nvars(nv),
		count(0),
		array(0),
		vardesc(0),
		state(0)
	{
		periodlength = CMTime::Diff(end, beg, incunits, inclength) + 1;
		if (end<beg)
			state |= failbit;
		else if (periodlength>0 && nvars > 0){
			long sz = periodlength*nvars;
			array = new CMVBigArray<accumulator>(sz);
			for (long i = 0; i < sz; i++)
				array->Add(accumulator());
			vardesc = new CMVSmallArray<CMVariableDescriptor>(nvars);
		}
	}

	CMAccumulatorArray::CMAccumulatorArray(const wchar_t* name) :
		array(0),
		vardesc(0),
		state(0)
	{
		wifstream	file(name, ios::in | IOS_BINARY);
		if (file.fail())
			state |= failbit;
		else {
			ReadBinary(file);
			if (end < beg || count < 0)
				state |= failbit;
		}
	}

	void CMAccumulatorArray::Reset()
	{
		for (long i = 0; array && i < array->Count(); i++)
			array->At(i).Reset();
		count = 0L;
	}

	CMAccumulatorArray::~CMAccumulatorArray()
	{
		delete array;
		delete vardesc;
	}

	CMString CMAccumulatorArray::GetVariableName(unsigned n) const
	{
		if (n < vardesc->Count())
			return vardesc->At(n).Name();
		return CMString();
	}

	CMString CMAccumulatorArray::GetVariableType(unsigned n) const
	{
		if (n < vardesc->Count())
			return vardesc->At(n).Type();
		return CMString();
	}

	int CMAccumulatorArray::GetVariableState(unsigned n) const
	{
		return (n < vardesc->Count()) ? vardesc->At(n).GetState() : 0;
	}

	void CMAccumulatorArray::SetVariableState(unsigned n, int aState, BOOL action)
	{
		if (n < vardesc->Count())
			vardesc->At(n).SetState(aState, action);
	}

	void CMAccumulatorArray::AddAt(const CMTime& t, unsigned var, double val)
	{
		if (!array || Fail()) return;
		long per = CMTime::Diff(t, beg, incunits, inclength);
		array->At(per*nvars + var) += val;
		count++;
	}

	double CMAccumulatorArray::Mean(const CMTime& t, unsigned var) const
	{
		if (!array) return 0;
		long per = CMTime::Diff(t, beg, incunits, inclength);
		return array->At(per*nvars + var).Mean(Trials());
	}

	double CMAccumulatorArray::StdDev(const CMTime& t, unsigned var) const
	{
		if (!array) return 0;
		long per = CMTime::Diff(t, beg, incunits, inclength);
		return array->At(per*nvars + var).StdDev(Trials());
	}

	double CMAccumulatorArray::Variance(const CMTime& t, unsigned var) const
	{
		if (!array) return 0;
		long per = CMTime::Diff(t, beg, incunits, inclength);
		return array->At(per*nvars + var).Variance(Trials());
	}

	double CMAccumulatorArray::Min(const CMTime& t, unsigned var) const
	{
		if (!array) return 0;
		long per = CMTime::Diff(t, beg, incunits, inclength);
		return array->At(per*nvars + var).Min();
	}

	double CMAccumulatorArray::Max(const CMTime& t, unsigned var) const
	{
		if (!array) return 0;
		long per = CMTime::Diff(t, beg, incunits, inclength);
		return array->At(per*nvars + var).Max();
	}

	long CMAccumulatorArray::GetPeriod(CMTime& b, CMTime& e, CMTIMEUNIT& units, int& length) const
	{
		b = beg;
		e = end;
		units = incunits;
		length = inclength;
		return periodlength;
	}

	unsigned CMAccumulatorArray::VariableIndex(const CMString& name) const
	{
		for (unsigned i = 0; i < vardesc->Count(); i++)
		if (vardesc->At(i) == name)
			return i;
		return nvars;
	}

	wostream& CMAccumulatorArray::WriteBinary(wostream& s)
	{
		s.write((const wchar_t*)&count, sizeof(count));
		s.write((const wchar_t*)&nvars, sizeof(nvars));
		beg.Write(s, 1);
		end.Write(s, 1);
		s.write((const wchar_t*)&incunits, sizeof(incunits));
		s.write((const wchar_t*)&inclength, sizeof(inclength));
		for (unsigned i = 0; i < nvars; i++)
			vardesc->At(i).WriteBinary(s);
		for (long j = 0; j < periodlength*nvars; j++)
			s << array->At(j);
		return s;
	}

	wistream& CMAccumulatorArray::ReadBinary(wistream& s)
	{
		if (array) delete array;
		if (vardesc) delete vardesc;
		array = 0;
		vardesc = 0;
		s.read((wchar_t*)&count, sizeof(count));
		s.read((wchar_t*)&nvars, sizeof(nvars));
		beg.Read(s, 1);
		end.Read(s, 1);
		s.read((wchar_t*)&incunits, sizeof(incunits));
		s.read((wchar_t*)&inclength, sizeof(inclength));
		periodlength = CMTime::Diff(end, beg, incunits, inclength) + 1;
		if (periodlength > 0 && nvars > 0) {
			array = new CMVBigArray<accumulator>(periodlength*nvars);
			vardesc = new CMVSmallArray<CMVariableDescriptor>(nvars);
			for (unsigned i = 0; i < nvars; i++) {
				CMVariableDescriptor vd;
				vd.ReadBinary(s);
				vardesc->AddAt(i, vd);
			}
			for (long j = 0; j < periodlength*nvars; j++)
				s >> array->At(j);
		}
		return s;
	}

	long CMAccumulatorArray::BinarySize() const
	{
		long ret = 2 * CMTime::BinarySize() + 3 * sizeof(short)+sizeof(long);
		for (unsigned i = 0; i < nvars; i++)
			ret += vardesc->At(i).BinarySize();
		return ret + periodlength*nvars*accumulator::BinarySize();
	}
