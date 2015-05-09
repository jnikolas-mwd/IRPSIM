// bgvarray.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Large array of values
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

template <class T> class CMVArrayNode
{
	static T dummy;
public:
   T* array;
	long startindex;
   int level;
   long size;
   CMVArrayNode<T> *next,*child;

	CMVArrayNode(long ndx,long n,int lvl);
	~CMVArrayNode();
	int AddAt(long pos,const T& val);
   T& At(long n) const;
   long Resize(long n);
};

template <class T> class CMVBigArray
{
	CMVArrayNode<T> *root,*left;
   CMVArrayNode<T>* make_new_leaf(int level,long sz);
   int travlength;
   long delta;
   long count,size;
   CMVArrayNode<T>* seek(long pos) const;
public:
	CMVBigArray(long sz=0,long d=100,int tlength=4);
	~CMVBigArray();
	CMVBigArray<T>& operator = (const CMVBigArray<T>& a);
	int operator == (const CMVBigArray<T>& a) const;
	long Count() const;
   long Resize(long sz);
	void Reset(int freemem=0);
	void Exchange(long i,long j);
	int AddAt(long pos,const T& val);
	int Add(const T& val);
   T& At(long n) const;
	T& operator [] (long n) const;
	int Contains(const T& val,long* loc=0);
	void Detach(const T& loc);
	void DetachAt(long loc);
};

template <class T> class CMVSBigArray : public CMVBigArray<T>
{
   void sort_call(long p,long r);
public:
	CMVSBigArray(long sz=0,long d=100,int tlength=4);
	void Sort();
};

#if defined (_CM_COMPILE_INSTANTIATE)
#include "bgvarray.cc"
#endif