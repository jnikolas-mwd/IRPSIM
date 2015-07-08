// options.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMOption implemets an option (string-value pair) used to control a
// simulation.
// CMOptions implements a collection of options
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__OPTIONS_H)
#define __OPTIONS_H

#include "irp.h"
#include "irpobject.h"
#include "cmdefs.h"
#include "arraydic.h"
//#include <iostream.h>
#include <iostream>
using namespace std;

class _IRPCLASS CMOption : public CMIrpObject {
//	CMString name;
	CMString value;
public:
	CMOption() : CMIrpObject() , value(L"") {}
	CMOption(const CMString& n, const CMString& v, int app_id);
	CMOption(const CMOption& o) : CMIrpObject(o.name) { value= o.value; app_id = o.app_id; }
	CMOption& operator = (const CMOption& o) { name = o.name; value = o.value; app_id = o.app_id; return *this; }
//	int operator == (const CMOption& o) {return name==o.name;}
//	int operator < (const CMOption& o) {return name<o.name;}
//	CMString& GetName() {return name;}
	const CMString& GetValue() const {return value;}
	void SetValue(const CMString& v);
	void SetValueAndAppId(const CMString& v, int id) { SetValue(v); app_id = id; }

//	wostream& WriteBinary(wostream& s);
//	wistream& ReadBinary(wistream& s);
//	short    BinarySize();
protected:
	virtual const wchar_t* IsA() { return L"CMOption"; }
};

class _IRPCLASS CMOptions
{
	friend class CMOptionIterator;
	friend _IRPFUNC wostream& operator << (wostream& s, CMOptions& o);
	friend _IRPFUNC wistream& operator >> (wistream& s, CMOptions& o);
	CMArrayDictionary<CMOption> options;
	short maxwidth;
	int app_id = -1;
	long app_index = 0;
public:
	CMOptions();
	CMOptions(const CMOptions& op);
	~CMOptions() {options.ResetAndDestroy(1);}

	void   SetApplicationId(int id) { this->app_id = id; }
	int	   GetApplicationId() { return this->app_id; }
	void   SetApplicationIndex(long index) { this->app_index = index; }
	int	   GetApplicationIndex() { return this->app_id; }
	
	CMString GetOption(const CMString& option);
	const  wchar_t* GetOptionString(const CMString& option);
	double GetOptionDouble(const CMString& option);
	short	 GetOptionInt(const CMString& option);
	long	 GetOptionLong(const CMString& option);

	CMOption* SetOption(const CMString& line, int app_id);
	CMOption* SetOption(const CMString& name, const CMString& value, int app_id);
	CMOption* SetOption(const CMString& name, double option, int app_id);
	CMOption* SetOption(const CMString& name, const CMString& value) { return SetOption(name, value, -1); }
	CMOption* SetOption(const CMOption& op) { return SetOption(op.GetName(), op.GetValue(), op.GetApplicationId()); }

	unsigned  Count() const {return options.Count();}
	CMOption* At(unsigned short n) const {return options.At(n);}

    CMOptions& operator = (const CMOptions& op);
	//wostream& WriteBinary(wostream& s);
	//wistream& ReadBinary(wistream& s);
	//long     BinarySize();
};

_IRPFUNC wostream& operator << (wostream& s, CMOptions& o);
_IRPFUNC wistream& operator >> (wistream& s, CMOptions& o);

#endif

