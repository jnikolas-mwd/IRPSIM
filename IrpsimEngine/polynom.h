// polynom.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMPolynomial implements IRPSIM's expression parsing and evaluation
// technology.
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
#include "variable.h"
#include "timemach.h"

#include "smvarray.h"
#include "smparray.h"
#include "cmdefs.h"
#include "function.h"
#include "cmstring.h"
#include <iostream>
using namespace std;

//class ostream;

class _IRPCLASS CMVNameIterator;
class _IRPCLASS CMPolynomial;

class _IRPCLASS CMPolynomial
{
	friend int operator == (const CMPolynomial& p1,const CMPolynomial& p2);
	friend wostream& operator << (wostream& os,CMPolynomial& ex);
	friend class _IRPCLASS CMPolynomialIterator;

	class _IRPCLASS CMVSmallArray<double> 		 constants;
	class _IRPCLASS CMVSmallArray<CMString> 	 varnames;
	class _IRPCLASS CMPSmallArray<CMVariable> 	 variables;
	class _IRPCLASS CMPSmallArray<CMPolynomial> expressions;
	class _IRPCLASS CMVSmallArray<CMFunction> 	 functions;
	class _IRPCLASS CMVSmallArray<int> 			 expression;

	CMTimeMachine*	 				 time;

	static wchar_t* errorstrings[];
	static int globalstate;
	int state;

	CMString* original;

// used during evaluation

	int 	  index;     // index into expression. Also used as a parentheses
							 // counter during translation
	int	  token;     // current token
	int 	  offset;  	 // offset into a list of constants,variables, or expressions
							 // the list depends on the current token
	int	  nargs;		 // number of arguments for current function or variable

	static  wchar_t *ops[];
	static  wchar_t *special_funcs[];

	enum statebits {failed=0x0001,stochastic=0x0002,hasvariables=0x0004};
	enum globalstatebits {ignoremissingvars=0x0001};

	enum toks {NoToken=0,If,Constant,Expression,UserFunc,Parameter,
				Integral,Definition,SpecialFunction,Function,Variable,VariableOrig,

				FirstVariableModifier,
				ArrayIndex=FirstVariableModifier,
				Lag,LagSecond,LagMinute,LagHour,LagDay,LagWeek,LagMonth,LagYear,
				Fwd,FwdSecond,FwdMinute,FwdHour,FwdDay,FwdWeek,FwdMonth,FwdYear,
				LastVariableModifier=FwdYear,

				FirstSpecialFunc,
            FuncSum=FirstSpecialFunc,
            FuncProd,
            LastSpecialFunc=FuncProd,

				FirstOp,
				Equal=FirstOp,NotEqual,LessThanOrE,GrThanOrE,LessThan,GrThan,
				Or2,Or,And2,And,Not,
				Assign,Plus,Minus,Mul,Div,Mod,Power,LPar,RPar,LBracket,RBracket,
				LastOp=RBracket
				};

	int islogical(int tok)   {return (tok>=Or2 && tok<=Not);}
	int isrelation(int tok)  {return (tok>=Equal && tok<=GrThan);}
	int isbinaryop(int tok)  {return (tok>=Equal && tok<=Power);}
	int isspecialfunc(int tok)	{return (tok>=FirstSpecialFunc && tok<=LastSpecialFunc);}
	int isop(int tok)		    {return (tok>=FirstOp && tok<=LastOp);}

	void level1(double& result);    // logical (||,&&)
	void level2(double& result);    // relational (<,<=,>=,>,==)
	void level3(double& result);    // add or subtract
	void level4(double& result);    // multiply or divide
	void level5(double& result);    // exponent
	void level6(double& result);    // unary + or -
	void level7(double& result);    // functions, parentheses, numbers

	void logical(int op,double& r,double& h);
	void relation(int op,double& r,double& h);
	void arith(int op,double& r,double& h);
	void unary(int op,double& r);
	void get_token();

	double eval_variable(int current);
	double eval_special_function();
	double eval_function();
	double eval_condition();
	double eval_const_or_expression(int type,int off)
		{return (type==Constant) ? constants[off] : expressions[off]->Evaluate(time);}

	void set_equal_to(const CMPolynomial& p);
	int translate(const wchar_t* aString);
	int translate_next_token(wchar_t*& ptr, int& tok, CMString& str, int& n, CMVSmallArray<CMString>& args);
	int translate_variable_arg(const CMString& str,int& tok,CMString& s);
	int add_constant_or_expression(CMString& str);
	int get_to_stop_symbol(const wchar_t* symbols, wchar_t*& ps, CMString& str);
	int find_bugs();
	void report_error(int code,const CMString& t);

	wostream& write(wostream& os);
	wostream& debug(wostream& os);
public:
	enum  {XNoExpression,XUndefinedSymbol,XMissingLPar,
			 XMissingRPar,XUnbalancedPar,XUndefinedFunction,
			 XMissingRBracket,XMissingComma,XTooManyIndexes,
			 XBadIndex,XBadExpression,XMissingArgument,
			 XNotEnoughArguments,XTooManyArguments,
			 XBadArgumentList,XInvalidArgument,XIllegalCondition,
			 XDivideByZero,XFunctionDomain,XMissingVariable
			 };
/*
	class CMXPolynomial : public xmsg
	{
		static CMString make_string(int c,const CMString& t);
		CMString text;
		int code;
	public:
		CMXPolynomial(int c,const CMString& t) :
			code(c), text(t) , xmsg(make_string(c,t)) {}
		CMXPolynomial(const CMXPolynomial& xp) :
			code(xp.code), text(xp.text) , xmsg(make_string(xp.code,xp.text)) {}
	};
	friend class CMXPolynomial;
*/
	CMPolynomial(const wchar_t* str = 0);
	CMPolynomial(const CMPolynomial& p);
	~CMPolynomial();
	CMString GetString();
	void UpdateVariableLinks();
	int  ContainsVariables() {return (state&hasvariables);}
	double Evaluate(CMTimeMachine* t);
	int Fail() {return (state&failed);}
	CMPolynomial& operator = (const wchar_t* str) { translate(str); return *this; }
	CMPolynomial& operator = (const CMPolynomial& p) {set_equal_to(p);return *this;}

	static void IgnoreMissingVariables(int action)
		{if (action) globalstate|=ignoremissingvars; else globalstate&=~ignoremissingvars;}
};


class _IRPCLASS CMPolynomialIterator
{
	// for iterating through variable names
	enum {iterNames,iterExpressions,iterVariables};
	int state;
	int pos;
	CMPolynomial& exp;
	CMPolynomialIterator* eIterator;
	CMVNameIterator* vIterator;
public:
	CMPolynomialIterator(CMPolynomial& aExp);
	~CMPolynomialIterator();
	const wchar_t* GetNext();
};


inline wostream& operator << (wostream& os,CMPolynomial& ex)
{
	return ex.write(os);
}
