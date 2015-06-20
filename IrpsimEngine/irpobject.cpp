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
#include "StdAfx.h"
#include "irpobject.h"
#include "cmlib.h"

CMIrpObjectIterator::CMIrpObjectIterator(CMIrpObject* o) :
obj(o),
array(),
pos(-2)
{
}

CMIrpObjectIterator::~CMIrpObjectIterator()
{
	array.ResetAndDestroy(1);
}

int CMIrpObjectIterator::Reset()
{
	if (pos >= -1) {
		pos = 0;
		return 1;
	}
	return 0;
}

const wchar_t* CMIrpObjectIterator::GetNext()
{
	const wchar_t* ret = 0;
	if (!obj->IsIterating()) {
		obj->SetIterating(true);
		if (pos == -2) {
			if ((ret = get_next()) != 0)
				array.Insert(ret);
			else
				pos = -1;
		}
		else if (pos>-1)
			ret = (pos<array.Count()) ? array[pos++]->c_str() : 0;
		obj->SetIterating(false);
	}
	return ret;
}

CMIrpObject::CMIrpObject(const CMString& n, int id) :
app_id(id)
{
	//name.to_lower();
	name = stripends(n);
	if (name.length() && name[0] == L'#')
		name = name.substr(1, name.length() - 1);
}


/*
CMIrpObject::CMIrpObject(const wchar_t* aName,int id) :
name(aName ? aName : L""),
app_id(id)
{
}
*/

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
