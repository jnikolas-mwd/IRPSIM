// strval.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// stringValue implements a string-value pair, with value of type TP
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

#include "string.h"

template <class T> class stringValue : public string
{
	T value;
public:
	stringValue() : string() , value() {}
	stringValue(const string& str, const T& val) : string(str) , value(val) {}
	stringValue(const wchar_t* str, const T& val) : string(str), value(val) {}
	stringValue(const stringValue<T>& s) : string((const string)s) , value(s.value) {}
	stringValue<T>& operator = (const stringValue<T>& s)
		{string::operator = (s); value=s.value; return *this;}
	T& Value() {return value;}
   void SetValue(const T& v) {value=v;}
};

#endif

