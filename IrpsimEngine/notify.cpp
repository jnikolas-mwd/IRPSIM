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

CMNotifier* CMNotifier::pNotifier = NULL;

int CMNotifier::Notify(ntype type, const CMString& msg)
{
	if (!pNotifier)
		return 0;

	if (type==CMNotifier::LOGTIME) {
		int oldformat = CMTime::SetOutputFormat(CMTime::formatFull);
		CMString newmsg = CMTime().GetString() + L": " + msg;
		CMTime::SetOutputFormat(oldformat);
		return pNotifier->notify(type, newmsg);
	}

	return pNotifier->notify(type, msg);
}

int CMNotifier::Notify(ntype type)
{
	return Notify(type, L"");
}