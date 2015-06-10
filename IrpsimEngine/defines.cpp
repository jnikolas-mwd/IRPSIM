// defines.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMDefine implements a user-defined value (string key, double value)
// CMDefines implements a dictionary of CMDefine members
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "defines.h"

CMPSmallArray<CMDefinition> CMDefinitions::definition_array;

CMDefinition* CMDefinitions::Find(const CMString& n,unsigned short* loc)
{
	if (loc) *loc=0;

	if (!definition_array.Count())
   	return 0;

	for (unsigned short i=0;i<definition_array.Count();i++) {
		if (n == definition_array[i]->GetName()) {
			if (loc) *loc=i;
			return definition_array[i];
		}
	}
	return 0;
}

double CMDefinitions::GetDefinition(const CMString& n)
{
	return IsDefined(n) ? GetValue(n) : _wtof(n.c_str());
}

CMDefinition* CMDefinitions::GetDefinition(unsigned n)
{
	return (n >= definition_array.Count()) ? 0 : definition_array.At(n);
}

const wchar_t* CMDefinitions::GetDefinitionName(unsigned n)
{
   return (n>=definition_array.Count()) ? 0 : definition_array.At(n)->GetName().c_str();
}

double CMDefinitions::GetDefinitionValue(unsigned n)
{
   return (n>=definition_array.Count()) ? 0 : definition_array.At(n)->GetValue();
}

