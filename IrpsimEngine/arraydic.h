// arraydic.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Defined Array Dictionary template of type TP
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

#include "cmstring.h"
#include "cmdefs.h"

template <class TP> class CMArrayDictionary
{
	unsigned short count;
	unsigned short size;
	unsigned short delta;
	TP** array;
	TP** make_new_array(unsigned short& sz);
	void reset(int freemem,int destroy);
	int issorted;
	int addat(unsigned short pos,TP* val);
	unsigned short resize(unsigned short sz);
public:
	CMArrayDictionary(unsigned short sz=0,unsigned short d=10);
	~CMArrayDictionary();
	CMArrayDictionary<TP>& operator = (const CMArrayDictionary<TP>& a);
	int operator == (const CMArrayDictionary<TP>& a);
	void Reset(int freemem=0);
	void ResetAndDestroy(int freemem=0);
	int Add(TP* val);
	void Detach(const CMString& val,int destroy=0);
	unsigned short Count() const;
	TP* At(unsigned short n) const;
	TP* operator [] (unsigned short n) const;
	TP* Find(const CMString& val,unsigned short* loc=0);
	void Sort();
};

#if defined (_CM_COMPILE_INSTANTIATE)
#include "arraydic.cc"
#endif


