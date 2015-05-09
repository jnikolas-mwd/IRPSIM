// smvarray.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Small array of values
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__SMVARRAY_H)
#define __SMVARRAY_H

#include "cmdefs.h"

template <class T> class CMVSmallArray
{
protected:
	unsigned short count;
	unsigned short size;
	unsigned short delta;
	T* array;
	T* make_new_array(unsigned short& sz);
public:
	CMVSmallArray(unsigned short sz=0,unsigned short d=10);
	~CMVSmallArray();
	CMVSmallArray<T>& operator = (const CMVSmallArray<T>& a);
	int operator == (const CMVSmallArray<T>& a) const;
	void Reset(int freemem=0);
	int InsertAt(unsigned short pos,const T& val);
	void Exchange(unsigned short i,unsigned short j);
	int AddAt(unsigned short pos,const T& val);
	int Add(const T& val);
	int Contains(const T& val,unsigned short* loc=0);
	unsigned short Count() const;
	void Detach(const T& val);
	void DetachAt(unsigned short loc);
	unsigned short Resize(unsigned short sz);
	T& At(unsigned short n) const;
	T& operator [] (unsigned short n) const;
	T* Array() const;
};

template <class T> class CMVSSmallArray : public CMVSmallArray<T>
{
	unsigned short get_insert_position(const T& val,int& entryexists) const;
   void sort_call(unsigned short p,unsigned short r);
public:
	CMVSSmallArray(unsigned short sz=0,unsigned short d=10);
	int  Find(const T& val,unsigned short* loc=0);
	int  Insert(const T& val,int duplicate_allowed=0);  // insert while maintaining sort order
	void Sort();
};

#if defined (_CM_COMPILE_INSTANTIATE)
#include "smvarray.cc"
#endif

#endif


