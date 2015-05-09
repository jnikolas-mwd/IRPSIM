// variable.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVariable is the virtual base class for simulation variables.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__CMVARIABLE_H)
#define __CMVARIABLE_H

#include "irp.h"
#include "timemach.h"
#include "assoc.h"
#include "irpobject.h"
#include "cmstring.h"
#include "cmdefs.h"
#include "tmarray.h"
#include "smparray.h"
#include "phashdic.h"
#include "strval.h"
#include "queue.h"

class _IRPCLASS CMVNameIterator
{
	friend class _IRPCLASS CMVariable;
	class _IRPCLASS CMPSSmallArray<CMString> varray;
	int vpos;
	int reset();
protected:
	CMVariable* variable;
	virtual const wchar_t* get_next() {return 0;}
public:
	CMVNameIterator(CMVariable* v);
	~CMVNameIterator();
	const wchar_t* GetNext();
};

class _IRPCLASS CMVariable : public CMIrpObject
{
	friend class _IRPCLASS CMVariableIterator;
	friend class _IRPCLASS CMVariableCollection;
//	friend int _IRPFUNC operator < (const CMVariable& v1,const CMVariable& v2);

	// Data elements ***************

	int type;
	ULONG state;
	int errorcode;
	CMVNameIterator* iterator;
	//CMPSMALLSTRINGARRAY types;
	CMPSmallArray<CMString> types;

	CMAssociations associations;
   int region;

	CMValueQueue* realized;

	// ******************************

	static CMVariableCollection* variables;
	static wchar_t* errorstrings[];
	static int static_state;
	static int sort_method;

	// reset_trial is called at the beginning of each simulation trial
	void	 reset_trial(CMTimeMachine* t);
	void 	 delete_iterator() {if (iterator) delete iterator;iterator=0;}
	void   set_value_queue(int sz); // set variable to maintain realized values;
	static CMString get_next_eval_type(wistream& s,CMString& aname);
   enum {NOTYPE=32000};
protected:
	static const wchar_t* vardef_begin;
	static const wchar_t* vardef_end;
	static int output_precision;
	static int column_width;
	// Virtual functions that must be overridden in each derived class
	// Evaluate variables. Indexes depend on nature of derived variable class

	// virtual function to perform any addition resetting at beginning of
	// a simulation trial
	virtual void reset(CMTimeMachine* t) {}
	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0) {return 0;}
	virtual int is_less_than(const CMIrpObject& v2) const;
	virtual CMVNameIterator* create_iterator();
	virtual void update_variable_links() {}
	virtual void read_body(wistream& s);
	virtual void write_body(wostream& s) {}
	virtual wistream& read(wistream& s);
	virtual wostream& write(wostream& s);
	virtual const wchar_t* CMVariable::IsA();
public:
	static const ULONG vsFailed;
	static const ULONG vsStochastic;
	static const ULONG vsSystem;
	static const ULONG vsDontDestroy;
  	static const ULONG vsAggregate;
	static const ULONG vsRegional;
	static const ULONG vsEvaluating;
	static const ULONG vsIterating;
	static const ULONG vsCircularity;
	static const ULONG vsDontMaintain;
	static const ULONG vsAlwaysEvaluate;
	static const ULONG vsLinksUpdated;
	static const ULONG vsSelected;
	static const ULONG vsLinked;
	static const ULONG vsSaveSummary;
	static const ULONG vsSaveOutcomes;
   static const ULONG vsOutput;
	static const ULONG vsGraph;
	static const ULONG vsSum;
	static const ULONG vsInteger;
	static const ULONG vsMoney;
	static const ULONG vsNoUnits;
	static const ULONG vsResetRequired;
	static const ULONG vsCarryForward;
   static const ULONG vsInUse;
	static const ULONG vsAutoCreated;
	static const ULONG vsMonitor;
	static const ULONG vsAccumulate;
	static const ULONG vsDontEdit;
	static const ULONG vsFlag; // set this for bulk operations

	enum {byName,byType,byDependency};

	enum {XAlreadyExists,XIllegalName,XCircularity,XMisplacedEndvar,
			XIncorrectNumRows,XIncorrectNumColumns,XBadVardef,
			XIllegalTimeLag,XTimeOutOfRange,XNoAssociatedVariable};

	CMVariable(const CMString& aName,ULONG astate=0L,int id=-1);
	~CMVariable();

	void   ReportError(int c,CMTime* t=0);
	void   ReportError(int c,const CMString& st,CMTime* t=0);
	virtual CMString VariableType() {return GetEvalType();}
	// For setting variable-specific quantities
   virtual void SetSpecialValues(CMTimeMachine* t,double v1=0,double v2=0,double v3=0,double v4=0) {}

	int Fail() {return (state&vsFailed) ? 1 : 0;}
   void  SetRegion(int r) {region=r;}
	int	GetRegion() {return region;}
//   void  SetApplicationId(int id) {app_id=id;}
//	int   GetApplicationId() {return app_id;}
	ULONG GetState() const {return state;}
	BOOL  GetState(ULONG aState) const {return (state&aState) ? TRUE : FALSE;}
	BOOL  ToggleState(ULONG aState);
	ULONG SetState(ULONG aState,BOOL action);
//	const CMString& GetName() {return name;}
	double GetValue(CMTimeMachine* t,int current=1,int index1=0,int index2=0,int force_evaluation=0);
	void   SetValue(CMTimeMachine* t,double val);
	void   AddTo(CMTimeMachine* t,double val)
		{double v=GetValue(t,1);v+=val;SetValue(t,v);}

	void   AddAssociation(const CMString& aName,const CMString& val);
	CMString GetAssociation(const CMString& aName);
	int 	 GetAssociation(int n,CMString& s1,CMString& s2);

	void   SetType(const CMString& aName);
	void   SetType(int val); // for setting negative types
	int	 IsType(const CMString& aName);
	CMString GetSpecialType();

	void	UpdateVariableLinks();
	void	UpdateLinkStatus();

	CMVNameIterator* CreateIterator();

// ------------- Static methods

	static const wchar_t* GetEvalType() { return L"CMVariable"; }
	static CMVariable* Make(wistream& s);
	static CMVariable* Find(const CMString& aName);
	static void SetSortMethod(int meth) {sort_method = meth;}

	static void ResetTrial();
    static CMVariableCollection* SetCollectionContext(CMVariableCollection* c)
   	{CMVariableCollection* oldcontext=variables;variables=c;return oldcontext;}
};

/*
inline int operator == (const CMVariable& v1,const CMVariable& v2)
{
	CMString::set_case_sensitive(0);return (v1.name==v2.name);
}

inline istream& operator >> (istream& s, CMVariable& v)
{
	return v.Read(s);
}

inline ostream& operator << (ostream& s, CMVariable& v)
{
	return v.Write(s);
}
*/

inline wistream& operator >> (wistream& s, CMVariable*& v )
{
	v=CMVariable::Make(s);return s;
}

#endif
