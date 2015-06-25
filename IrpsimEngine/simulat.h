// simulat.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMSimulation implements IRPSIM's simulation engine
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__SIMULAT_H)
#define __SIMULAT_H

#include "irp.h"
#include "options.h"
#include "variable.h"
#include "simarray.h"
#include "accum.h"

#include "cmdefs.h"
#include "smparray.h"
#include "smvarray.h"
#include "bgparray.h"
//#include <windows.h>

class _IRPCLASS CMIrpApplication;
class _IRPCLASS CMReliability;
class _IRPCLASS CMScript;
class _IRPCLASS CMTimeMachine;

class _IRPCLASS CMSimulation : public CMIrpObject
{
	static const wchar_t* forbidden_set_options[];
	static const wchar_t *file_header;
	static const wchar_t *file_footer;
   static CMSimulation* active_simulation;
	
	//static DWORD WINAPI SimRunProc(LPVOID lpParameter);

	enum {sStopped=0x0001,sPaused=0x0002,sRunning=0x0004,
			sFromFile=0x0008,sInitialized=0x0010,
         sMissingVariables=0x0100,sCantOpenBinaryFile=0x0200,
         sBadBinaryFile=0x0400,sBadScript=0x0800};

	//HANDLE hRunEvent;   // Event to start simulation running.
	//DWORD  dwThreadId;  // Id of simulation run thread.

	short state;
	CMString filename, comments; //, tempfile
	long trialno;
	long elapsedtime;
	CMTime loadtime;
	CMTime begintime;
	CMIrpApplication* pApp;
	CMScript* script;
	long ntrials;
   long randomseed;

   class _IRPCLASS CMPSSmallArray<CMVariable> 	costvars;
   class _IRPCLASS CMPSSmallArray<CMVariable> 	summaryvars;
   class _IRPCLASS CMPSSmallArray<CMVariable> 	outcomevars;
   class _IRPCLASS CMVSmallArray<CMString>		missingvars;
   //class _IRPCLASS CMVSmallArray<CMString>		loadedfiles;

	CMSimulationArray*  simarray;
	CMAccumulatorArray* accumulator;
	CMReliability* 	  reliability;
	CMTimeMachine*		  timemachine;
	CMOptions	  		  options;

	double get_cost(int region = -1);
   void set_variables_inuse(BOOL action);
	int  find_missing_variables();
	void get_data_from_options();
   //void save_simulation_tail(wostream& s,int startpoint);
   int  get_vardesc_state(CMVariable* v);
	void initialize();
protected:
	virtual const wchar_t* IsA() { return L"CMSimulation"; }
	//friend class CMIrpApplication;
	// These functions should only be called by CMIrpApplication;
	//CMSimulation(const CMString& fname,short mode=0,CMIrpApplication* a=0);
public:
	CMSimulation(CMIrpApplication* a);
	~CMSimulation();

	BOOL Run();
	void Pause(BOOL action) { if (action) state |= sPaused; else state &= ~sPaused; }
	void Stop()   { state |= sStopped; }

	//BOOL LoadSimulationFile(const CMString& fname,short mode=0);

	static CMSimulation* ActiveSimulation() {return active_simulation;}

	//MString GetName() const {return simname;}
	CMString GetFileName() const {return filename;}

	void Reset();
	//int  SaveTo(const CMString& fname);
	//void Save();
	BOOL Fail()    {return (state&0xFF00) ? TRUE : FALSE;}
	BOOL Paused()  {return (state&sPaused) ? TRUE : FALSE;}
	BOOL Stopped() {return (state&sStopped) ? TRUE : FALSE;}
	BOOL Running() {return (state&sRunning) ? TRUE : FALSE;}
	//BOOL FromFile() {return (state&sFromFile) ? TRUE : FALSE;}
	BOOL Initialized() {return (state&sInitialized) ? TRUE : FALSE;}

   int UsesVariable(const CMString& varname);

	CMTime BeginTime() {return begintime;}
   CMTime CurrentTimeStep() {return timemachine->Now();}
	const CMTimeMachine* TimeMachine() {return timemachine;}
	void GetOptions(CMOptions& op) {op = options;}
	void SetOptions(const CMOptions& op);
	CMString GetOption(const CMString& opname) {return options.GetOption(opname);}
	int GetOptionInt(const CMString& opname) { return options.GetOptionInt(opname); }
	double GetOptionDouble(const CMString& opname) { return options.GetOptionDouble(opname); }
	const wchar_t* GetOptionString(const CMString& opname) { return options.GetOption(opname).c_str(); }
	void SetOption(const CMString& opname,const CMString& opval);
   CMString GetComments() {return comments;}
   void SetComments(const CMString& c) {comments=c;}
	
	void SetScript(const CMString& sname);

	//int LoadedFiles() {return loadedfiles.Count();}
	//CMString LoadedFile(unsigned short n) {if (n<loadedfiles.Count()) return loadedfiles[n]; return CMString();}
    //int RebuildFiles(const CMString& path);

	long   ElapsedSeconds() {return elapsedtime;}
	long   Trial() {return trialno;}
	long   Trials() {return ntrials;}
	CMSimulationArray* SimArray() 	  {return simarray;}
	CMAccumulatorArray* Accumulator()  {return accumulator;}
	CMReliability* Reliability()       {return reliability;}

	void IgnoreMissingVariables();
	int MissingVariables() {return missingvars.Count();}
	CMString MissingVariable(unsigned short n) {if (n<missingvars.Count()) return missingvars[n];return L"";}
};

inline int operator == (const CMSimulation& s1,const CMSimulation& s2)
{
	return s1.GetFileName() == s2.GetFileName();
}

inline int operator < (const CMSimulation& s1,const CMSimulation& s2)
{
	return s1.GetFileName() < s2.GetFileName();
}

#endif
