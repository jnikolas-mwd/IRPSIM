// units.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMUnits implements a units translator
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
#include "string.h"
#include "cmdefs.h"
#include <iostream>

using namespace std;

class _IRPCLASS CMUnits
{
	friend _IRPFUNC wostream& operator << (wostream& s, CMUnits& u);
	friend _IRPFUNC wistream& operator >> (wistream& s, CMUnits& u);
	enum {af,g,cf,cm,hm,d};

	static const double factors[];
	static const wchar_t* list[];

	wchar_t   _type;
	string _units;
	int   _prec;
	double _xlatToLocalValue;
public:
	enum {Local,Global};
	enum {User,AcreFeet,Gallons,CubicFeet,CubicMeters,HectareMeters,Dollars};
	CMUnits();
	CMUnits(const string& def);
	CMUnits(const CMUnits& u) {Set(u._units);}
	void   Set(const string& def);
	void   Set(int aType,double aMult,int aPrec);
	double TranslateToLocal(double aVal)  const {return aVal*_xlatToLocalValue;}
	double TranslateToGlobal(double aVal) const {return aVal/_xlatToLocalValue;}
	int	 Type() const {return (int)_type;}
	double Multiplier() const;
	int    Precision() const {return _prec;}

	operator string () {return _units;}

	CMUnits& operator = (const CMUnits& u) {Set(u._units);return *this;}
	CMUnits& operator = (const string& def) {Set(def);return *this;}

	wostream& WriteBinary(wostream& s);
	wistream& ReadBinary(wistream& s);

	int BinaryLength();
};
