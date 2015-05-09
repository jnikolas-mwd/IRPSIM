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
#include "assoc.h"

void CMAssociations::AddAssociation(const CMString& aName,const CMString& val)
{
	unsigned short i=0;
	for (;i<Count();i++) {
		if (*At(i) == aName) {
      	At(i)->SetValue(val);
         return;
      }
   }
	Add(new CMStringValue<CMString>(aName,val) );
}

CMString CMAssociations::GetName(unsigned short n)
{
	if (n<Count())
		return (CMString)(*At(n));
   return CMString();
}

CMString CMAssociations::GetValue(unsigned short n)
{
	if (n<Count())
		return At(n)->Value();
   return CMString();
}

CMString CMAssociations::GetValue(const CMString& aName)
{
	int loc = GetAssociationIndex(aName);
   if (loc>=0) return At(loc)->Value();
   return CMString();
}

int CMAssociations::GetAssociationIndex(const CMString& aName)
{
	for (unsigned short i=0;i<Count();i++)
		if (*At(i) == aName)
			return (int) i;
	return -1;
}

int CMAssociations::GetAssociation(unsigned short n,CMString& s1,CMString& s2)
{
	if (n<Count()) {
		s1 = (CMString)(*At(n));
		s2 = At(n)->Value();
		return 1;
	}
	return 0;
}

int CMAssociations::IsAssociation(CMString& s1,CMString& s2)
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
