// cmtime.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
//   Defines class CMTime.  CMTime allows you to define and manipulate time
//   objects with 1-second resolution.
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
#include "cmdefs.h"
#include "string.h"
#include <iostream>

using namespace std;

const ULONG JULIAN1901    = 2415386L;	// Julian day for 1/1/1901
const long  SEC_IN_MINUTE = 60L;
const long  SEC_IN_HOUR   = 3600L;
const long  SEC_IN_DAY	  = 86400L;
const long  SEC_IN_WEEK	  = 604800L;
const long  SEC_IN_MONTH  = 2629800L;
const long  SEC_IN_YEAR	  = 31557600L;

typedef short CMTIMEUNIT;

const short CM_NOTIMEUNIT = 0;
const short CM_SECOND 	  = -1;
const short CM_MINUTE 	  = -2;
const short CM_HOUR 	  = -3;
const short CM_DAY 		  = -4;
const short CM_WEEK       = -5;
const short CM_MONTH 	  = -6;
const short CM_YEAR 	  = -7;

class _IRPCLASS CMTime
{
	friend class _IRPCLASS CMTimeMachine;
	friend bool operator < (const CMTime& t1,const CMTime& t2);
	friend bool operator <= (const CMTime& t1, const CMTime& t2);
	friend bool operator > (const CMTime& t1, const CMTime& t2);
	friend bool operator >= (const CMTime& t1, const CMTime& t2);
	friend bool operator == (const CMTime& t1, const CMTime& t2);
	friend bool operator != (const CMTime& t1, const CMTime& t2);

	// two data members: julnum and nseconds

	ULONG julnum;    // Julian day
	long  nseconds;  // number of seconds within the current day

	static const int daysinmonth[12];
	static const int firstdayofmonth[12];
	static const wchar_t* monthname[12];
	static const wchar_t* dayname[7];
	static short output_format;
	static long seconds_in_increment(CMTIMEUNIT units,short length);
	void set_to_string(const string& str);
public:
	enum {formatFull=1,formatDate,YYYYMMDDHHMMSS,YYYYMMDDHHMM,
			YYYYMMDDHH,YYYYMMDD,YYYYMM,YYYY,MM};

	// Constructors
	CMTime();
	CMTime(int y, int m=1, int d=1, int hrs=0, int mns=0, int secs=0);
	CMTime(const CMTime& t);
	CMTime(const string& str) {set_to_string(str);}
	CMTime(const wchar_t* str)   { set_to_string(string(str ? str : L"")); }
	~CMTime() {};

	void SetTime(int y, int m=-1, int d=-1, int hrs=-1, int mns=-1, int secs=-1);

	wchar_t*  GetString(wchar_t* buffer, size_t sz) const;
	string GetString() const { wchar_t buffer[128]; return string(GetString(buffer, 128)); }
	wostream& Write(wostream& os,int binary=0) const;
	wistream& Read(wistream& os,int binary=0);

	void YearMonthDay(int& y, int& m, int& d) const;
	void HourMinuteSecond(int& h, int& m, int& s) const;

	int  Year() const;
	int  Month() const;
	int  Day() const;
	int  Hour() const;
	int  Minute() const;
	int  Second() const;
	int  DayInYear() const;
	int  DaysInYear() const;
	int  DaysInMonth() const;
	int  FirstDayOfMonth() const;
	int  LastDayOfMonth() const;
	double MidMonthDay() const;

	int WeekDay() const;
	const wchar_t* WeekDayName() const;
	CMTime& inc(long n,CMTIMEUNIT units);

	CMTime& operator = (const CMTime& t);
	CMTime& operator = (const string& str) {set_to_string(str);return *this;}
	CMTime& operator = (const wchar_t* str) { set_to_string(string(str)); return *this; }
	CMTime& operator = (ULONG n) {julnum = n;return *this;}

	operator ULONG () const {return julnum;}

	CMTime  Plus(long n,CMTIMEUNIT units) const;

   // AtBeginning
   // returns time at beginning of "units" If units is CM_WEEK, the function does
   // the following: if weekbegin is 1-7, that day is taken to be the beginning
   // day of the week and julnum is adjusted accordingly. Otherwise, no
   // adjustment is made

	CMTime  AtBeginning(CMTIMEUNIT units,int weekbegin=0) const;
	CMTime  AtEnd(CMTIMEUNIT units,int weekbegin=0) const; // returns time at end of "units"

   // SetOutputFormat(short) -- if argument is positive, interprets as
   // YYYYMMDD, etc. If argument is negative, interprets as a CMTIMEUNIT
   // value and sets format appropriately

	static short SetOutputFormat(short format);
	static ULONG ymdTojday(int y, int m, int d);
	static long  hmsToseconds(int h, int m, int s);
	static BOOL IsDayWithinMonth(int month, int day, int year);
	static BOOL IsLeapYear(unsigned year);
	static int DaysInMonth(int m,int y);
	static int FirstDayOfMonth(int m,int y);
	static int LastDayOfMonth(int m,int y);
	static double MidMonthDay(int m,int y);
	static int DaysInYear(unsigned year);
	static long Diff(const CMTime& t1,const CMTime& t2,CMTIMEUNIT units,int length);
	static CMTime GetStartPoint(const string& s,CMTIMEUNIT units);
	static CMTime GetEndPoint(const string& s,CMTIMEUNIT units);
	static long GetInterval(const wchar_t* b, const wchar_t* e, CMTIMEUNIT incunits, int inclength, int lastmonth, CMTime& beg, CMTime& end);

	static ULONG PresentDate();
	static long PresentTime();
	static const wchar_t* MonthName(int n) { return (n>0 && n<13) ? monthname[n - 1] : L""; }
	static const wchar_t* DayName(int n) { return (n>0 && n<8) ? dayname[n - 1] : L""; }
	static int Month(const wchar_t* name);
	// Gets resolution implied by input format -- e.g. 1996==CM_YEAR
   // 19960201==CM_DAY, etc.
	static CMTIMEUNIT GetResolution(const wchar_t* str);
   static CMTIMEUNIT StringToTimeUnit(const string& str);
   static string   TimeUnitToString(CMTIMEUNIT units);

	static const short BinarySize() {return 2*(short)sizeof(long);}
};

inline long CMTime::hmsToseconds(int h, int m, int s)
{
	return SEC_IN_HOUR*(ULONG)h + 60*(ULONG)m + (ULONG)s;
}

inline wostream& operator << (wostream& s , const CMTime& t)
{
	return t.Write(s,0);
}

inline wistream& operator >> (wistream& s , CMTime& t)
{
	return t.Read(s,0);
}

inline bool operator < (const CMTime& t1,const CMTime& t2)
{
	return (t1.julnum == t2.julnum) ? (t1.nseconds < t2.nseconds) : (t1.julnum < t2.julnum);
}

inline bool operator > (const CMTime& t1,const CMTime& t2)
{
	return (t1.julnum == t2.julnum) ? (t1.nseconds > t2.nseconds) : (t1.julnum > t2.julnum);
}

inline bool operator == (const CMTime& t1,const CMTime& t2)
{
	return ( (t1.julnum==t2.julnum) && (t1.nseconds==t2.nseconds) );
}

inline bool operator != (const CMTime& t1,const CMTime& t2)
{
	return ( (t1.julnum!=t2.julnum) || (t1.nseconds!=t2.nseconds) );
}

inline bool operator <= (const CMTime& t1,const CMTime& t2)
{
	return ( (t1<t2) || (t1==t2) );
}

inline bool operator >= (const CMTime& t1,const CMTime& t2)
{
	return ( (t1>t2) || (t1==t2) );
}



