// token.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMTokenizer implements a basic lexical string analyzer
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__CMTOKEN_H)
#define __CMTOKEN_H

#include "irp.h"
#include "cmdefs.h"
#include "cmstring.h"

class _IRPCLASS CMTokenizer
{
	CMString  theString;
//	const	CMString  theString;
	const wchar_t*		place;
public:
	CMTokenizer(const CMString& s);	// Construct to lex a string

  // Advance to next token, delimited by s:
	CMSubString operator()(const wchar_t* s);
	CMSubString operator()(); // { return operator()(" \t\n"); }
    void Reset(const CMString& s);
};

#endif
