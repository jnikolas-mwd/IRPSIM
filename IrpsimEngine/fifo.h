// fifo.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Class CMFifo implements a first-in, first-out (FIFO) stack.
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
#include <deque>

class _IRPCLASS CMFifo
{
private:
	std::deque< std::pair<int, double> >* m_deque;
public:
	CMFifo();
	~CMFifo();

	void Push(int nStep, double dAmt);
	double Pop(double dAmt);
	double AvgAge(int nCurrentStep);
	double AmtOlderThan(int nStep);
	void Clear();
};



