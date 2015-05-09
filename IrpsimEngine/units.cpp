// units.cpp : implementation file
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
#include <stdio.h>
#include <stdlib.h>
#include "units.h"
#include "token.h"
#include "cmlib.h"

const double CMUnits::factors[] = {1,1,325900.0,43560.0,1233.64486,0.12335,1,1};
const wchar_t* CMUnits::list[] = { L"AF", L"G", L"CF", L"CM", L"HM", L"D", 0 };

CMUnits::CMUnits() : _units(),_xlatToLocalValue(1),_type(User)
{
}

CMUnits::CMUnits(const CMString& def) : _units()
{
	Set(def);
}

void CMUnits::Set(const CMString& def)
{
	_units.resize(0);
	_xlatToLocalValue=1;
	_type=User;
	_prec = 0;

	if (!def.length())
		return;

	_units.resize(0);
	_units = def;
	_units = _units.strip(CMString::Both);

	CMTokenizer next(_units);
	CMString unitstr = next(L"Pp");
	CMString precstr = next(L"Pp");

	_prec = _wtoi(precstr.c_str());
	if (_prec<0) _prec=-_prec;
	if (_prec>10) _prec=10;

	if (isnumber(unitstr.c_str())) {
		_type = User;
		_xlatToLocalValue = _wtof(unitstr.c_str());
		if (_xlatToLocalValue <= 0) _xlatToLocalValue = 1;
		return;
	}

	next.Reset(unitstr);

	CMString prefix  = next(L" \tXx");
	CMString multstr = next(L" \tXx");

	switch (contains(prefix.c_str(),list)) {
		case af: _type = AcreFeet; break;
		case g:	_xlatToLocalValue = factors[Gallons];_type = Gallons;break;
		case cf:	_xlatToLocalValue = factors[CubicFeet];_type = CubicFeet;break;
		case cm:	_xlatToLocalValue = factors[CubicMeters];_type = CubicMeters;break;
		case hm: _xlatToLocalValue = factors[HectareMeters];_type=HectareMeters;break;
		case d:  _type = Dollars; break;
		default: _type = User;break;
	}

	if (!multstr.is_null()) {
		double mult = _wtof(multstr.c_str());
		if (mult != 0)	_xlatToLocalValue /= mult;
	}
}

void CMUnits::Set(int aType,double aMult,int aPrec)
{
	wchar_t buffer[64];
	_type = aType;
	_prec = aPrec>=0 ? aPrec : -aPrec;
	if (_prec>10) _prec=10;

	if (aMult <= 0) aMult = 1;
	const wchar_t* ptr;
	switch (aType) {
		case AcreFeet: ptr = list[af]; _xlatToLocalValue=factors[AcreFeet]; break;
		case Gallons:  ptr = list[g];  _xlatToLocalValue=factors[Gallons];break;
		case CubicFeet:  ptr = list[cf];  _xlatToLocalValue=factors[CubicFeet];break;
		case CubicMeters:  ptr = list[cm];  _xlatToLocalValue=factors[CubicMeters];break;
		case HectareMeters: ptr = list[hm]; _xlatToLocalValue=factors[HectareMeters];break;
		case Dollars: ptr = list[d]; _xlatToLocalValue=factors[Dollars];break;
		default: _type=User; ptr = 0; _xlatToLocalValue=factors[User];break;
	}

	if (ptr)	swprintf_s(buffer, 64, L"%sX%fp%d",ptr,aMult,_prec);
	else    	swprintf_s(buffer, 64, L"%fp%d",aMult,_prec);

	_units = buffer;
	_xlatToLocalValue /= aMult;
}

double CMUnits::Multiplier() const
{
	return factors[_type]/_xlatToLocalValue;
}

wostream& operator << (wostream& s, CMUnits& u)
{
	return s << u._units;
}

wistream& operator >> (wistream& s, CMUnits& u)
{
	CMString line;
	line.read_line(s);
	u.Set(line);
	return s;
}

wostream& CMUnits::WriteBinary(wostream& s)
{
	writestringbinary(_units,s);
	return s;
}

wistream& CMUnits::ReadBinary(wistream& s)
{
	Set(readstringbinary(s));
	return s;
}

int CMUnits::BinaryLength()
{
	return stringbinarylength(_units);
}
