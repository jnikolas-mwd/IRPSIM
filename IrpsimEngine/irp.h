// irp.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Global project definitions
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

#if defined (_WIN32)
	#if defined (_STATICIRPLIB)
		#define _IRPCLASS
		#define _IRPFUNC
		#define _IRPDATA
	#elif defined (_BUILDIRP)
		#define _IRPCLASS __declspec(dllexport)
		#define _IRPFUNC  __declspec(dllexport)
		#define _IRPDATA  __declspec(dllexport)
	#else
		#define _IRPCLASS __declspec(dllimport)
		#define _IRPFUNC  __declspec(dllimport)
		#define _IRPDATA  __declspec(dllimport)
#endif
#else
	#define _IRPCLASS
	#define _IRPFUNC
	#define _IRPDATA
#endif

#if !defined (_RTLENTRY)
#define _RTLENTRY  __cdecl
#endif
