// savesims.h : header file
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
#if !defined (__SAVESIMS_H)
#define __SAVESIMS_H

// Save Simulation -- Spreadsheed format

#include "savesim.h"

class _IRPCLASS CMSaveSimulationSpreadsheet : public CMSaveSimulation
{
	struct _range {
   	unsigned short type;
      unsigned short length;
		unsigned short start_col;
		unsigned short start_row;
		unsigned short end_col;
		unsigned short end_row;
	} range;

   struct _window_description {
   	unsigned short type;
      unsigned short length;
   	unsigned short  cursor_col;
   	unsigned short  cursor_row;
      unsigned char 	 format;             // 0xf1
      unsigned char 	 unused1;      		// 0
      unsigned short  col_width;  		   // 8
      unsigned short  cols_on_screen;     // 8
      unsigned short  rows_on_screen;     // 0x14
      unsigned short  left_col;		      // 0
      unsigned short  top_row;            // 0
      unsigned short  title_cols; 		   // 0
      unsigned short  title_rows;         // 0
      unsigned short  left_title_col;		// 0
      unsigned short  top_title_row;		// 0
      unsigned short  border_width_col;   // 0x04
      unsigned short  border_width_row;   // 0x04
      unsigned short  window_width;	      // 72
      unsigned char   unused2;				// 0
	} window_description;

   struct _column_width {
   	unsigned short type;
      unsigned short length;
   	unsigned short col;
      unsigned char width;
   } column_width;

   struct _hex_64 {
   	unsigned short type;
      unsigned short length;
   	unsigned char str[32]; //0
   } hex_64;

   struct _named_range {
   	unsigned short type;
      unsigned short length;
   	char name[16]; // null-terminated string;
      unsigned short start_col;
      unsigned short start_row;
      unsigned short end_col;
      unsigned short end_row;
   } named_range;

   struct _query_range {
   	unsigned short type;
      unsigned short length;
      unsigned short start_input_col;
      unsigned short start_input_row;
      unsigned short end_input_col;
      unsigned short end_input_row;
      unsigned short start_output_col;    // 0xff until last entry
      unsigned short start_output_row;
      unsigned short end_output_col;
      unsigned short end_output_row;
      unsigned short start_criteria_col;
      unsigned short start_criteria_row;
      unsigned short end_criteria_col;
      unsigned short end_criteria_row;
      unsigned char command;				   // 0
   } query_range;

   struct _print_borders {
   	unsigned short type;
      unsigned short length;
      unsigned short start_row_col; // 0xff for all
      unsigned short start_row_row;
      unsigned short end_row_col;
      unsigned short end_row_row;
      unsigned short start_col_col;
      unsigned short start_col_row;
      unsigned short end_col_col;
      unsigned short end_col_row;
   } print_borders;

   struct _label {
   	unsigned short type;
      unsigned short length;
		unsigned char format; // bit-field. bit 7 = protection
         	                // bits 0-3 == decimal places
      unsigned short col;
      unsigned short row;
   } label; // followed by null-terminated ascii string;

	struct _intval {
   	unsigned short type;
      unsigned short length;
   	unsigned char format; // bit-field 0x80
      unsigned short col;
      unsigned short row;
      short value;
   } intval;

	struct _floatval {
   	unsigned short type;
      unsigned short length;
   	unsigned char format; // bit-field 0x8/#decimal places
      unsigned short col;
      unsigned short row;
      double value;
   } floatval;

	void output_header(int which);
	void output_footer(int which);
	void output_record_end(long row);

   void output_item(int which,const char* val,long row,long col,int width,int prec);
   void output_item(int which,double val,long row,long col,int width,int prec);
public:
	CMSaveSimulationSpreadsheet(CMSimulation& s,CMIrpApplication* a=0);
};

#endif