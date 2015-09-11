// cmtime.cpp : implementation file
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
#include "StdAfx.h"
#include "cmtime.h"
#include "cmlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include <fstream>
//static wofstream sdebug("debug_cmtime.txt");

const int CMTime::daysinmonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
const int CMTime::firstdayofmonth[12] = {1,32,60,91,121,152,182,213,244,274,305,335};

const wchar_t* CMTime::monthname[12] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun",
													  L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"};
const wchar_t* CMTime::dayname[7] = { L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };


short CMTime::output_format = CMTime::formatFull;


// Default constructor: today's date
CMTime::CMTime() :
julnum(CMTime::PresentDate()),
nseconds(CMTime::PresentTime())
{
}

CMTime::CMTime(int y, int m, int d, int hrs, int mns, int secs) :
julnum(CMTime::ymdTojday(y,m,d)),
nseconds(CMTime::hmsToseconds(hrs,mns,secs))
{
}

CMTime::CMTime(const CMTime& t)
{
	julnum = t.julnum;
	nseconds = t.nseconds;
}

CMTime& CMTime::operator = (const CMTime& t)
{
	julnum = t.julnum;
	nseconds = t.nseconds;
	return *this;
}

void CMTime::SetTime(int y, int m, int d, int h, int mn, int s)
{
	int cy,cm,cd,ch,cmn,cs;
	YearMonthDay(cy,cm,cd);       // Get Current values
	HourMinuteSecond(ch,cmn,cs);

	// set for values >= 0, otherwise don't change

	julnum 	= ymdTojday(y>=0?y:cy,m>=0?m:cm,d>=0?d:cd);
	nseconds = hmsToseconds(h>=0?h:ch,mn>=0?mn:cmn,s>=0?s:cs);
}

short CMTime::SetOutputFormat(short format)
{
	short old = output_format;
	if (format>0)
		output_format = format;
   else switch (format) {
   	case CM_SECOND: output_format = YYYYMMDDHHMMSS; break;
		case CM_MINUTE: output_format = YYYYMMDDHHMM; break;
		case CM_HOUR: output_format = YYYYMMDDHH; break;
		case CM_DAY:
		case CM_WEEK: output_format = YYYYMMDD; break;
		case CM_MONTH: output_format = YYYYMM; break;
		default: output_format = YYYY; break;
   }
	return old;
}

void CMTime::set_to_string(const CMString& str)
{
	int y=1995,m=1,d=1,h=0,mn=0,s=0;

   int len = str.length();

	if (len >= 4)	y = _wtoi(((CMString)str.substr(0,4)).c_str());
	if (len >= 6)	m = _wtoi(((CMString)str.substr(4, 2)).c_str());
	if (len >= 8)	d = _wtoi(((CMString)str.substr(6, 2)).c_str());
	if (len >= 10)	h = _wtoi(((CMString)str.substr(8, 2)).c_str());
	if (len >= 12)	mn = _wtoi(((CMString)str.substr(10, 2)).c_str());
	if (len >= 14)	s = _wtoi(((CMString)str.substr(12, 2)).c_str());

	SetTime(y,m,d,h,mn,s);
};

BOOL CMTime::IsLeapYear(unsigned year)
{
	if( year <= 99 ) year += 1900;
	if (year%4)
		return FALSE;
	else if (!(year%100) && (year%400))
		return FALSE;
	else
		return TRUE;
}

BOOL CMTime::IsDayWithinMonth(int month, int day, int year)
{
  if (day < 1 || month < 1 || month > 12) return FALSE;
  return day <= DaysInMonth(month,year);
}

int CMTime::DaysInYear(unsigned year)
{
  return IsLeapYear(year) ? 366 : 365;
}

int CMTime::DaysInMonth(int m,int y)
{
	int d = daysinmonth[m-1];
	if (IsLeapYear(y) && m == 2) d++;
	return d;
}

int CMTime::FirstDayOfMonth(int m,int y)
{
	int d = firstdayofmonth[m-1];
	if (IsLeapYear(y) && m > 2) d++;
	return d;
}

int CMTime::LastDayOfMonth(int m,int y)
{
	int d = firstdayofmonth[m-1];
	int dim = daysinmonth[m-1];
	if (IsLeapYear(y)) {
		if (m>2) d++;
      if (m==2) dim++;
   }
	return d+dim-1;
}

double CMTime::MidMonthDay(int m,int y)
{
	int d = firstdayofmonth[m-1];
	int dim = daysinmonth[m-1];
	if (IsLeapYear(y)) {
		if (m>2) d++;
      if (m==2) dim++;
   }
	return d+((double)dim-1)/2;
}

ULONG CMTime::ymdTojday(int y, int m, int d)
{
  unsigned long c, ya;
  if( y <= 99 ) y += 1900;
  if( !IsDayWithinMonth(m, d, y) ) return (ULONG)0;

  if (m > 2) { m -= 3; }  // wash out the leap day
  else       { m += 9;	y--; }
  c = y / 100;
  ya = y - 100*c;
  return ((146097L*c)>>2) + ((1461*ya)>>2) + (153*m + 2)/5 + d + 1721119L;
}

/*
 * Convert a Julian day number to its corresponding Gregorian calendar
 * date.  Algorithm 199 from Communications of the ACM, Volume 6, No. 8,
 * (Aug. 1963), p. 444.  Gregorian calendar started on Sep. 14, 1752.
 * This function not valid before that.
 */

void CMTime::YearMonthDay(int& y, int& m, int& D) const
{
  unsigned long d;
  unsigned long j = julnum - 1721119L;
  y = (unsigned) (((j<<2) - 1) / 146097L);
  j = (j<<2) - 1 - 146097L*y;
  d = (j>>2);
  j = ((d<<2) + 3) / 1461;
  d = (d<<2) + 3 - 1461*j;
  d = (d + 4)>>2;
  m = (unsigned)(5*d - 3)/153;
  d = 5*d - 3 - 153*m;
  D = (unsigned)((d + 5)/5);
  y = (unsigned)(100*y + j);
  if (m < 10)
	m += 3;
  else {
	m -= 9;
	y++;
  }
}

void CMTime::HourMinuteSecond(int& h, int& m, int& s) const
{
	h = (unsigned) (nseconds/SEC_IN_HOUR);
	m = (unsigned) ((nseconds - h*SEC_IN_HOUR)/60);
	s = (unsigned) (nseconds%60);
}

int CMTime::Year() const
{
	int y,m,d;YearMonthDay(y,m,d);return y;
}

int CMTime::Month() const
{
	int y,m,d;YearMonthDay(y,m,d);return m;
}

int CMTime::Day() const
{
	int y,m,d;YearMonthDay(y,m,d);return d;
}

int CMTime::Hour() const
{
	int h,m,s;HourMinuteSecond(h,m,s);return h;
}

int CMTime::Minute() const
{
	int h,m,s;HourMinuteSecond(h,m,s);return m;
}

int CMTime::Second() const
{
	int h,m,s;HourMinuteSecond(h,m,s);return s;
}

int CMTime::WeekDay() const
{
	return (int) (julnum % 7 + 1);
}

int CMTime::DayInYear() const
{
	int y,m,d;YearMonthDay(y,m,d);
	return FirstDayOfMonth(m,y)+m-1;
}

int CMTime::DaysInYear() const
{
	int y,m,d;YearMonthDay(y,m,d);
   return DaysInYear(y);
}

int CMTime::DaysInMonth() const
{
	int y,m,d;YearMonthDay(y,m,d);
   return DaysInMonth(m,y);
}

int CMTime::FirstDayOfMonth() const
{
	int y,m,d;YearMonthDay(y,m,d);
   return FirstDayOfMonth(m,y);
}

int CMTime::LastDayOfMonth() const
{
	int y,m,d;YearMonthDay(y,m,d);
   return LastDayOfMonth(m,y);
}

double CMTime::MidMonthDay() const
{
	int y,m,d;YearMonthDay(y,m,d);
   return MidMonthDay(m,y);
}

const wchar_t* CMTime::WeekDayName() const
{
	return dayname[WeekDay()-1];
}

int CMTime::Month(const wchar_t* name)
{
	int ret=0;
	if (isnumber(name))
		ret = _wtoi(name);

	for (int i=0;i<12&&!ret;i++)
		if (!_wcsnicmp(name,monthname[i],wcslen(monthname[i])))
			ret=i+1;

	return (ret>0 && ret <13) ? ret : 12;
}

unsigned long CMTime::PresentDate()
{
  time_t clk = time(0);
  struct tm t;
  //struct tm *t = localtime(&clk);
  errno_t err = localtime_s(&t, &clk);
  return ymdTojday(t.tm_year+1900,t.tm_mon+1,t.tm_mday);
}

long CMTime::PresentTime()
{
  time_t clk = time(0);
  struct tm t;
  //struct tm *t = localtime(&clk);
  errno_t err = localtime_s(&t, &clk);
  return hmsToseconds(t.tm_hour, t.tm_min, t.tm_sec);
}

CMTime& CMTime::inc(long n,CMTIMEUNIT units)
{
	int m,d,y;
	if (units==CM_SECOND || units==CM_MINUTE || units==CM_HOUR) {
		switch (units) {
			case CM_SECOND: nseconds += n; break;
			case CM_MINUTE: nseconds += 60*n; break;
			case CM_HOUR:   nseconds += 3600*n; break;
		}
		while (nseconds>=SEC_IN_DAY) {
			nseconds -= SEC_IN_DAY;
			julnum++;
		}
		while (nseconds<0) {
			nseconds += SEC_IN_DAY;
			julnum--;
		}
	}
	else {
		switch (units) {
			case CM_DAY: julnum += n;	break;
			case CM_WEEK:	julnum += 7*n;	break;
			case CM_MONTH:
				{
				YearMonthDay(y,m,d);
				long mn = m+n;
				while (mn>12) {
					mn-=12;
					y++;
				}
				while (mn<1) {
					mn+=12;
					y--;
				}
			   m = (int)mn;
				int dys = DaysInMonth(m,y);
				if (d>=28) d=dys;
				julnum = ymdTojday(y,m,d);
				}
				break;
			default:  // i.e. year
				{
				YearMonthDay(y,m,d);
				y+=(int)n;
				julnum = ymdTojday(y,m,d);
				}
				break;
		}
	}
	return *this;
}

CMTime CMTime::Plus(long n,CMTIMEUNIT units) const
{
	CMTime ret(*this);
	ret.inc(n,units);
	return ret;
}

CMTime  CMTime::AtBeginning(CMTIMEUNIT units,int weekbegin) const
{
	int m,d,y,h,mn,s;
	YearMonthDay(y,m,d);
	HourMinuteSecond(h,mn,s);

   switch (units) {
   	case CM_MINUTE:	return CMTime(y,m,d,h,mn,0);
      case CM_HOUR:   	return CMTime(y,m,d,h,0,0);
      case CM_DAY:    	return CMTime(y,m,d,0,0,0);
		case CM_WEEK:
         {
			CMTime ret(*this);
         ret.nseconds=0;
			if (weekbegin) {
         	long inc = julnum%7 - weekbegin + 1;
            if (inc<0) inc+=7;
	         ret.julnum -= inc;
         }
         return ret;
         }
      case CM_MONTH:  	return CMTime(y,m,1,0,0,0);
      case CM_YEAR:    	return CMTime(y,1,1,0,0,0);
	}
	return CMTime(y,m,d,h,mn,s);
}

CMTime CMTime::AtEnd(CMTIMEUNIT units,int weekbegin) const
{
	int m,d,y,h,mn,s;
	YearMonthDay(y,m,d);
	HourMinuteSecond(h,mn,s);

   switch (units) {
   	case CM_MINUTE:	return CMTime(y,m,d,h,m,59);
      case CM_HOUR:   	return CMTime(y,m,d,h,59,59);
      case CM_DAY:    	return CMTime(y,m,d,23,59,59);
		case CM_WEEK:
      	{
			CMTime ret(*this);
         ret.nseconds = 86399L;
			if (weekbegin) {
	         long inc = 5+weekbegin-julnum%7;
            if (inc>=7) inc-=7;
	         ret.julnum += inc;
         }
         return ret;
         }
      case CM_MONTH:
         d=daysinmonth[m-1];
         if (m==2 && IsLeapYear(y)) d+=1;
			return CMTime(y,m,d,23,59,59);
      case CM_YEAR: return CMTime(y,12,31,23,59,59);
	}
	return CMTime(y,m,d,h,mn,s);
}

long CMTime::seconds_in_increment(CMTIMEUNIT units,short length)
{
	switch (units) {
		case CM_MINUTE: 	return length*SEC_IN_MINUTE;
		case CM_HOUR: 		return length*SEC_IN_HOUR;
		case CM_DAY: 		return length*SEC_IN_DAY;
		case CM_WEEK: 		return length*SEC_IN_WEEK;
		case CM_MONTH: 	return length*SEC_IN_MONTH;
		case CM_YEAR: 		return length*SEC_IN_YEAR;
		default:				return length;
	}
}

wchar_t* CMTime::GetString(wchar_t* buffer, size_t sz) const
{
	int m,d,y,h,mn,s;
	YearMonthDay(y,m,d);
	HourMinuteSecond(h,mn,s);
	switch (output_format) {
		case formatDate:
			swprintf_s(buffer, sz, L"%02d/%02d/%02d",m,d,y%100); break;
		case formatFull:
			swprintf_s(buffer, sz, L"%02d/%02d/%02d  %02d:%02d:%02d",m,d,y%100,h,mn,s); break;
		case YYYY_MM_DD_HHMMSS:
			swprintf_s(buffer, sz, L"%04d-%02d-%02d-%02d%02d%02d", y, m, d, h, mn, s); break;
		case YYYYMMDDHHMM:
			swprintf_s(buffer, sz, L"%04d%02d%02d%02d%02d",y,m,d,h,mn); break;
		case YYYYMMDDHH:
			swprintf_s(buffer, sz, L"%04d%02d%02d%02d",y,m,d,h); break;
		case YYYYMMDD:
			swprintf_s(buffer, sz, L"%04d%02d%02d",y,m,d); break;
		case YYYYMM:
			swprintf_s(buffer, sz, L"%04d%02d",y,m); break;
		case YYYY:
			swprintf_s(buffer, sz, L"%04d",y); break;
		case MM:
			swprintf_s(buffer, sz, L"%02d",m); break;
		default:
			swprintf_s(buffer, sz, L"%04d%02d%02d%02d%02d%02d",y,m,d,h,mn,s); break;
	}
	return buffer;
}

CMTIMEUNIT CMTime::GetResolution(const wchar_t* str)
{
   int len = (str ? wcslen(str) : 0);
	CMTIMEUNIT resolution;
   switch (len) {
   	case 4: resolution=CM_YEAR;break;
   	case 6: resolution=CM_MONTH;break;
  		case 8: resolution=CM_DAY;break;
   	case 10: resolution=CM_HOUR;break;
  		case 12: resolution=CM_MINUTE;break;
  		case 14: resolution=CM_SECOND;break;
   	default: resolution=CM_NOTIMEUNIT;break;
	}
   return resolution;
}

CMTIMEUNIT CMTime::StringToTimeUnit(const CMString& str)
{
	int flag = CMString::set_case_sensitive(0);
	CMTIMEUNIT ret = CM_YEAR;

	if (str.contains(L"sec"))			ret = CM_SECOND;
	else if (str.contains(L"min"))		ret = CM_MINUTE;
	else if (str.contains(L"hour"))	ret = CM_HOUR;
	else if (str.contains(L"da"))		ret = CM_DAY; // works with "day" or "daily"
	else if (str.contains(L"week"))	ret = CM_WEEK;
	else if (str.contains(L"mon")) 	ret = CM_MONTH;

	CMString::set_case_sensitive(flag);
	return ret;
}

CMString CMTime::TimeUnitToString(CMTIMEUNIT units)
{
	switch (units) {
   	case CM_YEAR:	 return L"year";
   	case CM_MONTH:  return L"month";
   	case CM_WEEK: 	 return L"week";
   	case CM_DAY: 	 return L"day";
   	case CM_HOUR: 	 return L"hour";
   	case CM_MINUTE: return L"minute";
   	case CM_SECOND: return L"second";
	}
   return L"unknown";
}

CMTime CMTime::GetStartPoint(const CMString& s,CMTIMEUNIT units)
{
   CMTime ret(s);
   ret = ret.AtBeginning(GetResolution(s.c_str()));
	return ret.AtEnd(units);
}

CMTime CMTime::GetEndPoint(const CMString& s,CMTIMEUNIT units)
{
   CMTime ret(s);
   ret = ret.AtEnd(GetResolution(s.c_str()));
	return ret.AtEnd(units);
}

long CMTime::GetInterval(const wchar_t* b, const wchar_t* e, CMTIMEUNIT incunits, int inclength, int lastmonth, CMTime& beg, CMTime& end)
{
	beg = b;
   end = e;

   int begweek = beg.WeekDay();

	if (CMTime::GetResolution(e)==CM_YEAR) {
		end = CMTime(end.Year(),lastmonth);
		end = end.AtEnd(CM_MONTH);
   	if (incunits==CM_YEAR)
      	end = end.AtBeginning(CM_MONTH);
      else
	      end = end.AtBeginning(incunits,begweek);
   }
	else {
		end = end.AtEnd(CMTime::GetResolution(e));
	   end = end.AtBeginning(incunits,begweek);
   }

	if (CMTime::GetResolution(b)==CM_YEAR) {
   	if (incunits==CM_YEAR)
      	beg = CMTime(beg.Year(),lastmonth).AtBeginning(CM_MONTH);
      else {
			beg = CMTime(lastmonth<12 ? beg.Year()-1 : beg.Year(),lastmonth%12+1);
		   beg = beg.AtBeginning(incunits,begweek);
      }
   }
   else {
		beg = beg.AtBeginning(CMTime::GetResolution(b));
	   beg = beg.AtBeginning(incunits,begweek);
   }

	long diff = CMTime::Diff(end,beg,incunits,inclength);
	end = beg.Plus(diff*inclength,incunits);
	return (diff+1);
}

wostream& CMTime::Write(wostream& os,int binary) const
{
	if (binary) {
		os.write((wchar_t*)&julnum,sizeof(julnum));
		os.write((wchar_t*)&nseconds, sizeof(nseconds));
	}
	else {
		wchar_t buffer[128];
		os << GetString(buffer,128);
	}
	return os;
}

wistream& CMTime::Read(wistream& is,int binary)
{
	if (binary) {
		is.read((wchar_t*)&julnum, sizeof(julnum));
		is.read((wchar_t*)&nseconds, sizeof(nseconds));
	}
	else {
		CMString str;
		is >> str;
		set_to_string(str);
	}
	return is;
}

long CMTime::Diff(const CMTime& t1,const CMTime& t2,CMTIMEUNIT units,int length)
{
	double sii = seconds_in_increment(units,length);
	double diff = (t1.julnum-t2.julnum)*(SEC_IN_DAY/sii) + (t1.nseconds-t2.nseconds)/sii;
	return (long)round(diff);
}


