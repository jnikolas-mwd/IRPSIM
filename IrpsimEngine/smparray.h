// smparray.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Small array of pointers
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__SMPARRAY_H)
#define __SMPARRAY_H

#include "cmdefs.h"

template <class TP> class CMPSmallArray
{
protected:
	unsigned short count;
	unsigned short size;
	unsigned short delta;
	TP** array;
	TP** make_new_array(unsigned short& sz);
	void reset(int freemem,int destroy);
public:
	CMPSmallArray(unsigned short sz=0,unsigned short d=10);
	~CMPSmallArray();
//	CMPSmallArray<TP>& operator = (const CMPSmallArray<TP>& a);
	int operator == (const CMPSmallArray<TP>& a) const;
	void Reset(int freemem=0);
	void ResetAndDestroy(int freemem=0);
	int InsertAt(unsigned short pos,TP* val);
	void Exchange(unsigned short  i,unsigned short j);
	int AddAt(unsigned short pos,TP* val);
	int Add(TP* val);
	int Contains(const TP& val,unsigned short* loc=0);
	void Detach(const TP* loc,int destroy=0);
	void Detach(const TP& loc,int destroy=0);
	void DetachAt(unsigned short loc,int destroy=0);
	unsigned short Count() const;
	unsigned short Resize(unsigned short sz);
	TP* At(unsigned short n) const;
	TP* operator [] (unsigned short n) const;
	TP** Array() const;
};

template <class TP> class CMPSSmallArray : public CMPSmallArray<TP>
{
	unsigned short get_insert_position(const TP& val,int& entryexists) const;
   void sort_call(short unsigned p,short unsigned r);
public:
	CMPSSmallArray(unsigned short sz=0,unsigned short d=10);
	int  Find(const TP& val,unsigned short* loc=0);
	int  Insert(TP* val,int duplicate_allowed=0);  // insert while maintaining sort order
	int  Insert(const TP& val,int duplicate_allowed=0);  // insert while maintaining sort order
	void Sort();
//	void BSort();
};

#if defined (_CM_COMPILE_INSTANTIATE)
#include "smparray.cc"
#endif

#endif


