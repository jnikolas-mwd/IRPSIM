// irpobject.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMIrpObject is the virtual base class for all IRPSIM objects.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "irpobject.h"

CMIrpObject::CMIrpObject(const CMString& aName,int id) :
name(aName),
app_id(id)
{
}

CMIrpObject::CMIrpObject(const wchar_t* aName,int id) :
name(aName ? aName : L""),
app_id(id)
{
}

wistream& CMIrpObject::read(wistream& s)
{
	return s;
}

wostream& CMIrpObject::write(wostream& s)
{
	return s;
}

int CMIrpObject::is_less_than(const CMIrpObject& o2) const
{
	return name < o2.name;
}

int CMIrpObject::is_equal_to(const CMIrpObject& o2) const
{
	return name == o2.name;
}

const wchar_t* CMIrpObject::IsA()
{
	return L"";
}

wistream& operator >> (wistream& s, CMIrpObject& o)
{
	return o.read(s);
}

wostream& operator << (wostream& s, CMIrpObject& o)
{
	return o.write(s);
}

int operator < (const CMIrpObject& o1,const CMIrpObject& o2)
{
	return o1.is_less_than(o2);
}

int operator == (const CMIrpObject& o1,const CMIrpObject& o2)
{
	return o1.is_equal_to(o2);
}
