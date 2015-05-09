// accum.h : header file
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
#pragma once

#include "vardesc.h"
#include "timemach.h"
#include "cmdefs.h"
#include "bgvarray.h"
#include "smvarray.h"
#include "cmstring.h"
#include <fstream>

	class accumulator
	{
		double sum;
		double sumsq;
		double vmin;
		double vmax;
	public:
		accumulator() : sum(0), sumsq(0), vmin(1e20), vmax(-1e20) {}
		accumulator(const accumulator& a) :
			sum(a.sum), sumsq(a.sumsq), vmin(a.vmin), vmax(a.vmax)  {}
		accumulator& operator = (const accumulator& a)
		{
			sum = a.sum; sumsq = a.sumsq; vmin = a.vmin; vmax = a.vmax; return *this;
		}
		void Reset();
		int operator == (const accumulator& a)
		{
			return (sum == a.sum&&sumsq == a.sumsq&&vmin == a.vmin&&vmax == a.vmax);
		}
		accumulator& operator += (double val);
		double Mean(long count) const;
		double StdDev(long count) const;
		double Variance(long count) const;
		double Min() const { return (vmin == 1e20) ? 0 : vmin; }
		double Max() const { return (vmax == -1e20) ? 0 : vmax; }
		wostream& WriteBinary(wostream& s);
		wistream& ReadBinary(wistream& s);

		static int BinarySize() { return 4 * sizeof(double); }
	};

	inline wostream& operator << (wostream& s, accumulator& a)
	{
		return a.WriteBinary(s);
	}

	inline wistream& operator >> (wistream& s, accumulator& a)
	{
		return a.ReadBinary(s);
	}

	class _IRPCLASS CMAccumulatorArray
	{
		CMVBigArray<accumulator>* array;
		CMVSmallArray<CMVariableDescriptor>* vardesc;
		CMTIMEUNIT incunits;			// e.g. CMTime::months, CMTime::years
		short inclength;			// e.g. # of months, # of years in increment
		CMTime beg;          // beginning of period
		CMTime end;          // end of period
		unsigned short nvars;        // number of variables being recorded
		long	 periodlength; // length of period (e.g. # of months)
		short	 state;
		long	 count;
		unsigned get_variable_index(const CMString& name);
		enum { failbit = 0x0001 };
	public:
		CMAccumulatorArray();
		CMAccumulatorArray(const CMTimeMachine& t, unsigned nv);
		CMAccumulatorArray(const wchar_t* name);
		~CMAccumulatorArray();

		int   Fail() const { return (state & failbit); }
		void  Reset();
		void  AssignVariable(unsigned short var, const CMString& name, const CMString& type, int s)
		{
			if (vardesc) vardesc->AddAt(var, CMVariableDescriptor(name, type, s));
		}
		void  AddAt(const CMTime& t, unsigned var, double val);

		double Mean(const CMTime& t, unsigned var) const;
		double StdDev(const CMTime& t, unsigned var) const;
		double Variance(const CMTime& t, unsigned var) const;
		double Min(const CMTime& t, unsigned var) const;
		double Max(const CMTime& t, unsigned var) const;

		double Mean(const CMTime& t, const CMString& var) const	{ return Mean(t, VariableIndex(var)); }
		double StdDev(const CMTime& t, const CMString& var) const { return StdDev(t, VariableIndex(var)); }
		double Variance(const CMTime& t, const CMString& var)	const { return Variance(t, VariableIndex(var)); }
		double Min(const CMTime& t, const CMString& var) const { return Min(t, VariableIndex(var)); }
		double Max(const CMTime& t, const CMString& var) const { return Max(t, VariableIndex(var)); }

		long Trials() const { return (nvars&&periodlength) ? count / (periodlength*nvars) : 0; }
		unsigned Variables() const { return nvars; }
		CMTime BeginPeriod() const { return beg; }
		CMTime EndPeriod() const { return end; }
		long TimeSteps() const { return periodlength; }
		long GetPeriod(CMTime& b, CMTime& e, CMTIMEUNIT& units, int& length) const;
		CMString GetVariableName(unsigned n) const;
		CMString GetVariableType(unsigned n) const;
		int    GetVariableState(unsigned n) const;
		void   SetVariableState(unsigned n, int aState, BOOL action);
		unsigned VariableIndex(const CMString& name) const;
		wostream& WriteBinary(wostream& s);
		wistream& ReadBinary(wistream& s);

		long BinarySize() const;
	};

	inline wostream& operator << (wostream& s, CMAccumulatorArray& a)
	{
		return a.WriteBinary(s);
	}

	inline wistream& operator >> (wistream& s, CMAccumulatorArray& a)
	{
		return a.ReadBinary(s);
	}
