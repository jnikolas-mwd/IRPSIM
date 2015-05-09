// vtime.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVTime implements internal IRPSIM variables that vary by time. 
// Inherits from CMVariable.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#include "vtime.h"

#include <math.h>

wchar_t* CMVTime::varnames[] = {L"_timestep",L"_year",L"_cyear",L"_tyear",L"_ctyear",
	L"_period",L"_tperiod",L"_month",L"_day",L"_weekday",L"_hour",
	L"_minute",L"_second",L"_dayinyear",L"_daysinyear",L"_daysinmonth",
   L"_weekinyear",L"_firstdayofmonth",L"_lastdayofmonth",
   L"_jan",L"_feb",L"_mar",L"_apr",L"_may",L"_jun",
   L"_jul",L"_aug",L"_sep",L"_oct",L"_nov",L"_dec",
   L"_midmonthday",
   L"_sin1",L"_sin2",L"_sin3",L"_sin4",L"_sin5",L"_sin6",
   L"_cos1",L"_cos2",L"_cos3",L"_cos4",L"_cos5",L"_cos6",
   L"_sin1d",L"_sin2d",L"_sin3d",L"_sin4d",L"_sin5d",L"_sin6d",
   L"_cos1d",L"_cos2d",L"_cos3d",L"_cos4d",L"_cos5d",L"_cos6d"
   };

CMVTime::CMVTime(int aType) :
CMVariable(CMVTime::varnames[aType]),
time_type(aType)
{
	SetState(vsDontMaintain|vsSystem|vsDontDestroy|vsNoUnits,TRUE);
   if (aType<MidMonthDay) SetState(vsInteger,TRUE);
}

double CMVTime::evaluate(CMTimeMachine* t,int,int)
{
	int lastmonth;

	CMTime& tm = t->At(0).Current();

	switch (time_type) {
		case Timestep:
			return (t->TimeStep());
		case Year:
			lastmonth = t->At(0).End().Month();
			return (tm.Year() + (tm.Month()>lastmonth ? 1 : 0));
		case TYear:
			lastmonth = t->At(1).End().Month();
			return (t->At(1).Current().Year() + (t->At(1).Current().Month()>lastmonth ? 1 : 0));
		case Period:
			return(((long)tm.Year())*100+tm.Month());
		case TPeriod:
			return (((long)t->At(1).Current().Year())*100+t->At(1).Current().Month());
		case CYear:					return(tm.Year());
		case CTYear:				return(t->At(1).Current().Year());
		case Month: 				return(tm.Month());
		case Day:   				return(tm.Day());
		case WeekDay: 				return(tm.WeekDay());
		case Hour:    				return(tm.Hour());
		case Minute:  				return(tm.Minute());
		case Second:  				return(tm.Second());
     	case DayInYear:      	return(tm.DayInYear());
      case DaysInYear:     	return(tm.DaysInYear());
      case DaysInMonth:    	return(tm.DaysInMonth());
      case WeekInYear:			return((int)tm.DayInYear()/7);
      case FirstDayOfMonth:	return(tm.FirstDayOfMonth());
      case LastDayOfMonth: 	return(tm.LastDayOfMonth());
      case Jan: 					return(tm.Month()==1 ? 1 : 0);
      case Feb: 					return(tm.Month()==2 ? 1 : 0);
      case Mar: 					return(tm.Month()==3 ? 1 : 0);
      case Apr: 					return(tm.Month()==4 ? 1 : 0);
      case May: 					return(tm.Month()==5 ? 1 : 0);
      case Jun: 					return(tm.Month()==6 ? 1 : 0);
      case Jul: 					return(tm.Month()==7 ? 1 : 0);
      case Aug: 					return(tm.Month()==8 ? 1 : 0);
      case Sep: 					return(tm.Month()==9 ? 1 : 0);
      case Oct: 					return(tm.Month()==10 ? 1 : 0);
      case Nov: 					return(tm.Month()==11 ? 1 : 0);
      case Dec: 					return(tm.Month()==12 ? 1 : 0);
      case MidMonthDay: 		return(tm.MidMonthDay());
		case Sin1:					return(sin(2*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Sin2:					return(sin(4*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Sin3:					return(sin(6*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Sin4:					return(sin(8*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Sin5:					return(sin(10*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Sin6:					return(sin(12*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Cos1:					return(cos(2*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Cos2:					return(cos(4*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Cos3:					return(cos(6*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Cos4:					return(cos(8*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Cos5:					return(cos(10*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Cos6:					return(cos(12*M_PI*tm.MidMonthDay()/tm.DaysInYear()));
		case Sin1D:					return(sin(2*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Sin2D:					return(sin(4*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Sin3D:					return(sin(6*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Sin4D:					return(sin(8*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Sin5D:					return(sin(10*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Sin6D:					return(sin(12*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Cos1D:					return(cos(2*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Cos2D:					return(cos(4*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Cos3D:					return(cos(6*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Cos4D:					return(cos(8*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Cos5D:					return(cos(10*M_PI*tm.DayInYear()/tm.DaysInYear()));
		case Cos6D:					return(cos(12*M_PI*tm.DayInYear()/tm.DaysInYear()));
	}
   return 0;
}

