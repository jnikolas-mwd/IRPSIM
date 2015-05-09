// vardesc.cpp : source file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVariableDescriptor maintains the name, type and state of a variable.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "vardesc.h"

#include "cmlib.h"

wostream& CMVariableDescriptor::WriteBinary(wostream& s)
{
	s.write((const wchar_t*)&state, sizeof(state));
	writestringbinary(name,s);
	writestringbinary(type,s);
	return s;
}

wistream& CMVariableDescriptor::ReadBinary(wistream& s)
{
	s.read((wchar_t*)&state, sizeof(state));
	readstringbinary(name,s);
	readstringbinary(type,s);
	return s;
}

int CMVariableDescriptor::BinarySize()
{
	return sizeof(state) + stringbinarylength(name) + stringbinarylength(type);
}


