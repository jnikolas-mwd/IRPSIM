// strval.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMStringValue implements a string-value pair, with value of type TP
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__STRVAL_H)
#define __STRVAL_H

#include "cmstring.h"

template <class T> class CMStringValue : public CMString
{
	T value;
public:
	CMStringValue() : CMString() , value() {}
	CMStringValue(const CMString& str, const T& val) : CMString(str) , value(val) {}
	CMStringValue(const wchar_t* str, const T& val) : CMString(str), value(val) {}
	CMStringValue(const CMStringValue<T>& s) : CMString((const CMString)s) , value(s.value) {}
	CMStringValue<T>& operator = (const CMStringValue<T>& s)
		{CMString::operator = (s); value=s.value; return *this;}
	T& Value() {return value;}
   void SetValue(const T& v) {value=v;}
};

#endif

