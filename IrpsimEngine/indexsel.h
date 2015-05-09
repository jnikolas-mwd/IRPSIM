// indexsel.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMIndexSelector implements a collection of realized simulation results
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
#include "smparray.h"
#include "smvarray.h"
#include "bgvarray.h"

class _IRPCLASS CMVariable;
class _IRPCLASS CMTimeMachine;

class _IRPCLASS CMIndexSelector
{
	int nvars;
	int sorted;

	class _IRPCLASS CMPSmallArray<CMVariable> variables;
	class _IRPCLASS CMVBigArray<double>   array;
	class _IRPCLASS CMVSmallArray<double> vector;    // vector of realized values
	class _IRPCLASS CMVSmallArray<double> smallest;  // smallest values
	class _IRPCLASS CMVSmallArray<double> largest;   // largest values
	
	int  vector_is_greater_than(long pos);
	int  vector_is_equal_to(long pos);
	long closest_to_vector(long pos1,long pos2);

	int  switch_if_greater_than(long index1,long index2);
	long index_at(long pos) {return (long)array[pos*(nvars+1)+nvars];}
	void sort();
public:
	CMIndexSelector(int nv=0) {SetNumVariables(nv);}
	void SetVariable(int n,CMVariable* v);
	void SetNumVariables(int nv);
	int  GetNumVariables() {return nvars;}
	void Add(long index,int var,double val);
	double GetValue(long index,int var);
	long GetIndex(CMTimeMachine* t);
};



