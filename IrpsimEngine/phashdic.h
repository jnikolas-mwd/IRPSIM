// phashdic.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Template class CMPHashDictionary implements a hash of <TP> objects with 
// a CMString key
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__PHASHDIC_H)
#define __PHASHDIC_H

#include "cmstring.h"
#include "smparray.h"

template <class TP> class CMPHashDictionaryIterator;

template <class TP> class CMPHashDictionary
{
	friend class CMPHashDictionaryIterator<TP>;
	short arraysize;
	CMPSmallArray<TP>* array;
	int csflag;  // case sensitive flag
public:
	CMPHashDictionary(unsigned short sz=64,int flag=0);
	~CMPHashDictionary();
	void Add(TP* val);
	void Reset(int destroy=0);
	int  Detach(const CMString& val,int destroy=0);
    long Count();
	TP* Find(const CMString& val);
};

template <class TP> class CMPHashDictionaryIterator
{
	const CMPHashDictionary<TP>* dictionary;
	unsigned short which;
	unsigned short pos;
public:
	CMPHashDictionaryIterator(const CMPHashDictionary<TP>* d);
	void Reset();
	TP* operator ()();
};

#if defined (_CM_COMPILE_INSTANTIATE)
#include "phashdic.cc"
#endif

#endif


