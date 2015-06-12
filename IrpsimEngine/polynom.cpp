// polynom.cpp : implementation file
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
#include "StdAfx.h"
#include <math.h>
#include <ctype.h>

#include "defines.h"
#include "polynom.h"

#include "advmath.h"
#include "cmlib.h"
#include "notify.h"

//#include <fstream>
//static wofstream sdebug(L"debug_polynom.txt");

wchar_t* CMPolynomial::errorstrings[] = {
	L"no expression",
	L"undefined symbol",
	L"missing left parenthesis",
	L"missing right parenthesis",
	L"unbalanced parentheses",
	L"undefined function",
	L"missing right bracket",
	L"missing comma",
	L"too many variable indexes",
	L"bad variable index",
	L"bad expression",
	L"missing function argument",
	L"not enough function arguments",
	L"too many function arguments",
	L"bad function argument list",
	L"invalid argument",
	L"illegal conditional expression",
	L"divide by zero",
	L"function domain error",
	L"missing variable",
	L"unrecognized operator"
};


wchar_t* CMPolynomial::badops[] = { L"=>", L"=<", L"===", NULL };

wchar_t* CMPolynomial::ops[] = { L"==", L"!=", L"<=", L">=", L"<", L">",
							L"||", L"|", L"&&", L"&", L"!", L"=", L"+", L"-",
							L"*", L"/", L"%", L"^", L"(", L")", L"{", L"}", NULL};

wchar_t* CMPolynomial::special_funcs[] = { L"sum", L"product", NULL };

int CMPolynomial::globalstate=0;

/* Construction and parsing functions */

CMPolynomial::CMPolynomial(const wchar_t* str) :
constants(0,16),
varnames(0,16),
variables(0,16),
expressions(0,16),
functions(0,16),
expression(5,10),
original(0),
state(0)
{
	int errorcode = translate(str);
	if (errorcode >= 0) {
		err_code = errorcode;
		state |= failed;
		report_error(errorcode, str);
//		throw CMXPolynomial(errorcode,CMString(str));
	}
}

CMPolynomial::CMPolynomial(const CMPolynomial& p) :
constants(0,16),
varnames(0,16),
variables(0,16),
expressions(0,16),
functions(0,16),
expression(5,10),
original(0),
state(0)
{
	set_equal_to(p);
}

CMPolynomial::~CMPolynomial()
{
	expressions.ResetAndDestroy(1);
   if (original) delete original;
}

void CMPolynomial::report_error(int code,const CMString& t)
{
	CMString s(errorstrings[code]);
	s += L" : " + t;
	CMNotifier::Notify(CMNotifier::ERROR,s);
}

void CMPolynomial::set_equal_to(const CMPolynomial& p)
{
	constants.Reset(0);
	varnames.Reset(0);
	variables.Reset(0);
	functions.Reset(0);
	expression.Reset(1);
	expressions.ResetAndDestroy(0);

	state = p.state;
	if (original) {
   	delete original;
      original=0;
   }

   if (p.original)
	  	original = new CMString(*p.original);

   unsigned short i;

	for (i=0;i<p.expression.Count();i++)
		expression.AddAt(i,p.expression[i]);
	for (i=0;i<p.constants.Count();i++)
		constants.AddAt(i,p.constants[i]);
	for (i=0;i<p.varnames.Count();i++)
		varnames.AddAt(i,p.varnames[i]);
	for (i=0;i<p.variables.Count();i++)
		variables.AddAt(i,p.variables[i]);
	for (i=0;i<p.functions.Count();i++)
		functions.AddAt(i,p.functions[i]);
	for (i=0;i<p.expressions.Count();i++)
		expressions.AddAt(i,new CMPolynomial(*p.expressions[i]));

	if (constants.Count()) 		constants.Resize(constants.Count());
	if (varnames.Count())  		varnames.Resize(varnames.Count());
	if (variables.Count()) 		variables.Resize(variables.Count());
	if (expressions.Count()) 	expressions.Resize(expressions.Count());
	if (functions.Count()) 		functions.Resize(functions.Count());
//	if (expression.Count()) 	expression.Resize(expression.Count());
}

int operator == (const CMPolynomial& p1,const CMPolynomial& p2)
{
	unsigned short i;
	int ret = 1;

	for (i=0;ret && i<p2.expression.Count();i++)
		ret = (p1.expression[i] == p2.expression[i]);
	for (i=0;ret && i<p2.constants.Count();i++)
		ret = (p1.constants[i] == p2.constants[i]);
	for (i=0;ret && i<p2.varnames.Count();i++)
		ret = (p1.varnames[i] == p2.varnames[i]);
	for (i=0;ret && i<p2.functions.Count();i++)
		ret = (p1.functions[i] == p2.functions[i]);
	for (i=0;ret && i<p2.expressions.Count();i++)
		ret = (*p1.expressions[i] == *p2.expressions[i]);

	return ret;
}

void CMPolynomial::UpdateVariableLinks()
{
	int i;
	for (i = 0; i < varnames.Count(); i++) {
		CMVariable* v = CMVariable::Find(varnames[i]);
		if (v != 0)
			variables.AddAt(i, v);
		else
			variables.AddAt(i, CMDefinitions::Find(varnames[i]));
	}
	for (i=0;i<expressions.Count();i++)
		expressions[i]->UpdateVariableLinks();
}

int CMPolynomial::translate(const wchar_t* aString)
{
	constants.Reset(0);
	varnames.Reset(0);
	variables.Reset(0);
	expressions.ResetAndDestroy(0);
	functions.Reset(0);
	expression.Reset(1);
   unsigned short i;
   int errorcode;
	state = 0;
	index = 0;

   if (original) {
   	delete original;
      original = 0;
   }

    wchar_t* ptr = (wchar_t*)aString;
	CMString str;
	CMVSmallArray<CMString> args;
	int tok;  // the token
	int n;  // the number of modifiers or offset (e.g. for definitions)

	while ((errorcode=translate_next_token(ptr,tok,str,n,args))<0) {
		expression.Add(tok);
		if (tok==NoToken)
			break;
		if (tok == Constant) {
			expression.Add((int)constants.Count());
			constants.Add(_wtof(str.c_str()));
		}
		else if (tok == Definition)
			expression.Add(n);
		else if (tok==Variable || tok==VariableOrig) {
			// insert index of variable
			expression.Add((int)varnames.Count());
			// insert number of variable arguments
			expression.Add(n);
			varnames.Add(str.c_str());
			variables.Add(0);
			int toktype;
			CMString s;
			for (int i=0;i<n&&errorcode<0;i++) {
				// type of variable arg (index or lag) will appear in
				// toktype, the argument itself will appear in s
				errorcode = translate_variable_arg(args[i],toktype,s);
				expression.Add(toktype);
				if (errorcode<0)
					errorcode = add_constant_or_expression(s);
			}
		}
		else if (isspecialfunc(tok)) {
			expression.Add(n);
			for (int i=0;i<n && i<args.Count() && errorcode<0;i++)
				errorcode = add_constant_or_expression(args[i]);
		}
		else if (tok==Function) {
			expression.Add((int)functions.Count());
			expression.Add(n);
			CMFunction fn(str.c_str());
			functions.Add(fn);
			int correctnum = fn.NumParameters();
			if (correctnum && n<correctnum)
				errorcode = XNotEnoughArguments;
			else if (correctnum && n>correctnum)
				errorcode = XTooManyArguments;
			for (int i=0;i<n && i<args.Count() && errorcode<0;i++)
				errorcode = add_constant_or_expression(args[i]);
		}
		else if (tok==If) {
			expression.Add(n);
			for (int i=0;i<n && errorcode<0;i++)
				errorcode = add_constant_or_expression(args[i]);
		}
		if (errorcode>=0)
			break;
	}
	if (errorcode<0 && index)
		errorcode = XUnbalancedPar;
	if (errorcode<0)
		errorcode = find_bugs();
	if (errorcode>=0)
		original = new CMString(stripends(CMString(aString)));

	if (constants.Count()) 		constants.Resize(constants.Count());
	if (varnames.Count())  		varnames.Resize(varnames.Count());
	if (variables.Count()) 		variables.Resize(variables.Count());
	if (expressions.Count()) 	expressions.Resize(expressions.Count());
	if (functions.Count()) 		functions.Resize(functions.Count());

	if (varnames.Count())
		state |= hasvariables;
	for (i=0;!(state&hasvariables) && i<expressions.Count();i++)
		if (expressions[i]->varnames.Count())
			state |= hasvariables;

	return errorcode;
}

int CMPolynomial::find_bugs()
{
	int err = -1;
	int nmod,next;
   unsigned short i;
	for (i=0;i<expression.Count() && err<0;i++) {
		if (isop(expression[i]))
			continue;
		else if (isspecialfunc(expression[i])) {
			nmod = expression[i+1];
			i += (1 + nmod*2);
		}
		else if (expression[i]==Constant || expression[i]==Definition)
			i++;
		else if (expression[i]==Function) {
			nmod = expression[i+2];
			i += (2 + nmod*2);
		}
		else if (expression[i]==Variable || expression[i]==VariableOrig) {
			nmod = expression[i+2];
			i += (2 + nmod*3);
		}
		else if (expression[i]==If) {
			nmod = expression[i+1];
			i += (1 + nmod*2);
		}
		next = (i+1<expression.Count()) ? expression[i+1] : NoToken;
		if (!isop(next) && next!=NoToken)
			err = XBadExpression;
	}
	return err;
}

int CMPolynomial::add_constant_or_expression(CMString& str)
{
	int ret = -1;
	if (isnumber(str.c_str())) {
		expression.Add(Constant);
		expression.Add((int)constants.Count());
		constants.Add(_wtof(str.c_str()));
	}
	else {
		expression.Add(Expression);
		expression.Add((int)expressions.Count());
		CMPolynomial* p = new CMPolynomial(str.c_str());
		expressions.Add(p);
		ret = p->GetErrorCode();
	}
	return ret;
}

int CMPolynomial::translate_next_token(wchar_t*& ptr, int& tok, CMString& str, int& n, CMVSmallArray<CMString>& args)
{
	int i;
	int err=-1;

	str.resize(0);
	args.Reset(0);

	if (!skipwhite(ptr)) {
		tok = NoToken;
		return -1;
	}

	if (isnumber(*ptr)) {
		while (isnumber(*ptr))
			str.append(*ptr++);
		tok = Constant;
		return -1;
	}

	// check to see if item is #IF
	if (!_wcsnicmp(ptr,L"@IF",3)) {
		n=1;
		args.AddAt(0,CMString());
		tok = If;
		ptr += 3;
		skipwhite(ptr);
		int symb = *ptr++;
		if (!(symb==L'(' || symb==L'{'))
			return XMissingLPar;
		if (get_to_stop_symbol(L")}",ptr,args[0])>=0)
			return XIllegalCondition;
		skipwhite(ptr);
		if (!_wcsnicmp(ptr, L"@THEN", 5)) {
			ptr+=5;
			skipwhite(ptr);
		}
		symb = *ptr++;
		args.AddAt(1,CMString());
		n=2;
		if (!(symb==L'(' || symb==L'{'))
			return XIllegalCondition;
		if (get_to_stop_symbol(L"})",ptr,args[1])>=0)
			return XIllegalCondition;
		skipwhite(ptr);
		if (!_wcsnicmp(ptr, L"@ELSE", 5)) {
			n=3;
			args.AddAt(2,CMString());
			ptr+=5;
			skipwhite(ptr);
			symb = *ptr++;
			if (!(symb==L'(' || symb==L'{'))
				return XIllegalCondition;
			if (get_to_stop_symbol(L"})",ptr,args[2])>=0)
				return XIllegalCondition;
		}
		return -1;
	}

	// check to see if item is a bad operation
	for (i = 0; badops[i] != NULL; i++) {
		if (!wcsncmp(ptr, badops[i], wcslen(badops[i])))
			return XUnrecognizedOperator;
	}

	// check to see if item is an operation
	for (i=0;ops[i]!=NULL;i++) {
		if (!wcsncmp(ptr, ops[i], wcslen(ops[i]))) {
			tok = FirstOp+i;
			if ((tok==LPar) || (tok==LBracket))
				index++;
			else if ((tok==RPar) || (tok==RBracket))
				index--;
			ptr += wcslen(ops[i]);
			return -1;
		}
	}

	// not a number or operation, so must be a special function,function or variable
	// first check to see if first symbol is allowed
	if (!(iswalpha(*ptr) || *ptr == L'_'))
		return XUndefinedSymbol;

	// check to see if this is a definition

	/* Definitions are now incorporated into variables
	const wchar_t* defname;
	sdebug << ptr << ENDL;
	for (i=0;(defname=CMDefinitions::GetDefinitionName(i))!=0;i++) {
		len = wcslen(defname);
		sdebug << L"   " << defname << L" " << len << ENDL;
		if (_wcsnicmp(defname, ptr, len)==0) {
			sdebug << L"FOUND " << defname << ENDL;
			// if it is in the list of definitions and the next symbol is not
			// a letter or '_' (implying a variable name that happens to begin
			// with the same letters as a special symbol), then add the 'special'
			// token to the CMString.
			if (!(iswalnum(*(ptr+len))) && *(ptr+len) != L'_') {
				tok = Definition;
				n = i;
				ptr += len;
				return -1;
			}
			break;
		}
	}
	*/

	// check to see if item is a special function
	for (i=0;special_funcs[i]!=NULL;i++) {
		if (!wcsncmp(ptr, special_funcs[i], wcslen(special_funcs[i]))) {
			tok = FirstSpecialFunc+i;
			ptr += wcslen(special_funcs[i]);
			skipwhite(ptr);
			if (*ptr++ != L'(')
				return XMissingArgument;
			CMString arglist;
			if (get_to_stop_symbol(L")",ptr,arglist)>=0)
				return XBadArgumentList;
			arglist += L',';
			wchar_t* argptr = (wchar_t*)arglist.c_str();
			for (n=0;*argptr && err<0;n++) {
				args.AddAt(n,CMString());
				if (get_to_stop_symbol(L",:",argptr,args[n])>=0)
					err = XBadArgumentList;
			}
			return err;
      }
	}

	// check to see if it is a function
	const wchar_t* func;
	if ((func=CMFunction::FindFunction(ptr))!=0) {
		tok = Function;
		str = func;
		ptr += wcslen(func);
		skipwhite(ptr);
		if (*ptr++ != L'(')
			return XMissingArgument;
		CMString arglist;
		if (get_to_stop_symbol(L")",ptr,arglist)>=0)
			return XBadArgumentList;
		arglist += L',';
		wchar_t* argptr = (wchar_t*)arglist.c_str();
		for (n=0;*argptr && err<0;n++) {
			args.AddAt(n,CMString());
			if (get_to_stop_symbol(L",:",argptr,args[n])>=0)
				err = XBadArgumentList;
		}
		return err;
	}

	// Not a number, operation, or pre-defined function
	// so must be a variable

	while (iswalnum(*ptr) || (*ptr == L'_') || (*ptr == L'@') || (*ptr == L'.') )
		str.append(*ptr++);

	CMString indexstr;
	n = 0;
	tok = Variable;
	while (skipwhite(ptr) && err<0) {
		if (*ptr == L'~') {
			tok = VariableOrig;
			ptr++;
		}
		if (*ptr == L'[') {
			args.AddAt(n,CMString());
			if (n < 4)
				err =	get_to_stop_symbol(L"]",++ptr,args[n++]);
			else
				err = XTooManyIndexes;
		}
		else
			break;
	}

	return err;
}

/* translate_variable_arg(const CMString& str,int& tok,CMString& s) translates
str (appearing between [ and ]) into a lag or forward token or an
array index. The CMString s is the lag or array index
(may be a number or expression) */

int CMPolynomial::translate_variable_arg(const CMString& str,int& tok,CMString& s)
{
	enum {_t=0,_s,_i,_h,_d,_w,_m,_y};

	s = str;
	int i,len = str.length();

	tok = ArrayIndex;

	for (i=0;i<len && str[i]<=L' ';i++);

	if (i==len)
		return XBadIndex;

	if (str[i] == L'_') {
		int toktype = -1;
		if (i+1 >= len)
			return XBadIndex;
		switch (str[i+1]) {
			case L't': case L'T': toktype = _t; break;
			case L's': case L'S': toktype = _s; break;
			case L'i': case L'I': toktype = _i; break;
			case L'h': case L'H': toktype = _h; break;
			case L'd': case L'D': toktype = _d; break;
			case L'w': case L'W': toktype = _w; break;
			case L'm': case L'M': toktype = _m; break;
			case L'y': case L'Y': toktype = _y; break;
		}
		if (toktype>=0) {
			if (i+2 >= len)
				return XBadIndex;
			if (str[i+2]<=L' ' || str[i+2]==L'+' || str[i+2]==L'-') {
         	int j;
				for (j=i+2;j<len && str[j]<=L' ';j++);
				if (j>=len || !(str[j]==L'+' || str[j]==L'-'))
					return XBadIndex;
				if (str[j] == L'-')
					tok = Lag + toktype;
				else
					tok = Fwd + toktype;
				s = str.c_str()+(j+1);
			}
		}
	}
	return -1;
}

int CMPolynomial::get_to_stop_symbol(const wchar_t* rsymbols,wchar_t*& ps,CMString& str)
{
	int ret = -1;
	int count = 1;

	str.resize(0);
	while (TRUE) {
		if (count==1 && wcschr(rsymbols,*ps)) count--;
//		if (count==1 && *ps==rsymbol) count--;
		else if (*ps==L'(' || *ps==L'[' || *ps==L'{') count++;
		else if (*ps==L')' || *ps==L']' || *ps==L'}') count--;
		if (!count) {
			ps++;
			break;
		}
		str.append(*ps++);
	}

	if (count) {
		switch (*rsymbols) {
			case L',': ret = XMissingComma; break;
			case L']': ret = XMissingRBracket; break;
			default:  ret = XMissingRPar; break;
		}
	}

	return ret;
}

CMString CMPolynomial::GetString()
{
	CMString ret;
	if (Fail()) {
   		if (original) 
			return *original;
		else
			return L"ERROR!";
   }
	int nmod;
   unsigned short i;
	for (i=0;i<expression.Count();i++) {
		if (expression[i]==Constant)
      	ret += constants[expression[++i]];
		else if (expression[i]==Definition)
			ret += CMDefinitions::GetDefinitionName(expression[++i]);
		else if (expression[i]==If) {
			nmod = expression[++i];
			ret += L"@IF (";
			if (expression[++i]==Constant)
	      	ret += constants[expression[++i]];
			else
				ret += expressions[expression[++i]]->GetString();
			ret += L") {";
			if (expression[++i]==Constant)
	      	ret += constants[expression[++i]];
			else
				ret += expressions[expression[++i]]->GetString();
			ret += L"}";
			if (nmod==3) {
				ret += L" @ELSE {";
				if (expression[++i]==Constant)
		      	ret += constants[expression[++i]];
				else
					ret += expressions[expression[++i]]->GetString();
				ret += L"}";
			}
		}
		else if (expression[i]==Variable||expression[i]==VariableOrig) {
			int isorig = (expression[i]==VariableOrig);
			ret += varnames[expression[++i]];
			if (isorig) ret += L"~";
			nmod = expression[++i];
			if(nmod) i++;
			for (int j=0;j<nmod;j++) {
				ret += L"[";
				switch (expression[i]) {
					case Lag: case Fwd: ret+= L"_t"; break;
					case LagSecond: case FwdSecond: ret+= L"_s"; break;
					case LagMinute: case FwdMinute: ret+= L"_i"; break;
					case LagHour: case FwdHour: ret+= L"_h"; break;
					case LagDay: case FwdDay: ret+= L"_d"; break;
					case LagWeek: case FwdWeek: ret+= L"_w"; break;
					case LagMonth: case FwdMonth: ret+= L"_m"; break;
					case LagYear: case FwdYear: ret+= L"_y"; break;
				}
				if (expression[i] >= Lag && expression[i] <= LagYear)
					ret += L"-";
				if (expression[i] >= Fwd && expression[i] <= FwdYear)
					ret += L"+";
				if (expression[++i]==Constant)
		      	ret += constants[expression[++i]];
				else
					ret += expressions[expression[++i]]->GetString();
				ret += L"]";
				if (j<nmod-1) i++;
			}
		}
		else if (isop(expression[i]))
			ret += ops[expression[i]-FirstOp];
		else if (expression[i]==Function||isspecialfunc(expression[i])) {
         if (expression[i]==Function)
				ret += functions[expression[++i]].GetName();
   		else
				ret += special_funcs[expression[i]-FirstSpecialFunc];
         ret += L"(";
			nmod = expression[++i];
			i++;
			for (int j=0;j<nmod;j++) {
				if (expression[i++]==Constant)
		      	ret += constants[expression[i]];
				else
					ret += expressions[expression[i]]->GetString();
				ret += ((j==nmod-1) ? L")" : L",");
				if (j<nmod-1) i++;
			}
		}
	}
	return ret;
}

wostream& CMPolynomial::write(wostream& os)
{
	return os << GetString();
}

wostream& CMPolynomial::debug(wostream& os)
{
	int i;
/*
	for (unsigned i=0;i<expression.Count();i++) {
		if (expression[i]==Constant) {
			i++;
			os << "Constant " << expression[i] << ": " << constants[expression[i]] << ENDL;
		}
		else if (expression[i]==Special) {
			i++;
			os << "Special " << expression[i] << ": " << specials[expression[i]].name << ENDL;
		}
		else if (expression[i]==Variable) {
			i++;
			int nmod = expression[i+1];
			os << "Variable " << expression[i] << ": " << varnames[expression[i]]
				<< "  " << nmod << " modifiers" << ENDL;
			i+=2;
			for (int j=0;j<nmod;j++) {
				os << "  Modifier " << (j+1) << ": ";
				switch (expression[i]) {
					case Lag:
					case Fwd:
						os << "_t";
						break;
					case LagSecond:
					case FwdSecond:
						os << "_s";
						break;
					case LagMinute:
					case FwdMinute:
						os << "_i";
						break;
					case LagHour:
					case FwdHour:
						os << "_h";
						break;
					case LagDay:
					case FwdDay:
						os << "_d";
						break;
					case LagWeek:
					case FwdWeek:
						os << "_w";
						break;
					case LagMonth:
					case FwdMonth:
						os << "_m";
						break;
					case LagYear:
					case FwdYear:
						os << "_y";
						break;
				}
				if (expression[i] >= Lag && expression[i] <= LagYear)
					os << '-';
				if (expression[i] >= Fwd && expression[i] <= FwdYear)
					os << '+';
				if (expression[++i]==Constant) {
					i++;
					os << "Constants " << expression[i] << ": " << constants[expression[i]] << ENDL;
				}
				else {
					i++;
					os << "Expression " << expression[i] << ": ";
					expressions[expression[i]]->write(os) << ENDL;
				}
				if (j<nmod-1)
					i++;
			}
		}
		else if (isop(expression[i]))
			os << "Operation " << ops[expression[i]-FirstOp] << ENDL;
		else if (isfunction(expression[i])) {
			int nargs = expression[i+1];
			os << "Function " << funcs[expression[i]-FirstFunc1].name
				<< "  " << nargs << " arguments" << ENDL;
			i+=2;
			for (int j=0;j<nargs;j++) {
				os << "Argument " << (j+1) << ": ";
				if (expression[i++]==Constant)
					os << "Constants " << expression[i] << ": " << constants[expression[i]] << ENDL;
				else {
					os << "Expression " << expression[i] << ": ";
					expressions[expression[i]]->write(os) << ENDL;
				}
				if (j<nargs-1) i++;
			}
		}
	}
*/
	write(os) << ENDL;
	os << L"Expression length =   " << expression.Count() << ENDL;
	os << L"Number of variables = " << varnames.Count() << ENDL;
	for (i=0;i<varnames.Count();i++)
		os << "   " << varnames[i] << ENDL;
	os << L"Number of constants = " << constants.Count() << ENDL;
	for (i=0;i<constants.Count();i++)
		os << "   " << constants[i] << ENDL;
	os << L"Number of embedded expressions = " << expressions.Count() << ENDL;
	for (i=0;i<expressions.Count();i++)
		expressions[i]->write(os) << ENDL;
	return os;
}


/* Evaluation functions */

double CMPolynomial::Evaluate(CMTimeMachine* t)
{
	if (Fail())
		return 0;

	time  = t;
	index = 0;
	double r;

	get_token();

	level1(r);

	return r;
}

void CMPolynomial::level1(double& result)    // logical
{
	register int op;
	double hold;

	level2(result);
	while (islogical(token)) {
		op = token;
		get_token();
		level2(hold);
		logical(op,result,hold);
	}
}

void CMPolynomial::level2(double& result)    // relational
{
	register int op;
	double hold;

	level3(result);
	while (isrelation(token)) {
		op = token;
		get_token();
		level3(hold);
		relation(op,result,hold);
	}
}

void CMPolynomial::level3(double& result)         //plus or minus
{
	register int op;
	double hold;

	level4(result);
	while (token == Plus || token == Minus) {
		op = token;
		get_token();
		level4(hold);
		arith(op,result,hold);
	}
}

void CMPolynomial::level4(double& result)  // multiplication, division, assign
{
	register int op;
	double hold;

	level5(result);
	while (token==Assign || token == Mul || token == Div || token == Mod) {
		op = token;
		get_token();
		level5(hold);
		arith(op,result,hold);
	}
}

void CMPolynomial::level5(double& result)     // powers
{
	double hold;

	level6(result);
	if (token == Power) {
		get_token();
		level6(hold);
		arith(Power,result,hold);
	}
}

void CMPolynomial::level6(double& result)    // unary operators
{
	int op = 0;

	if (token == Plus || token == Minus || token == Not) {
		op = token;
		get_token();
	}
	level7(result);
	if (op)
		unary(op,result);
}

void CMPolynomial::level7(double& result)      // functions, (), numbers,
{                                            // user functions, parameters
	if (token == LPar || token == LBracket) {
		get_token();
		level1(result);
	}
	else if (token == If)
		result = eval_condition();
	else if (token == Function)
		result = eval_function();
	else if (token==Variable||token==VariableOrig)
		result = eval_variable(token==Variable);
	else if (token == Constant)
		result = constants[offset];
	/*
	else if (token == Definition) 
	{
		result = CMDefinitions::GetDefinitionValue(offset);
		sdebug << "Definition " << offset << result << ENDL;
	}
	*/
	else if (isspecialfunc(token))
		result = eval_special_function();
	get_token();
}

void CMPolynomial::logical(int op,double& r,double& h)
{
	if (op == And || op == And2)
		r = (double) ((int) r && (int) h);
	else if (op == Or || op == Or2)
		r = (double) ((int) r || (int) h);
}

void CMPolynomial::relation(int op,double& r,double& h)
{
	switch (op) {
		case LessThan:	    r = (double) (r < h); break;
		case LessThanOrE:	r = (double) (r <= h); break;
		case GrThan:        r = (double) (r > h); break;
		case GrThanOrE:   	r = (double) (r >= h); break;
		case Equal:     	r = (fabs(r-h) < 0.00001) ? 1 : 0; break;
		case NotEqual:  	r = (fabs(r-h) < 0.00001) ? 0 : 1; break;
	}
}

void CMPolynomial::arith(int op,double& r,double& h)
{
	switch (op) {
		case Assign: r = h; break;
		case Minus:	r -= h; break;
		case Plus:  r += h; break;
		case Mul:  	r *= h; break;
		case Mod:   r = (double)((long)r%(long)h); break;
		case Power: r = pow(r,h); break;
		case Div:
			if (h == 0)
				report_error(XDivideByZero,GetString());
			r /= h;
			break;
	}
}

void CMPolynomial::unary(int op,double& r)
{
	if (op==Minus)
		r = -r;
	else if (op==Not)
		r = (r == 0) ? 1 : 0;
}

void CMPolynomial::get_token()
{
	token = expression[index++];

	if (token == Constant || token == Definition)
		offset = expression[index++];
	else if (token==If)
		nargs = expression[index++];
	else if (token == Variable || token == VariableOrig || token == Function) {
		offset = expression[index++];
		nargs = expression[index++];
	}
	else if (isspecialfunc(token))
		nargs = expression[index++];
}

double CMPolynomial::eval_variable(int current)
{
	double ret;

	// TODO: Fix to allow definitions
	if (variables[offset] == 0) {
		if (!(globalstate&ignoremissingvars))
			report_error(XMissingVariable,varnames[offset]);
		index += nargs*3;
		ret = 0;
	}
	else if (variables[offset]->IsA() == L"CMDefinition") {
		index += nargs * 3;
		ret = (((CMDefinition*)variables[offset])->GetValue());
	}
	else {
		int indexes[2];
		int timeoffset = 0;
		int i;
        CMTIMEUNIT interval;
		indexes[0] = indexes[1] = 0;
		int indexno = 0;
		for (i=0;i<nargs;i++) {
			int toktype = expression[index++];
			int c_or_e  = expression[index++];
			int listoff = expression[index++];
			int val = (int)eval_const_or_expression(c_or_e,listoff);
			if (toktype == ArrayIndex)
				indexes[indexno++] = val;
			else {
				switch (toktype) {
					case LagSecond: case FwdSecond: interval = CM_SECOND; break;
					case LagMinute: case FwdMinute: interval = CM_MINUTE; break;
					case LagHour: case FwdHour: interval = CM_HOUR; break;
					case LagDay: case FwdDay: interval = CM_DAY; break;
					case LagWeek: case FwdWeek: interval = CM_WEEK; break;
					case LagMonth: case FwdMonth: interval = CM_MONTH; break;
					case LagYear: case FwdYear: interval = CM_YEAR; break;
					default: interval = -1; break;
				}
				timeoffset = (toktype >= Lag && toktype <= LagYear) ? -val : val;
				time->Offset(timeoffset,interval);
			}
		}
		ret = ((CMVariable*)variables[offset])->GetValue(time,current,indexes[0],indexes[1]);
		if (timeoffset)
			time->Offset(-timeoffset,interval);
	}
	return ret;
}

double CMPolynomial::eval_special_function()
{
	int base = index;
   index += nargs*2;
	int c_or_e  = expression[base];
	int listoff = expression[base+1];
   int beglag = (int)eval_const_or_expression(expression[base+2],expression[base+3]);
   int ENDLag = (int)eval_const_or_expression(expression[base+4],expression[base+5]);
	int sign = (beglag<ENDLag) ? 1 : -1;
	time->Offset(beglag,-1);
	double ret = (token==FuncProd) ? 1 : 0;
	int i = beglag;
	while (1) {
		double r = eval_const_or_expression(c_or_e,listoff);
		if (token==FuncSum) ret += r;
		else if (token==FuncProd) ret *= r;
   	if (i==ENDLag)
      	break;
		time->Offset(sign,-1);
      i+=sign;
	}
	time->Offset(-ENDLag,-1);
   return ret;
}

double CMPolynomial::eval_function()
{
	double parms[100];
   int i;
	for (i=0;i<nargs&&i<100;i++) {
		int c_or_e  = expression[index++];
		int listoff = expression[index++];
		parms[i] = eval_const_or_expression(c_or_e,listoff);
	}
	return functions[offset](parms,nargs);
}

double CMPolynomial::eval_condition()
{
	int c_or_e[3];
	int listoff[3];
   int i;

	for (i=0;i<nargs&&i<3;i++) {
		c_or_e[i]  = expression[index++];
		listoff[i] = expression[index++];
	}

	double condition = eval_const_or_expression(c_or_e[0],listoff[0]);

	if (condition != 0 && nargs>1)
		return eval_const_or_expression(c_or_e[1],listoff[1]);
	else if (condition == 0 && nargs>2)
		return eval_const_or_expression(c_or_e[2],listoff[2]);

	return 0;
}

CMPolynomialIterator::CMPolynomialIterator(CMPolynomial& aExp) :
eIterator(0),
vIterator(0),
exp(aExp),
state(iterNames),
pos(0)
{
}

CMPolynomialIterator::~CMPolynomialIterator()
{
	if (eIterator) delete eIterator;
	if (vIterator) delete vIterator;
}

const wchar_t* CMPolynomialIterator::GetNext()
{
	const wchar_t* ret;

	if (!exp.ContainsVariables())
		return 0;

	if (state == iterNames) {
		if (pos < exp.varnames.Count())
			return exp.varnames[pos++].c_str();
		state = iterExpressions;
		pos=0;
	}

	if (state == iterExpressions) {
		while (pos < exp.expressions.Count()) {
			if (!eIterator)
				eIterator = exp.expressions[pos]->ContainsVariables() ? new CMPolynomialIterator(*exp.expressions[pos]) : 0;
			if (eIterator && (ret=eIterator->GetNext())!=0)
				return ret;
			if (eIterator) delete eIterator;
			eIterator = 0;
			pos++;
		}
		state = iterVariables;
		pos=0;
	}

	if (state == iterVariables) {
		while (pos < exp.variables.Count()) {
			if (exp.variables[pos]) {
				if (!vIterator)
					vIterator = exp.variables[pos]->CreateIterator();
				if (vIterator) {
					if ((ret = vIterator->GetNext())!=0)
						return ret;
					vIterator = 0;
				}
			}
			pos++;
		}
	}

	return 0;
}




