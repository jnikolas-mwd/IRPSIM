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
#include "cmdefs.h"
#include "arraydic.h"
//#include <iostream.h>
#include <iostream>
using namespace std;

class _IRPCLASS CMOption {
	CMString name;
	CMString value;
public:
	CMOption() : name() , value() {}
	CMOption(const CMString& n,const CMString& v);
	CMOption(const CMOption& o) : name(o.name) , value(o.value) {}
	CMOption& operator = (const CMOption& o) {name=o.name;value=o.value;return *this;}
	int operator == (const CMOption& o) {return name==o.name;}
	int operator < (const CMOption& o) {return name<o.name;}
	CMString& GetName() {return name;}
	CMString& GetValue() {return value;}
	void SetValue(const CMString& v);

	wostream& WriteBinary(wostream& s);
	wistream& ReadBinary(wistream& s);
	short    BinarySize();
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
public:
	static struct _def { const wchar_t* name; const wchar_t* value; } defaults[];
	CMOptions();
	CMOptions(const CMOptions& op);
	~CMOptions() {options.ResetAndDestroy(1);}

	void   SetDefaults();

	CMString GetOption(const CMString& option);
	const  wchar_t* GetOptionString(const CMString& option);
	double GetOptionDouble(const CMString& option);
	short	 GetOptionInt(const CMString& option);
	long	 GetOptionLong(const CMString& option);

	void   SetOption(const CMString& line);
	void   SetOption(const CMString& name,const CMString& value);
	void   SetOption(const CMString& name,double option);

	unsigned  Count() const {return options.Count();}
	CMOption* At(unsigned short n) const {return options.At(n);}

    CMOptions& operator = (const CMOptions& op);
	wostream& WriteBinary(wostream& s);
	wistream& ReadBinary(wistream& s);
	long     BinarySize();
};

_IRPFUNC wostream& operator << (wostream& s, CMOptions& o);
_IRPFUNC wistream& operator >> (wistream& s, CMOptions& o);

#endif

