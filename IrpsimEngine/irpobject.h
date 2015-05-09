// irpobject.h : header file
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
#pragma once

#include "irp.h"
#include "cmstring.h"
//#include <iostream.h>
#include <iostream>
using namespace std;


class _IRPCLASS CMIrpObject
{
	friend int _IRPFUNC operator < (const CMIrpObject& o1,const CMIrpObject& o2);
	friend int _IRPFUNC operator == (const CMIrpObject& o1,const CMIrpObject& o2);
	
	friend _IRPFUNC wostream&  operator << (wostream& s, CMIrpObject& o);
	friend _IRPFUNC wistream&  operator >> (wistream& s, CMIrpObject& o);

	// Data elements ***************
protected:
	CMString name;
	int app_id;
	
	virtual wistream& read(wistream& s);
	virtual wostream& write(wostream& s);
	virtual int is_less_than(const CMIrpObject& o2) const;
	virtual int is_equal_to(const CMIrpObject& o2) const;
public:
	CMIrpObject(const wchar_t* aName = NULL, int id = -1);
	CMIrpObject(const CMString& aName,int id=-1);

    void  SetApplicationId(int id) {app_id=id;}
	int   GetApplicationId() {return app_id;}
	const CMString& GetName() const {return name;}
	virtual const wchar_t* IsA();
};
