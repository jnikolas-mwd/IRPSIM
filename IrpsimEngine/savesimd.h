// savesimd.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMSaveSimulationDbase is a class for saving simulation outcomes to
// relational databases.
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

// Save Simulation -- dBase Format

#include "savesim.h"

class _IRPCLASS CMSaveSimulationDbase : public CMSaveSimulation
{
private:
	struct DbaseHeader {
		char version;				// FoxBase+, FoxPro, dBaseIII+, dBaseIV, no memo - 0x03
										// FoxBase+, dBaseIII+ with memo - 0x83
										//	FoxPro with memo - 0xF5
										//	dBaseIV with memo - 0x8B
										//	dBaseIV with SQL Table - 0x8E
	   unsigned char update_year;  // Last update, format YYMMDD first char is year, second month, third day
   	unsigned char update_month;
	   unsigned char update_day;
   	unsigned long records;  // Number of records in file (32-bit number)
	   unsigned short bytes_in_header; 	// Number of bytes in header (16-bit number)
   	unsigned short bytes_in_record; 	// Number of bytes in record (16-bit number)
	   char  reserved1[2]; 		// Reserved, fill with 0x00
   	char  transaction_flag; // 14	dBaseIV flag, incomplete transaction Begin
   									// Transaction sets it to 0x01
										//	End Transaction or RollBack reset it to 0x00
	   char  encryption_flag; 	// Encryption flag, encrypted 0x01 else 0x00
   	char  multi_user[12]; 	// dBaseIV multi-user environment use
	   char  production_index; // Production index exists - 0x01 else 0x00
		char  lang_driver_id; 	// dBaseIV language driver ID
	   char  reserved2[2];     // Reserved, fill with 0x00
	} header;

	struct FieldDescriptor {
		/*
		FIELD IDENTIFIER TABLE
		ASCII   DESCRIPTION
		C       Character
		D       Date, format YYYYMMDD
		F       Floating Point
		G       General - FoxPro addition
		L       Logical, T:t,F:f,Y:y,N:n,?-not initialized
		M       Memo (stored as 10 digits representing the dbt block number)
		N       Numeric
		P       Picture - FoxPro addition
		*/
		char name[11]; // Field Name ASCII padded with 0x00
		char type; // Field Type Identifier (see table)
		unsigned long displacement;  // Displacement of field in record
		unsigned char length; // Field length in bytes
		unsigned char decimal_places;	// Field decimal places
		char reserved1[2]; //	Reserved
		char work_area_id; // dBaseIV work area ID
		char reserved2[10]; //	Reserved
		char production_index_flag; // Field is part of production index - 0x01 else 0x00
	} descriptor;

	void output_header(int which);
	void output_footer(int which);
	void output_record_end(long row);

	void output_item(int which, const wchar_t* val, long row, long col, int width, int prec);
   void output_item(int which,double val,long row,long col,int width,int prec);
public:
	CMSaveSimulationDbase(CMSimulation& s,CMIrpApplication* a=0);
};