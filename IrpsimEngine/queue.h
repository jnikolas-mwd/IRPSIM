// queue.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMValueQueue implements a queue of values, maintaining current and
// original states
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__CMQUEUE_H)
#define __CMQUEUE_H

#include "irp.h"
#include "cmtime.h"
#include "cmdefs.h"
#include "string.h"

class _IRPCLASS CMValueQueue
{
	struct _value {
		double original;
		double current;
	} *values;
	int size;   // number of elements in queue
	int index;  // index into queue corresponding to "last"
	CMTime last; // the time from which lags are derived
//	static int incunits; // increment units in months, years, etc.
//	static int inclength; // number of (e.g.) months in increment
   int get_offset(const CMTime& t,CMTIMEUNIT incunits,int inclength);
	void preprocess_queue(const CMTime& current,CMTIMEUNIT incunits,int inclength);
public:
	CMValueQueue(int sz);
	~CMValueQueue();
//	static void SetIncrement(int units,int length) {incunits=units;inclength=length;}
	void Reset();
	int ContainsAt(const CMTime&t, const CMTime& current,CMTIMEUNIT incunits,int inclength);
	double At(const CMTime& t,CMTIMEUNIT incunits,int inclength,int which);
	int AddAt(const CMTime& t,const CMTime& current,CMTIMEUNIT incunits,int inclength,double v);
};

#endif

