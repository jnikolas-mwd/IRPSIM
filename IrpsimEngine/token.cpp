// token.cpp : implementation file
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
#include "StdAfx.h"
#include "token.h"

CMTokenizer::CMTokenizer(const string& s)
{
	theString = &s;
	place = 0;
}

void CMTokenizer::Reset(const string& s)
{
	theString = &s;
	place = 0;
}

CMSubString CMTokenizer::operator()(const wchar_t* ws)
{
	const wchar_t* eos = theString->c_str() + theString->length();

	if (place==0)		// the first time through?
		place = theString->c_str();	// Initialize 'place'

	size_t extent;
	while (1) {
		if (place >= eos) return (*(string*)theString)(CM_NPOS,0);
		place += wcsspn(place, ws);
		extent = wcscspn(place, ws);
		if (extent) break;
		++place; // skip null
	}
	size_t start = (size_t)(place - theString->c_str());
	place += extent;		// Advance the placeholder

  return (*(string*)theString)(start,extent);
}

CMSubString CMTokenizer::operator()()
  { return operator()(L" \t\r\n"); }

