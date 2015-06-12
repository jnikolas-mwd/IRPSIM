// fifo.cpp : implementation file
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
#include "StdAfx.h"
#include "fifo.h"

CMFifo::CMFifo()
{
	this->m_deque = new std::deque< std::pair<int, double> >();
}

CMFifo::~CMFifo()
{
	delete m_deque;
}

void CMFifo::Push(int nStep, double dAmt)
{
	m_deque->push_front(std::pair<int, double>(nStep, dAmt));
}

double CMFifo::Pop(double dAmt)
{
	if (m_deque->size() == 0) return 0;

	double dRemaining = dAmt;

	for (int i = m_deque->size() - 1; i >= 0 && dRemaining>0; i--) {
		if (m_deque->at(i).second>dRemaining) {
			m_deque->at(i).second -= dRemaining;
			dRemaining = 0;
		}
		else {
			dRemaining -= m_deque->at(i).second;
			m_deque->pop_back();
		}
	}
	return (dAmt - dRemaining);
}

double CMFifo::AvgAge(int nCurrentStep)
{
	double dSum = 0, dWeightedSum = 0;
	for (std::deque< std::pair<int, double> >::iterator i = m_deque->begin(); i != m_deque->end(); i++) {
		dSum += i->second;
		dWeightedSum += i->second * (nCurrentStep - i->first);
	}
	return (dSum != 0) ? dWeightedSum / dSum : 0;
}

double CMFifo::AmtOlderThan(int nStep)
{
	if (m_deque->size() == 0 || nStep<m_deque->at(m_deque->size() - 1).first)
		return 0;
	double dRet = 0;
	for (std::deque< std::pair<int, double> >::iterator i = m_deque->begin(); i != m_deque->end(); i++) {
		if (i->first <= nStep)
			dRet += i->second;
	}
	return dRet;
}

void CMFifo::Clear()
{
	m_deque->clear();
}

/*
void CMFifo::Push(int nStep,double dAmt) 
{
	push_front( std::pair<int,double>(nStep,dAmt) );
}

double CMFifo::Pop(double dAmt)
{
	if (size()==0) return 0;

	double dRemaining = dAmt;

	for (int i=size()-1;i>=0 && dRemaining>0;i--) {
		if (at(i).second>dRemaining) {
			at(i).second -= dRemaining;
			dRemaining = 0;
		}
		else {
			dRemaining -= at(i).second;
			pop_back();
		}
	}
	return (dAmt-dRemaining);
}

double CMFifo::AvgAge(int nCurrentStep)
{
	double dSum=0,dWeightedSum=0;
	for (iterator i=begin();i!=end();i++) {
		dSum+=i->second;
		dWeightedSum += i->second * (nCurrentStep - i->first);
	}
	return (dSum != 0) ? dWeightedSum/dSum : 0;
}

double CMFifo::AmtOlderThan(int nStep)
{
	if (size()==0 || nStep<at(size()-1).first)
		return 0;
	double dRet = 0;
	for (iterator i=begin();i!=end();i++) {
		if (i->first <= nStep)
			dRet += i->second;
	}
	return dRet;
}
*/

/*
double CMFifo::Pop(double dAmt)
{
	if (size()==0) 
		return 0;
	
	double dRemaining = dAmt;

	for (int i=size()-1;i>=0 && dRemaining>0;i--) {
		if (at(i).second>dRemaining) {
			at(i).second -= dRemaining;
			dRemaining = 0;
		}
		else {
			dRemaining -= at(i).second;
			pop_back();
		}
	}
	return (dAmt-dRemaining);
}

double CMFifo::AmtOlderThan(int nStep)
{
	if (size()==0 || nStep<at(size()-1).first)
		return 0;

	double dRet = 0;
	for (iterator i=begin();i!=end();i++) {
		if (i->first <= nStep)
			dRet += i->second;
	}
	return dRet;
}

double CMFifo::AvgAge(int nCurrentStep)
{
	double dSum=0,dWeightedSum=0;
	for (iterator i=begin();i!=end();i++) {
		dSum+=i->second;
		dWeightedSum += i->second * (nCurrentStep - i->first);
	}
	return (dSum != 0) ? dWeightedSum/dSum : 0;
}
*/