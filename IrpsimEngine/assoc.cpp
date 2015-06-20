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

void CMAssociations::AddAssociation(const CMString& aName,const CMString& val)
{
	for (size_t i=0 ;i<_associations->Count(); i++) {
		if (_associations->At(i)->name == aName) {
			_associations->At(i)->value = val;
         return;
      }
   }
	//_associations->Add(new CMStringValue<CMString>(aName.c_str(),val.c_str()) );
	_associations->Add(new NameValue(aName, val));
}

CMString CMAssociations::GetName(unsigned short n)
{
	if (n < _associations->Count())
		return _associations->At(n)->name;

	return L"";
}

CMString CMAssociations::GetValue(unsigned short n)
{
	if (n < _associations->Count())
		return _associations->At(n)->value;
	return L"";
}

CMString CMAssociations::GetValue(const CMString& aName)
{
	int loc = GetAssociationIndex(aName);
	if (loc >= 0) return _associations->At(loc)->value;
   return L"";
}

int CMAssociations::GetAssociationIndex(const CMString& aName)
{
	for (unsigned short i = 0; i<_associations->Count(); i++)
		if (_associations->At(i)->name == aName)
			return (int) i;
	return -1;
}

int CMAssociations::GetAssociation(unsigned short n,CMString& s1,CMString& s2)
{
	if (n<_associations->Count()) {
		s1 = _associations->At(n)->name;
		s2 = _associations->At(n)->value;
		return 1;
	}
	return 0;
}

int CMAssociations::IsAssociation(CMString& s1,CMString& s2)
{
	for (unsigned short i = 0; i<_associations->Count(); i++)
		if (_associations->At(i)->name == s1 && _associations->At(i)->value == s2)
			return 1;
	return 0;
}

void CMAssociations::Compact()
{
	if (_associations->Count()>0)
		_associations->Resize(_associations->Count());
}
