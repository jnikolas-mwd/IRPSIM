// cmerror.cpp : implementation file
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
#include "StdAfx.h"
#include "cmerror.h"

//#include <fstream.h>
//ofstream sdebug("cmerror.deb");

CMVSmallArray<CMString> CMError::errors;
int CMError::maxwarn=10;
int CMError::maxerr=5;
int CMError::warncount=0;
int CMError::errcount=0;

void CMError::Reset()
{
	warncount=errcount=0;
   errors.Reset(1);
}

void CMError::ReportError(const CMString& err) throw (CMException)
{
	errors.Add(CMString(L"Error: " + err));
	const wchar_t* test = err.c_str();
   if (++errcount>=maxerr) {
   	errcount=0;
      throw (CMException(L"Too many errors"));
   }
}

void CMError::ReportWarning(const CMString& err) throw (CMException)
{
	errors.Add(CMString(L"Warning: " + err));
   if (++warncount>=maxwarn) {
   	warncount=0;
      throw (CMException(L"Too many warnings"));
   }
}

CMString CMError::LatestError()
{
	if (errors.Count()==0)
   	return CMString();
   return Error(errors.Count()-1);
}

CMString CMError::Error(unsigned short n)
{
	if (n<errors.Count())
		return errors[n];
	return CMString();
}



