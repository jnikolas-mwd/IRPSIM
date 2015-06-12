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

typedef class _IRPCLASS stringValue<string> stringVALUESTRING;
typedef class _IRPCLASS CMPSmallArray<stringVALUESTRING> CMPSMALLSTRINGVALUESTRINGARRAY;

class _IRPCLASS	CMAssociations : public CMPSMALLSTRINGVALUESTRINGARRAY
{
public:
	CMAssociations(unsigned short sz=0,unsigned short d=16) : CMPSmallArray< stringValue<string> > (sz,d) {}
	void   AddAssociation(const string& aName,const string& val);
	string GetName(unsigned short n);
	string GetValue(unsigned short n);
	string GetValue(const string& aName);
    int	 GetAssociationIndex(const string& aName);
	int  GetAssociation(unsigned short n,string& s1,string& s2);
	int  IsAssociation(string& s1,string& s2);
    void   Compact();
};
