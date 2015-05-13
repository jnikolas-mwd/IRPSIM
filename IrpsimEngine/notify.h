// notify.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMNotify class for handling notifications
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	10 April 2015
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "cmstring.h"
#include "cmtime.h"

typedef int(*CMNotifyDelegatePointer)(int ntype, const wchar_t* msg, int data);

class _IRPCLASS CMException {
	CMString what;
public:
	CMException(const CMString& str) { what = str; }
	CMString What() { return what; }
};

class _IRPCLASS CMNotifier {
public:
	typedef enum ntype { ERROR = 1, WARNING, LOG, LOGTIME, INFO, PROGRESS };
	static CMNotifyDelegatePointer SetDelegate(CMNotifyDelegatePointer p) { pDelegate = p; return pDelegate; }
	static int Notify(ntype type, const CMString& msg, int data = 0);
	static int Notify(ntype type);
private:
	static CMNotifyDelegatePointer pDelegate;
};
