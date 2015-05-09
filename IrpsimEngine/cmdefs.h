// cmdefs.h : header file
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

#if !defined (FALSE)
#define FALSE               0
#endif

#if !defined (TRUE)
#define TRUE                1
#endif

#if !defined (M_PI)
#define M_PI        3.14159265358979323846
#endif

#if !defined M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif

#define BITSPERBYTE 8

#define CM_NODOUBLE		-1.23456789e21
#define CM_BIGDOUBLE		 1e21
#define CM_SMALLDOUBLE   1e-21

/***TODO: Uncomment as necessary
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef unsigned char   UCHAR;
typedef char* 		    PCHAR;
typedef unsigned short  USHORT;
typedef short           SHORT;
*/
typedef unsigned char   BYTE;
typedef unsigned long   ULONG;
typedef int	            BOOL;

#define cmabs(x)   (((x)<0) ? -(x) : (x))
#define cmmin(x,y) (x<y?x:y)
#define cmmax(x,y) (x>y?x:y)
#define cmsqr(x)   ((x)*(x))
#define cmsign(x,y)(((y)>=0) ? fabs(x) : -fabs(x))

#if defined (_CM_SOLARIS)
	#include <cm/soldefs.h>
	#define IOS_BINARY 0
	#define STRNCMPI strncasecmp
	#define STRCMPI  strcasecmp
   #define ENDL "\n"
#else
	#define IOS_BINARY ios::binary
	//#define STRNCMPI _strnicmp
	//#define STRCMPI  _stricmp
   #define ENDL L"\r\n"
#endif

