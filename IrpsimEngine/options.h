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
//	string name;
	string value;
public:
	CMOption() : CMIrpObject() , value(L"") {}
	CMOption(const string& n, const string& v, int app_id) : CMIrpObject(n, app_id) { SetValue(v); }
	CMOption(const CMOption& o) : value(o.value) {this->name = o.name; this->app_id = o.app_id; }
	CMOption& operator = (const CMOption& o) { name = o.name; value = o.value; app_id = o.app_id; return *this; }
//	int operator == (const CMOption& o) {return name==o.name;}
//	int operator < (const CMOption& o) {return name<o.name;}
//	string& GetName() {return name;}
	const string& GetValue() const {return value;}
	void SetValue(const string& v);
	void SetValueAndAppId(const string& v, int id) { SetValue(v); app_id = id; }

//	wostream& WriteBinary(wostream& s);
//	wistream& ReadBinary(wistream& s);
//	short    BinarySize();
protected:
	virtual const wchar_t* IsA() { return L"CMOption"; }
};


//template class _IRPCLASS CMArrayDictionary<CMOption>;
typedef class _IRPCLASS CMArrayDictionary<CMOption> CMOPTIONARRAYDICTIONARY;

class _IRPCLASS CMOptions
{
	friend class CMOptionIterator;
	friend _IRPFUNC wostream& operator << (wostream& s, CMOptions& o);
	friend _IRPFUNC wistream& operator >> (wistream& s, CMOptions& o);
//	CMArrayDictionary<CMOption> options;
	CMOPTIONARRAYDICTIONARY options;
	short maxwidth;
	int app_id = -1;
	long app_index = 0;
public:
	static struct _def { const wchar_t* name; const wchar_t* value; } defaults[];
	CMOptions();
	CMOptions(const CMOptions& op);
	~CMOptions() {options.ResetAndDestroy(1);}

	void   SetApplicationId(int id) { this->app_id = id; }
	int	   GetApplicationId() { return this->app_id; }
	void   SetApplicationIndex(long index) { this->app_index = index; }
	int	   GetApplicationIndex() { return this->app_id; }
	
	void   SetDefaults();

	string GetOption(const string& option);
	const  wchar_t* GetOptionString(const string& option);
	double GetOptionDouble(const string& option);
	short	 GetOptionInt(const string& option);
	long	 GetOptionLong(const string& option);

	CMOption* SetOption(const string& line, int app_id);
	CMOption* SetOption(const string& name, const string& value, int app_id);
	CMOption* SetOption(const string& name, double option, int app_id);
	CMOption* SetOption(const string& name, const string& value) { return SetOption(name, value, -1); }
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

