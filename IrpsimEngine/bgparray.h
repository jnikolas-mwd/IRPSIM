// bgparray.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Large array of pointers
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

#include "cmdefs.h"

template <class TP> class CMPArrayNode
{
public:
   TP** array;
	long startindex;
   long size;
   int level;
   CMPArrayNode<TP> *next,*child;

	CMPArrayNode(long ndx,long n,int lvl);
	~CMPArrayNode();

	void Destroy(long n);
   long Resize(long n);
	int AddAt(long pos,TP* val);
   TP* At(long n) const;
};

template <class TP> class CMPBigArray
{
	CMPArrayNode<TP> *root,*left;
   CMPArrayNode<TP>* make_new_leaf(int level,long sz);
   int travlength;
   long delta;
   long count,size;
   CMPArrayNode<TP>* seek(long pos) const;
	void reset(int freemem,int destroy);
public:
	CMPBigArray(long sz=0,long d=100,int tlength=4);
	~CMPBigArray();
	CMPBigArray<TP>& operator = (const CMPBigArray<TP>& a);
	int operator == (const CMPBigArray<TP>& a) const;
	long Count() const;
   long Resize(long sz);
	void Reset(int freemem=0);
	void ResetAndDestroy(int freemem=0);
	void Exchange(long i,long j);
   TP* At(long n) const;
	TP* operator [] (long n) const;
	int  AddAt(long pos,TP* val);
	int  Add(TP* val);
	int  Contains(const TP& val,long* loc=0);
	void Detach(const TP* val,int destroy=0);
	void Detach(const TP& loc,int destroy=0);
	void DetachAt(long loc,int destroy=0);
};

template <class TP> class CMPSBigArray : public CMPBigArray<TP>
{
   void sort_call(long p,long r);
public:
	CMPSBigArray(long sz=0,long d=100,int tlength=4);
	void Sort();
};

#if defined (_CM_COMPILE_INSTANTIATE)
#include "bgparray.cc"
#endif