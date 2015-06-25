// cmlib.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Library of utility functions
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
#include "cmstring.h"
#include "cmdefs.h"
#include <iostream>

using namespace std;

//long  _IRPFUNC round(double val);
//wchar_t* _IRPFUNC newstring(const wchar_t* str);
int	_IRPFUNC skipwhite(wchar_t*& ptr);
int	_IRPFUNC isnumber(const wchar_t* str);
int	_IRPFUNC isnumber(const wchar_t ch); // 1 if is digit or '.', 0 otherwise
int	_IRPFUNC isvariablename(const wchar_t* str); // is this a legal variable name?
int	_IRPFUNC isalgop(const wchar_t* str);  // is this an algebraic operation?
int	_IRPFUNC contains(const wchar_t* str, const wchar_t**list, int n = 10000, int case_sen = 0);
int	_IRPFUNC writestringbinary(const CMString& s,wostream& os);
int	_IRPFUNC readstringbinary(CMString& s,wistream& os);
CMString _IRPFUNC readstringbinary(wistream& is);
int   _IRPFUNC stringbinarylength(const CMString& s);
int	_IRPFUNC sign(double val);
wchar_t* roundstring(wchar_t* str, int precision);
CMString _IRPFUNC stripends(const CMString& s);
//CMString _IRPFUNC createtempfile(const wchar_t* prefix, const wchar_t* path);
int	_IRPFUNC removefile(const CMString& filename);
CMString _IRPFUNC getfileinfo(const CMString& file);
CMString _IRPFUNC strippath(const CMString& file);
CMString _IRPFUNC extractpath(const CMString& file);
//CMString _IRPFUNC getfullpathname(const wchar_t* name);
CMString _IRPFUNC getrelativepath(const wchar_t* master, const wchar_t* slave);
CMString _IRPFUNC getabsolutepath(const wchar_t* master, const wchar_t* slave);

