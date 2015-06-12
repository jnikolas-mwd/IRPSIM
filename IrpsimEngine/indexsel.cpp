// indexsel.cpp : implementation file
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
#include "StdAfx.h"
#include "indexsel.h"
#include "variable.h"

void CMIndexSelector::SetNumVariables(int nv)
{
	nvars=nv;
	sorted=0;
	array.Reset(1);
	variables.Reset(1);
	vector.Reset(1);
	smallest.Reset(1);
   largest.Reset(1);
	for (int i=0;i<nvars;i++) {
		smallest.AddAt(i,1e20);
		largest.AddAt(i,-1e20);
	}
}

void CMIndexSelector::SetVariable(int n,CMVariable* v)
{
	if (n>=0&&n<nvars) {
		variables.AddAt(n,v);
		vector.AddAt(n,0);
	}
}

void CMIndexSelector::Add(long index,int var,double val)
{
	if (var>=0 && var<nvars) {
		long base = index*(nvars+1);
		array.AddAt(base+var,val);
		array.AddAt(base+nvars,(double)index);
		if (val<smallest[var]) smallest[var] = val;
		if (val>largest[var])  largest[var] = val;
	}
}

double CMIndexSelector::GetValue(long index,int var)
{
	if (var<0 || var>=nvars)
		return 0;
	return array[index*(nvars+1)+var];
}

int CMIndexSelector::vector_is_greater_than(long pos)
{
	long base = pos*(nvars+1);

	for (int i=0;i<nvars;i++) {
		if (vector[i] > array[base+i])
			return 1;
		else if (vector[i] < array[base+i])
			return 0;
	}
	return 0;
}

int CMIndexSelector::vector_is_equal_to(long pos)
{
	long base = pos*(nvars+1);

	for (int i=0;i<nvars;i++)
		if (vector[i] != array[base+i])
			return 0;

	return 1;
}

long CMIndexSelector::closest_to_vector(long pos1,long pos2)
{
	long base1 = pos1*(nvars+1),base2 = pos2*(nvars+1);
	double d1=0,d2=0;

	for (int i=0;i<nvars && d1==0 && d2==0;i++) {
		d1 = array[base1+i] - vector[i];
		d2 = array[base2+i] - vector[i];
	}
	if (d1<0) d1 = -d1;
	if (d2<0) d2 = -d2;

	return (d1 < d2) ? pos1 : pos2;
}

int CMIndexSelector::switch_if_greater_than(long index1,long index2)
{
	long base1 = index1*(nvars+1),base2 = index2*(nvars+1);
	int greater=0;
   int i;

	for (i=0;i<nvars && !greater;i++) {
		if (array[base1+i] > array[base2+i])
			greater=1;
		else if (array[base1+i] < array[base2+i])
			break;
	}

	for (i=0;i<=nvars&&greater;i++) {  // switch values if greater
		double temp = array[base1+i];
		array[base1+i] = array[base2+i];
		array[base2+i] = temp;
	}

	return greater;
}

void CMIndexSelector::sort()
{
	long n = array.Count()/(nvars+1);
	int issorted = 0;

	while (!issorted && n > 0) {
		issorted = 1;
		for (long i=0;i<n-1;i++)
			if (switch_if_greater_than(i,i+1))
				issorted = 0;
		n--;
	}
	sorted = 1;
}

long CMIndexSelector::GetIndex(CMTimeMachine* t)
{
	long j;
	if (!sorted)
		sort();

	for (int i=0;i<nvars;i++) {
		vector[i] = variables[i] ? variables[i]->GetValue(t) : 0;
		if (vector[i] < smallest[i]) vector[i] = smallest[i];
		if (vector[i] > largest[i]) vector[i] = largest[i];
	}

	long bottom = 0;
	long top = array.Count()/(nvars+1)-1;

	if (!vector_is_greater_than(0))	return index_at(0);
	if (vector_is_greater_than(top))	return index_at(top);

	while (top - bottom > 12) {
		if (vector_is_equal_to(bottom)) return index_at(bottom);
		if (vector_is_equal_to(top))	  return index_at(top);
		long x = (1 + bottom + top)/2;
		if (vector_is_greater_than(x))
			bottom = x;
		else
			top = x;
	}

	for (j=bottom;j<=top && vector_is_greater_than(j);j++);

	return (j==0) ? index_at(0) : index_at(closest_to_vector(j,j-1));
}


