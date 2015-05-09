// savesimd.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMSaveSimulationDbase is a class for saving simulation outcomes to
// relational databases.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "savesimd.h"

#include <iomanip>
#include <stdio.h>

CMSaveSimulationDbase::CMSaveSimulationDbase(CMSimulation& s,CMIrpApplication* a) :
CMSaveSimulation(s,a)
{
}

void CMSaveSimulationDbase::output_header(int which)
{
   CMTime timenow;

	memset(&header,0,sizeof(header));

   header.version = 0x03;
	header.update_year = timenow.Year()%100;
	header.update_month = timenow.Month();
	header.update_day   = timenow.Day();

	switch (which) {
   	case OutRealizations:
			header.records = realization_records;
			header.bytes_in_header = 32*(3+arrayindex.Count()) + 1;
			header.bytes_in_record = 1 + fieldwidth*(2+arrayindex.Count());
         break;
   	case OutSummary:
			header.records = summary_records;
			header.bytes_in_header = 32*7 + 1;
			header.bytes_in_record = 1 + maxnamelength + fieldwidth*5;
         break;
   	case OutReliabilitySeries:
			header.records = relseries_records;
			header.bytes_in_header = 32*4 + 1;
			header.bytes_in_record = 1 + maxtargetlength + fieldwidth*2;
         break;
   	case OutReliabilityDetail:
			header.records = reldetail_records;
			header.bytes_in_header = 32*25 + 1;
			header.bytes_in_record = 1 + maxtargetlength + fieldwidth*23;
	       break;
	}

	fout->write((const char*)&header,sizeof(header));
}

void CMSaveSimulationDbase::output_footer(int which)
{
}

void CMSaveSimulationDbase::output_record_end(long row)
{
	if (row==0) fout->put((unsigned char)0x0d);
}

void CMSaveSimulationDbase::output_item(int which,const char* val,long row,long col,int width,int prec)
{
	if (row==0) {
	   memset(&descriptor,0,sizeof(descriptor));
		strncpy_s(descriptor.name,11,val,10);
		descriptor.type = (col==0) ? 'C' : 'N';
   	descriptor.length=width;
		descriptor.decimal_places=prec;
      if (which==OutRealizations)
      	descriptor.type = 'N';
		fout->write((const char*)&descriptor,sizeof(descriptor));
	}
	else {
   	char buffer[128];
		if (col==0) fout->put(' ');
      strcpy_s(buffer, 128, val);
		buffer[width]=0;
		*fout << setw(width) << buffer;
	}
}

void CMSaveSimulationDbase::output_item(int which,double val,long row,long col,int width,int prec)
{
	char buffer[128];
   sprintf_s(buffer, 128, "%.*f",prec,val);
   buffer[width]=0;
	if (col==0) fout->put(' ');
	*fout << setw(width) << buffer;
}


