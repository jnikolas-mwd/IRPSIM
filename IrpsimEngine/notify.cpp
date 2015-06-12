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
#include "StdAfx.h"
#include "notify.h"
#include "cmtime.h"

CMNotifier* CMNotifier::pNotifier = nullptr;

int CMNotifier::Notify(CMNotifier::ntype type, const CMString& msg, int data)
{
	if (pNotifier==nullptr)
		return 0;
	
	if (type==CMNotifier::LOGTIME) {
		int oldformat = CMTime::SetOutputFormat(CMTime::formatFull);
		CMString newmsg = CMTime().GetString() + L": " + msg;
		CMTime::SetOutputFormat(oldformat);
		return pNotifier->notify(type, newmsg.c_str(), data);
	}
	
	return  pNotifier->notify(type, msg.c_str(), data);
}