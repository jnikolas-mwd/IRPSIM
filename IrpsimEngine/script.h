// script.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMScript implements IRPSIM's scripting functionality
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
#include "irpobject.h"
#include "node.h"
#include "category.h"
#include "expresn.h"

#include "cmdefs.h"
#include "smvarray.h"
#include "smparray.h"
#include "cmstring.h"

#include <iostream>
using namespace std;

class _IRPCLASS CMScript;

class _IRPCLASS CMSimulation;
class _IRPCLASS CMIrpApplication;
class _IRPCLASS CMScriptIterator;

class _IRPCLASS CMScript : public CMIrpObject
{
	enum {sFail=0x0001};
	enum {TokEvaluate=-10000,TokCut,TokGain,TokLoss,TokPut,TokTake,TokTransfer,
   		TokShift,TokIf,TokElse,TokEndif,TokStage,TokCategory,TokNode,TokLimit,
         TokUsing,TokDemand,TokSupply,TokStorage,TokShortage,TokFrom,TokTo,TokDo,
         TokIgnore,TokEnd,TokLine,TokError};
   enum {ParseHeader,ParseFooter,ParseOK,ParseComment,ParseIf,ParseEndif,ParseError};
	enum {XSyntaxError,XMissingBracket,XBadAllocationRule,XMissingScript,XMissingEndif,XMisplacedElse,
   		XMisplacedEndif};
   enum {fSupply,fDemand,fTransfer,fStage,fAll,fCategory,fNode,fShortage};

   static wchar_t* error_strings[];
   static wchar_t* script_tokens[];
   static wchar_t* header;
   static wchar_t* footer;

	int 								state;
   CMVariable						*vnetsurplus;
   CMTimeMachine					*tm;
   CMSimulation					*sim;
   class _IRPCLASS CMVSmallArray<int>			script;
   class _IRPCLASS CMVSmallArray<CMString> 	 		lines; // actual text lines of script
   class _IRPCLASS CMVSmallArray<CMString> 	 		stagenames;
   class _IRPCLASS CMVSmallArray<CMString> 	 		categorynames;
   class _IRPCLASS CMVSmallArray<CMString> 	 		nodenames;
   class _IRPCLASS CMPSmallArray<CMExpression> 	expressions;
   class _IRPCLASS CMPSmallArray<CMNode>	  	 		nodes;
   class _IRPCLASS CMPSmallArray<CMCategory>		categories;
   class _IRPCLASS CMPSmallArray<CMVariable> 		*stages;

   virtual wistream& read(wistream& s);
   virtual wostream& write(wostream& s);
   virtual const wchar_t* IsA();

	int evaluatestage(int range,int index,int limindex,int rule);
	int evaluatenodes(int range,int index,int limindex,int rule,int nodetype);
	int cutback(int range,int index,int limindex,int rule,int ignorebalance,int nodetype);

	int gain(int range,int index);
   int loss(int range,int index);
   int put(int range,int index,int limindex,int rule,int ignorebalance);
   int take(int range,int index,int limindex,int rule,int ignorebalance);
	int transfer(int fromrange,int fromindex,int torange,int toindex,int limindex);
   int shift(int fromrange,int fromindex,int torange,int toindex,int limindex);

   int evaluate_token(const CMString& line);
   int parse_line(const CMString& line,CMIrpApplication& a);
   void parse_lines(CMScript& s,CMIrpApplication& a);
	int resolve_pointers();
   void reset();
public:
	CMScript(int id=-1);
	~CMScript();
   int SetSimulationContext(CMSimulation* s,CMIrpApplication* app);
   void RemoveSimulationContext(); // deletes all simulation associations
   int Run(CMTimeMachine* t);
	int Fail() {return (state&sFail);}
	void Parse(CMIrpApplication& a); // needs to be done after all scripts have been read
};

/*
class CMScriptLineIterator
{
	int lineno
   CMScript* script;
public:
	CMScriptIterator() : lineno(0) , script(0) {}
	CMScriptIterator(const CMScript& s) : lineno(0) , script(&s) {}
   void Reset(const CMScript& s) {lineno=0;script=&s;}
   CMString operator ()()
   	{if (lineno>=script->lines.Count()) return CMString();return script->lines[lineno++];}
};
*/

