// irpapp.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMIrpApplication is the main IRPSIM class and the point of entry for all
// IRPSIM functionality
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

#include "cmdefs.h"
#include "smparray.h"
#include "smvarray.h"
#include "units.h"

#include "irp.h"
#include "options.h"
#include "vartypes.h"
#include "script.h"
#include "category.h"
#include "scenario.h"
#include "simulat.h"
#include "variable.h"
#include "varcol.h"

#include <fstream>
using namespace std;

class _IRPCLASS CMReliability;
class _IRPCLASS CMSimulationArray;

class _IRPCLASS CMIrpApplication
{
protected:
	//******** DATA
	string m_strProjectFile;
    CMScript* currentscript;
    CMScenario* currentscenario;
	int errorcode;
	CMOptions options;
	CMVariableTypes vartypes;
	CMVariableCollection* oldvariables;
	CMVariableCollection* variables;

	struct _pct_complete_struct {
		int nVal;
		const wchar_t* pTitle;
	};
	
	class _IRPCLASS CMPSmallArray<CMScenario> scenarios;
	class _IRPCLASS CMPSmallArray<CMScript> scripts;
	class _IRPCLASS CMPSmallArray<CMSimulation> simulations;
	class _IRPCLASS CMVSmallArray<string> outputvars;
	class _IRPCLASS CMVSmallArray<string> loadedfiles;
	//class _IRPCLASS CMVSmallArray<string> attachedfiles;

	int casesensitiveflag;
	int skipwhitespaceflag;

	//********

	// Protected virtual functions

	//static DWORD WINAPI add_file_proc(LPVOID lpParameter);
	void CMIrpApplication::update_variable_links();
	int add_file_to_list(const string& name); // adds a file to appropriate list
	int read_file(const string& name,int& varsread);

	void SetVariableStateAll(ULONG aState, BOOL action);
	void UpdateVariableLinksAll();
	void UpdateVariableLinkStatusAll();
	//int DestroyVariable(const string& vname);
	void ResetOutputVariables() { outputvars.Reset(1); }
	void AddOutputVariable(const string& name) { outputvars.Add(name); }
	string OutputVariable(unsigned short n) { if (n<outputvars.Count()) return outputvars[n]; return string(); }
	unsigned short OutputVariables() const { return outputvars.Count(); }
	int IsOutputVariable(const string& name) { return outputvars.Contains(name); }

	enum {EUnknownError=1,EReadingFile,EOpeningFile,ENoSimFileName,
			ESimFileNameInUse,ESimInProgress,EMissingVariables};
public:
	enum {IRP_VARIABLE=1,IRP_SCENARIO,IRP_CATEGORY,IRP_SCRIPT,SYNC_FILE_ADDED,
		SYNC_FILE_ATTACHED,SYNC_SCENARIO_ADDED,SYNC_SIMULATION_UPDATE,SYNC_PCT_COMPLETE};

	CMIrpApplication();
	~CMIrpApplication();

	// Open an IRPSIM project, which is essentially a collection of files (vardef, scripts, etc.)
	void OpenProject(const string& name);
	//void AddFiles(string *pNames,int nFiles);
	//void DeleteFiles(string *pNames,int nFiles);

	CMOptions& Options() {return options;}
	void SetOptions(const CMOptions& op);

	// void SetLogFile(const string& fname);

	string GetProjectFile() { return m_strProjectFile; }
	//string GetProjectName();
	
	
	// Get the name (full path) of the file containing an IRPSIM object
	string GetObjectFileName(CMIrpObject* pObject); 

	unsigned short ScenariosCount() {return scenarios.Count();}
	CMScenario* Scenario(unsigned short n) {return (n<scenarios.Count()) ? scenarios[n] : 0;}
	//CMScenario* AddThisScenario(const string& name);
	CMScenario* UseScenario(const string& name);
    // void RemoveScenario(unsigned short n);
    //void AddScenario(CMScenario* sce);
	CMScenario* CurrentScenario() {return currentscenario;}

	unsigned short ScriptsCount() {return scripts.Count();}
	CMScript* Script(unsigned short n) {return (n<scripts.Count()) ? scripts[n] : 0;}
	CMScript* UseScript(const string& name);
    //void RemoveScript(unsigned short n);
    // void AddScript(CMScript* scr);
	CMScript* CurrentScript() {return currentscript;}

   unsigned short Categories() {return CMCategory::CategoryCount();}
   CMCategory* Category(unsigned short n) {return CMCategory::GetCategory(n);}

	CMScript*	FindScript(const string& name);
	CMCategory* FindCategory(const string& name);
	CMScenario* FindScenario(const string& name);

	CMIrpObject* FindIrpObject(const string& name);
		
   //CMVariable* AddVariable(const string& vdef);
   CMVariableCollection* VariableCollection() {return variables;}

	//Call to create a new simulation

    CMSimulation* CreateSimulation(); //const wchar_t* pFileName = 0);
	BOOL RunSimulation(CMSimulation* pSim);
	BOOL PauseSimulation(CMSimulation* pSim,BOOL bAction);
	CMSimulation* RunningSimulation();

	//int WriteOptions(const string& filename);
	//int WriteScenarios(const string& filename);
	//int WriteScripts(const string& filename);
	//int WriteVariableDefs(const string& filename);
	
	int WriteOutcomes(const string& filename,CMSimulation* sim);
	int WriteSummary(const string& filename,CMSimulation* sim);

	void ResetApplication();

	int LoadedFilesCount() {return loadedfiles.Count();}
	string LoadedFile(unsigned short n) {if (n<loadedfiles.Count()) return loadedfiles[n]; return string();}

	//int AttachedFiles() {return attachedfiles.Count();}
	//string AttachedFile(unsigned short n) {if (n<attachedfiles.Count()) return attachedfiles[n]; return string();}

	//int IsApplicationFile(const string& filename);

	void DeleteSimulation(CMSimulation* pSim,int save);
	unsigned SimulationsCount();

	//virtual void SetProgramParameter(const string& parm, const string& val) = 0;
	//virtual string GetProgramParameter(const string& parm) = 0;
	// use InfoMessage for "quick and dirty" messages
	//virtual void InfoMessage(const string& message) = 0;
	//virtual void ErrorMessage(const string& msg) = 0;
	// use ProgressMessage to add to previous messages. Call with message=0 to terminate
	
	// Synchronize is called by various threads for various reasons
	// lpParameter is set depending on syncType
	// return TRUE if user has cancelled operation
	//virtual BOOL Synchronize(int syncType, const void* lpParameter) { return FALSE; }
};
