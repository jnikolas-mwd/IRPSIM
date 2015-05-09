// savesim.h : header file
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
#pragma once

// Save Simulation -- ASCII Format

#include "savesim.h"

class _IRPCLASS CMSaveSimulationAscii : public CMSaveSimulation
{
private:
	CMString get_aggregation_string();

	void output_header(int which);

   void output_item(int which,const wchar_t* val,long row,long col,int width,int prec);
   void output_item(int which,double val,long row,long col,int width,int prec);
	void output_record_end(long row);

public:
	CMSaveSimulationAscii(CMSimulation& s,CMIrpApplication* a=0);
};