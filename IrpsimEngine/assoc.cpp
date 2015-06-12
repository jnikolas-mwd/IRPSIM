// assoc.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//

// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "assoc.h"

void CMAssociations::AddAssociation(const string& aName,const string& val)
{
	unsigned short i=0;
	for (;i<Count();i++) {
		if (*At(i) == aName) {
      	At(i)->SetValue(val);
         return;
      }
   }
	Add(new stringValue<string>(aName,val) );
}

string CMAssociations::GetName(unsigned short n)
{
	if (n<Count())
		return (string)(*At(n));
   return string();
}

string CMAssociations::GetValue(unsigned short n)
{
	if (n<Count())
		return At(n)->Value();
   return string();
}

string CMAssociations::GetValue(const string& aName)
{
	int loc = GetAssociationIndex(aName);
   if (loc>=0) return At(loc)->Value();
   return string();
}

int CMAssociations::GetAssociationIndex(const string& aName)
{
	for (unsigned short i=0;i<Count();i++)
		if (*At(i) == aName)
			return (int) i;
	return -1;
}

int CMAssociations::GetAssociation(unsigned short n,string& s1,string& s2)
{
	if (n<Count()) {
		s1 = (string)(*At(n));
		s2 = At(n)->Value();
		return 1;
	}
	return 0;
}

int CMAssociations::IsAssociation(string& s1,string& s2)
{
	for (unsigned short i=0;i<Count();i++)
		if (*At(i) == s1 && At(i)->Value() == s2)
			return 1;
	return 0;
}

void CMAssociations::Compact()
{
	if (Count()>0)
   	Resize(Count());
}
