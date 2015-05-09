// savesims.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMSaveSimulationSpreadsheet is a class for saving simulation outcomes to
// spreadsheets.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "savesims.h"

#include <iomanip>
#include <stdio.h>
#include <math.h>

/*
static const unsigned char* bof_const = (unsigned char*) "\x00\x00\x02\x00\x06\x04"; //6
static const unsigned char* eof_const = (unsigned char*) "\x01\x00\x00\x00"; //4
static const unsigned char* iteration_count_const = (unsigned char*) "\x2f\x00\x01\x00\x32"; //5
static const unsigned char* calcmode_const = (unsigned char*) "\x02\x00\x01\x00\xff"; //5
static const unsigned char* split_const = (unsigned char*) "\x04\x00\x01\x00\x00"; //5
static const unsigned char* sync_const = (unsigned char*) "\x05\x00\x01\x00\x00"; //5
static const unsigned char* cursor_location_const = (unsigned char*) "\x32\x00\x01\x00\x01"; //5
static const unsigned char* global_protection_const = (unsigned char*) "\x24\x00\x01\x00\x00"; //5
static const unsigned char* print_header_const = (unsigned char*) "\x26\x00\x04\x00|&A\x00"; //8
static const unsigned char* print_footer_const = (unsigned char*) "\x25\x00\x08\x00|Page #\x00"; //12
*/

static const char* bof_const =  "\x00\x00\x02\x00\x06\x04"; //6
static const char* eof_const =  "\x01\x00\x00\x00"; //4
static const char* iteration_count_const =  "\x2f\x00\x01\x00\x32"; //5
static const char* calcmode_const =  "\x02\x00\x01\x00\xff"; //5
static const char* split_const =  "\x04\x00\x01\x00\x00"; //5
static const char* sync_const =  "\x05\x00\x01\x00\x00"; //5
static const char* cursor_location_const =  "\x32\x00\x01\x00\x01"; //5
static const char* global_protection_const =  "\x24\x00\x01\x00\x00"; //5
static const char* print_header_const =  "\x26\x00\x04\x00|&A\x00"; //8
static const char* print_footer_const =  "\x25\x00\x08\x00|Page #\x00"; //12

/*
BOF 	 					 // 0x00  2  0x0404
EOF 						 // 0x01  0
RANGE 					 // 0x06  8
ITERATION COUNT  		 // 0x2f  1  0x32
CALCMODE 				 // 0x02  1  0xff
SPLIT    				 // 0x04  1  0x00
SYNC     				 // 0x05  1  0x00
WINDOW1  				 // 0x07  31
COLUMN WIDTH 			 // 0x08  3
HEX64        			 // 0x64  32  filled with 0
CURSOR LOCATION 		 // 0x32  1   0x01
NAMED  RANGE   		 // 0x0b  24
QRANGE         		 // 0x19  25
GLOBAL PROTECTION 	 // 0x24  1   0x00
PRINT HEADER   		 // 0x26  4   "|&A"
PRINT FOOTER   		 // 0x25  8   "Page #"
PRINT BORDERS  		 // 0x2a  16  filled with 0xff
LABEL          		 // 0x0f  variable length
INTEGER        		 // 0x0d  7
FLOAT          		 // 0x0e  13
*/

CMSaveSimulationSpreadsheet::CMSaveSimulationSpreadsheet(CMSimulation& s,CMIrpApplication* a) :
CMSaveSimulation(s,a)
{
	range.type = 0x06;
   range.length = 8;
   range.start_col = 0;
   range.start_row = 0;

	window_description.type = 0x07;
	window_description.length = 31;
	window_description.cursor_col = 0;
	window_description.cursor_row = 0;
	window_description.format = 0xf1;
	window_description.unused1 = 0x00;
	window_description.col_width = 8;
	window_description.cols_on_screen = 8;
	window_description.rows_on_screen = 20;
	window_description.left_col = 0;
	window_description.top_row = 0;
	window_description.title_cols = 0;
	window_description.title_rows = 0;
	window_description.left_title_col = 0;
	window_description.top_title_row = 0;
	window_description.border_width_col = 4;
	window_description.border_width_row = 4;
	window_description.window_width = 72;
	window_description.unused2 = 0x00;

   column_width.type = 0x08;
   column_width.length = 3;

   hex_64.type = 0x64;
   hex_64.length = 32;
   memset(hex_64.str,0,sizeof(hex_64.str));

   named_range.type = 0x0b;
   named_range.length = 24;
   named_range.start_col = 0;
   named_range.start_row = 0;
   memset(named_range.name,0,sizeof(named_range.name));
   strcpy_s(named_range.name, 16, "SIMULATION");

	query_range.type = 0x19;
	query_range.length = 25;
   query_range.start_input_col = 0;
   query_range.start_input_row = 0;
	query_range.start_output_col = 0xffff;
	query_range.start_output_row = 0xffff;
	query_range.end_output_col = 0xffff;
	query_range.end_output_row = 0xffff;
	query_range.start_criteria_col = 0xffff;
	query_range.start_criteria_row = 0xffff;
	query_range.end_criteria_col = 0xffff;
	query_range.end_criteria_row = 0xffff;
	query_range.command = 0x00;

	print_borders.type = 0x2a;
	print_borders.length = 16;
	print_borders.start_row_col = 0xffff;
	print_borders.start_row_row = 0xffff;
	print_borders.end_row_col = 0xffff;
	print_borders.end_row_row = 0xffff;
	print_borders.start_col_col = 0xffff;
	print_borders.start_col_row = 0xffff;
	print_borders.end_col_col = 0xffff;
	print_borders.end_col_row = 0xffff;

   label.type = 0x0f;

   intval.type = 0x0d;
	intval.format = 0x80;
   intval.length = 7;

   floatval.type = 0x0e;
   floatval.length = 13;
}

void CMSaveSimulationSpreadsheet::output_header(int which)
{
	unsigned int lastrow,lastcol,firstcolwidth;

	fout->write(bof_const,6);
	fout->write(iteration_count_const,5);
	fout->write(calcmode_const,5);
	fout->write(split_const,5);
	fout->write(sync_const,5);
	fout->write(cursor_location_const,5);
	fout->write(global_protection_const,5);
	fout->write(print_header_const,8);
	fout->write(print_footer_const,12);

	fout->write((const char*)&window_description,sizeof(window_description));
	fout->write((const char*)&hex_64,sizeof(hex_64));
	fout->write((const char*)&print_borders,sizeof(print_borders));

	switch (which) {
   	case OutRealizations:
         lastrow = realization_records;
         lastcol = 1+arrayindex.Count();
         firstcolwidth = fieldwidth;
         break;
   	case OutSummary:
         lastrow = summary_records;
         lastcol = 5;
         firstcolwidth = maxnamelength;
         break;
   	case OutReliabilitySeries:
         lastrow = relseries_records;
         lastcol = 2;
         firstcolwidth = maxtargetlength;
         break;
   	case OutReliabilityDetail:
         lastrow = reldetail_records;
         lastcol = 23;
         firstcolwidth = maxtargetlength;
         break;
	}

	range.end_col=named_range.end_col=query_range.end_input_col = lastcol;
	range.end_row=named_range.end_row=query_range.end_input_row = lastrow;

   for (unsigned i=0;i<=lastcol;i++) {
   	column_width.col = i;
   	column_width.width = (i==0) ? firstcolwidth : fieldwidth;
		fout->write((const char*)&column_width,sizeof(column_width));
   }
}

void CMSaveSimulationSpreadsheet::output_footer(int which)
{
	fout->write(eof_const,4);
}

void CMSaveSimulationSpreadsheet::output_record_end(long row)
{
}

void CMSaveSimulationSpreadsheet::output_item(int which,const char* val,long row,long col,int width,int prec)
{
   label.col = (unsigned short)col;
   label.row = (unsigned short)row;
	label.length = 7 + strlen(val);
   label.format = 0x80 | (prec>15?15:prec);
	fout->write((const char*)&label,sizeof(label));
   fout->put((unsigned char)(row==0 ? 0x22 : 0x27));
	fout->write(val,strlen(val)+1);
}

void CMSaveSimulationSpreadsheet::output_item(int which,double val,long row,long col,int width,int prec)
{
	if (prec==0 && fabs(val) <= 32767.0) {
	   intval.col = (unsigned short)col;
   	intval.row = (unsigned short)row;
      intval.value = (short)val;
		fout->write((const char*)&intval,sizeof(intval));
   }
   else {
	   floatval.col = (unsigned short)col;
   	floatval.row = (unsigned short)row;
      floatval.value = val;
	   floatval.format = 0x80 | (prec>15?15:prec);
		fout->write((const char*)&floatval,sizeof(floatval));
   }
}


