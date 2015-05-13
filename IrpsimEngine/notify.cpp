// notify.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMNotify class for handling notifications
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	10 April 2015
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#include "notify.h"
#include "cmtime.h"

CMNotifyDelegatePointer CMNotifier::pDelegate = nullptr;

int CMNotifier::Notify(ntype type, const CMString& msg, int data)
{
	if (pDelegate==nullptr)
		return 0;
	
	if (type==CMNotifier::LOGTIME) {
		int oldformat = CMTime::SetOutputFormat(CMTime::formatFull);
		CMString newmsg = CMTime().GetString() + L": " + msg;
		CMTime::SetOutputFormat(oldformat);
		return pDelegate(type, newmsg.c_str(), data);
	}
	
	return pDelegate(type, msg.c_str(), data);
}

int CMNotifier::Notify(ntype type)
{
	return Notify(type, L"", 0);
}