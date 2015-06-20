// assoc.h : header file
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
#pragma once

#include "irp.h"
#include "cmdefs.h"
#include "strval.h"
#include "smparray.h"

//typedef class _IRPCLASS CMStringValue<CMString> CMSTRINGVALUESTRING;
//typedef class _IRPCLASS CMPSmallArray<CMSTRINGVALUESTRING> CMPSMALLSTRINGVALUESTRINGARRAY;

class _IRPCLASS	CMAssociations
{
	class NameValue {
	public:
		CMString name;
		CMString value;

		NameValue(const CMString& n, const CMString& v) { name = n; value = v; }
	};

	CMPSmallArray<NameValue> *_associations;
//	CMPSMALLSTRINGVALUESTRINGARRAY* _associations;
public:
	CMAssociations(unsigned short sz = 0, unsigned short d = 16)  {
		//_associations = new CMPSmallArray< CMStringValue<CMString> >(sz, d);
		_associations = new CMPSmallArray<NameValue>(sz, d);
	}
	~CMAssociations() { _associations->ResetAndDestroy(1); delete _associations; }

	int Count() { return _associations->Count(); }
	void ResetAndDestroy() { _associations->ResetAndDestroy(1); }

	void   AddAssociation(const CMString& aName,const CMString& val);
	CMString GetName(unsigned short n);
	CMString GetValue(unsigned short n);
	CMString GetValue(const CMString& aName);
    int	 GetAssociationIndex(const CMString& aName);
	int  GetAssociation(unsigned short n,CMString& s1,CMString& s2);
	int  IsAssociation(CMString& s1,CMString& s2);
    void   Compact();
};
