// cmerror.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMError exception handling class
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
#include "string.h"
#include "smvarray.h"

#pragma warning( disable : 4290 ) 

class _IRPCLASS CMException {
	string what;
public:
	CMException(const string& str) {what=str;}
    string What() {return what;}
};

class _IRPCLASS CMError {
	static class _IRPCLASS CMVSmallArray<string> errors;
   static int maxwarn;
	static int warncount;
   static int maxerr;
	static int errcount;
public:
	static void ReportError(const string& err) throw (CMException);
	static void ReportWarning(const string& err) throw (CMException);
	static void Reset();
	static unsigned short Errors() {return errors.Count();}
	static string Error(unsigned short n);
	static string LatestError();
   static void SetExceptionTriggers(int err,int warn) {maxerr=err;maxwarn=warn;}
};