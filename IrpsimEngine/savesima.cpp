// savesim.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMSaveSimulationAscii is a class for saving simulation outcomes as ASCII
// flat files.
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
#include "savesima.h"
#include "cmlib.h"

#include <iomanip>
#include <stdio.h>

//#include <fstream>
//static wofstream sdebug(L"debug_savesima.txt");

CMSaveSimulationAscii::CMSaveSimulationAscii(CMSimulation& s,CMIrpApplication* a) :
CMSaveSimulation(s,a)
{
}

CMString CMSaveSimulationAscii::get_aggregation_string()
{
	wchar_t buffer[128];
   wchar_t tmbuffer[32];
   int oldformat = CMTime::SetOutputFormat(simincunits);
   outbeg.GetString(tmbuffer, 32);
	if (state & rCalendarAggregation)
		swprintf_s(buffer, 128, L"%s beginning %s",CMTime::TimeUnitToString(outincunits).c_str(),tmbuffer);
	else if (outincsteps > 1)
		swprintf_s(buffer, 128, L"%d %ss beginning %s",outincsteps,CMTime::TimeUnitToString(simincunits).c_str(),tmbuffer);
	else
     	swprintf_s(buffer, 128, L"T = %s",tmbuffer);
   CMTime::SetOutputFormat(oldformat);
   return CMString(buffer);
}

void CMSaveSimulationAscii::output_item(int which, const wchar_t* str, long row, long col, int width, int prec)
{
	if (!str) return;
	if (col == 0) *fout << str;
	else *fout << _delimiter << str;
	/*
	if (outch == L'\0')	    				*fout << setw(outlen) << str;
	else if (outch != L' ' && col != 0) *fout << (wchar_t)outch << str;
	else if (col != 0)						*fout << setw(outlen) << L"" << str;
	else *fout << str;
	*/
}

void CMSaveSimulationAscii::output_item(int which,double val,long row,long col,int width,int prec)
{
	wchar_t buffer[128];
	swprintf_s(buffer, 128, L"%.*f",prec,val);
	roundstring(buffer, prec);
	output_item(which, buffer, row, col, width, prec);
    //swprintf_s(buffer, 128, L"%.*f",prec,val);
    //output_item(which,buffer,row,col,width,prec);
}

void CMSaveSimulationAscii::output_record_end(long row)
{
	*fout << ENDL;
}



