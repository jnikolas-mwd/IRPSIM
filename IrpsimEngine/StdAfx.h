// stdafx.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define safe_delete(p){if(p){delete p;p=NULL;}}

#if defined(_DEBUG) && defined(_STATICIRPLIB)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	#define new DBG_NEW
#endif
#endif  // defined(_DEBUG) && defined(_STATICIRPLIB)
