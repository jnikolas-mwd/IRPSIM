// queue.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMValueQueue implements a queue of values, maintaining current and
// original states
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "queue.h"
#include <stdio.h>

//#include <fstream.h>
//static ofstream os("queue.deb");

//int CMValueQueue::incunits=-1;
//int CMValueQueue::inclength=1;

#define CM_BIGTIME 10000000L

CMValueQueue::CMValueQueue(int sz) :
size(sz),
index(0)
{
	values = new _value[sz];
	Reset();
//	if (incunits<0)
//		CMTime::GetIncrement(incunits,inclength);
}

CMValueQueue::~CMValueQueue()
{
	delete [] values;
}

void CMValueQueue::Reset()
{
	last = CM_BIGTIME;
	index = 0;
	for (int i=0;i<size;i++)
		values[i].original = values[i].current = CM_NODOUBLE;
}

void CMValueQueue::preprocess_queue(const CMTime& current,CMTIMEUNIT incunits,int inclength)
{
	if ((ULONG)last==CM_BIGTIME)
		last = current;
	if (current!=last) {
		index += (int)CMTime::Diff(current,last,incunits,inclength);
		last=current;
		while (index>=size)
			index-=size;
		values[index].original=values[index].current=CM_NODOUBLE;
	}
}

int CMValueQueue::get_offset(const CMTime& t,CMTIMEUNIT incunits,int inclength)
{
	int offset = (int)CMTime::Diff(last,t,incunits,inclength);
   if (offset<0)
   	return -1;
   if (offset>=size) {
      _value* temp = new _value[offset+1];
		int i;
      for (i=0;i<=offset;i++)
	     	temp[i].original = temp[i].current = CM_NODOUBLE;
      for (i=0;i<size;i++) {
      	int j=i<=index ? i : offset-size+1+i;
         temp[j].original = values[i].original;
         temp[j].current = values[i].current;
	   }
   	size=offset+1;
      delete [] values;
      values = temp;
   }
	return offset;
}

int CMValueQueue::ContainsAt(const CMTime&t, const CMTime& current,CMTIMEUNIT incunits,int inclength)
{
	preprocess_queue(current,incunits,inclength);
	int offset = get_offset(t,incunits,inclength);
	if (offset < 0)
		return -1;
	return (values[(index+size-offset)%size].current != CM_NODOUBLE) ? 1 : 0;
}

double CMValueQueue::At(const CMTime& t,CMTIMEUNIT incunits,int inclength,int which)
{
	int offset = get_offset(t,incunits,inclength);
	if (offset>=0) {
		int n = (index+size-offset)%size;
		return which ? values[n].current : values[n].original;
	}
	return 0;
}

int CMValueQueue::AddAt(const CMTime&t,const CMTime& current,CMTIMEUNIT incunits,int inclength,double v)
{
	preprocess_queue(current,incunits,inclength);
	int offset = get_offset(t,incunits,inclength);
	if (offset >= 0) {
		int n = (index+size-offset)%size;
		if (values[n].original==CM_NODOUBLE)
			values[n].original = values[n].current = v;
		else
			values[n].current=v;
	}
	return (offset>=0);
}


