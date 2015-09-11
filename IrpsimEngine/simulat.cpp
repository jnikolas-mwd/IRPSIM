// simulat.cpp : implementation file
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
#include "StdAfx.h"
#include "simulat.h"
#include "irpapp.h"
#include "regions.h"
#include "variable.h"
#include "varcol.h"
#include "vartypes.h"
#include "expresn.h"
#include "reliab.h"
#include "node.h"
#include "vardesc.h"
#include "notify.h"
#include "defines.h"
#include "./zip/zip.h"

#include "token.h"
#include "random.h"
#include "cmlib.h"
#include <time.h>

//#include <fstream>
//static wofstream sdebug("debug_simulat.txt");

using namespace std;

#define CM_BIGTIME 10000000L

/*
const wchar_t* CMSimulation::forbidden_set_options[] = {
L"demandcategories",
L"supplycategories",
L"transfercategories",
L"storagecategories",
L"numtrials",
L"simbegin",
L"simend",
L"tracebegin",
L"simulationfile",
L"traceend",
L"tracestart",
L"tracemode",
L"yearend",
L"saveincrementsize",
L"randomseed",
0
};
*/

CMSimulation* CMSimulation::active_simulation = 0;
BOOL CMSimulation::_saveArchive = TRUE;
BOOL CMSimulation::_saveOutcomes = TRUE;
BOOL CMSimulation::_saveSummary = TRUE;

CMSimulation::CMSimulation(CMIrpApplication* a) :
pApp(a),
script(0),
options(*a->Options()),
costvars(),
outcomevars(),
summaryvars(),
missingvars(),
timemachine(0),
simarray(0),
accumulator(0),
reliability(0),
trialno(0),
elapsedtime(0),
state(0)
//hRunEvent(NULL),
//dwThreadId(0L)
{
	loadtime.SetOutputFormat(CMTime::YYYY_MM_DD_HHMMSS);
	timemachine = new CMTimeMachine(CMTime::StringToTimeUnit(options.GetOption(L"siminterval")), 1);
	m_strProjectRoot = extractpath(a->GetProjectFile());
	get_data_from_options();
}

CMSimulation::~CMSimulation()
{
	set_variables_inuse(FALSE);
	if (timemachine) delete timemachine;
	if (simarray) 	  delete simarray;
	if (accumulator) delete accumulator;
	if (reliability) delete reliability;
   //if (tempfile.length()) removefile(tempfile);
   if (active_simulation==this)
   	active_simulation=0;
}

void CMSimulation::initialize()
{
   unsigned short i;

	//tempfile = createtempfile(L"irp",0);

	get_data_from_options();
	ntrials = options.GetOptionLong(L"numtrials");

	CMExpression::IgnoreMissingVariables(0);

   if (timemachine) delete timemachine;
	timemachine = new CMTimeMachine(CMTime::StringToTimeUnit(options.GetOption(L"siminterval")),1);

	begintime = CM_BIGTIME;

   CMString yearendstr = options.GetOption(L"yearend");
	int yearend = yearendstr.length() ? CMTime::Month(yearendstr.c_str()) : 12;

	timemachine->AddCycle(options.GetOption(L"simbegin").c_str(),
                         options.GetOption(L"simend").c_str(),0,yearend);

	timemachine->AddCycle(options.GetOption(L"tracebegin").c_str(),
                         options.GetOption(L"traceend").c_str(),
								 options.GetOption(L"tracestart").c_str(),yearend,
								 options.GetOption(L"tracemode")==L"dynamic" ? CM_YEAR : CM_NOTIMEUNIT);

	reliability = new CMReliability(*timemachine,options.GetOption(L"reliability"));

	wifstream is;

	// Save all loaded files in text form to a single file "tempfile"
	// files are separated by "file_header" and "file_footer" lines

	/*
    wofstream os(tempfile.c_str(),ios::out|IOS_BINARY);
	for (i = 0; pApp && i<pApp->LoadedFilesCount(); i++) {
		loadedfiles.Add(pApp->LoadedFile(i));
		CMTokenizer next(pApp->LoadedFile(i));
      CMString name = next(L" \t\r\n");
      is.open(name.c_str(),ios::in | IOS_BINARY);
      if (!is.fail()) {
		  os << file_header << L" " << pApp->LoadedFile(i) << ENDL;
         is >> os.rdbuf();
			os << ENDL << file_footer << ENDL;
      }
      is.close();
   }
   */

	if (script) {
		script->SetSimulationContext(this, pApp);
		if (script->Fail()) {
			CMNotifier::Notify(CMNotifier::ERROR, CMString(L"problem with script <" + script->GetName() + L">"));
			state |= sBadScript;
		}
   }
	else {
		CMNotifier::Notify(CMNotifier::ERROR, (L"can't run simulation: no script loaded"));
		state |= sBadScript;
	}

	CMVariableIterator iter;
	CMVariable* v;

	summaryvars.Add(CMVariable::Find(L"_tperiod"));
	outcomevars.Add(CMVariable::Find(L"_tperiod"));

	while ((v=iter())!=0) {
		if (v->GetState(CMVariable::vsSystem)==TRUE)
      		continue;
		if (v->GetState(CMVariable::vsSelected|CMVariable::vsSaveOutcomes) == TRUE) {    /*TODO This used to be GetState(CMVariable::vsSelected*/
			outcomevars.Add(v);
			summaryvars.Add(v);
			if (v->IsType(L"cost"))
				costvars.Add(v);
		}
	}

	if (find_missing_variables()) {
		CMNotifier::Notify(CMNotifier::ERROR, (L"can't run simulation: missing variables"));
		state |= sMissingVariables;
	}

	summaryvars.Sort();
	outcomevars.Sort();

	long incsize = options.GetOptionLong(L"saveincrementsize")<<10;

	accumulator = new CMAccumulatorArray(*timemachine,summaryvars.Count());
	simarray = new CMSimulationArray(*timemachine,outcomevars.Count(),ntrials);

	if (!simarray || simarray->Fail()) {
		state |= sCantOpenBinaryFile;
	}
	for (i=0;i<summaryvars.Count() && accumulator;i++)
		accumulator->AssignVariable(i,summaryvars[i]->GetName(),summaryvars[i]->GetSpecialType(),get_vardesc_state(summaryvars[i]));
	for (i=0;i<outcomevars.Count() && simarray;i++)
		simarray->AssignVariable(i,outcomevars[i]->GetName(),outcomevars[i]->GetSpecialType(),get_vardesc_state(outcomevars[i]));
	set_variables_inuse(TRUE);
	state |= sInitialized;
}

void CMSimulation::SetScript(const CMString& sname)
{
	// return if simulation is already initialized (or loaded from a file)
	if (state&(sInitialized))
		return;

	script = pApp->FindScript(sname);
}

void CMSimulation::Reset()
{
	//if (FromFile() || Fail())
	if (Fail())
		return;
	begintime = CM_BIGTIME;
	trialno = 0;
	elapsedtime = 0;
	timemachine->Reset();
	if (simarray) {
   		simarray->Reset();
    }
	if (accumulator) accumulator->Reset();
	if (reliability) reliability->Reset();
	state &= ~(sStopped | sRunning);
	state |= sPaused;
}

void CMSimulation::IgnoreMissingVariables()
{
	CMExpression::IgnoreMissingVariables(1);
	state &= ~sMissingVariables;
}

int CMSimulation::UsesVariable(const CMString& varname)
{
	for (unsigned i=0;i<summaryvars.Count();i++)
   	if (varname == summaryvars[i]->GetName())
      	return 1;
   return 0;
}

void CMSimulation::set_variables_inuse(BOOL action)
{
	for (unsigned i=0;i<summaryvars.Count();i++)
   	summaryvars[i]->SetState(CMVariable::vsInUse,action);
}

int CMSimulation::get_vardesc_state(CMVariable* v)
{
	int ret = 0;
	if (!v) return 0;
   if (v->GetState(CMVariable::vsSum)) 	 ret |= CMVariableDescriptor::vdSum;
   if (v->GetState(CMVariable::vsMoney)) 	 ret |= CMVariableDescriptor::vdMoney;
   if (v->GetState(CMVariable::vsNoUnits)) ret |= CMVariableDescriptor::vdNoUnits;
   if (v->GetState(CMVariable::vsInteger)) ret |= CMVariableDescriptor::vdInteger;
   if (v->GetState(CMVariable::vsOutput))  ret |= CMVariableDescriptor::vdOutput;
   if (v->GetState(CMVariable::vsGraph))   ret |= CMVariableDescriptor::vdGraph;
   return ret;
}

int CMSimulation::find_missing_variables()
{
	CMVariableIterator iter;
	CMVariable* v;
	CMIrpObjectIterator* vi;
	while ((v=iter())!=0) {
		if (v->GetState(CMVariable::vsSelected)) {
			vi = v->CreateIterator();
			const wchar_t* vname;
			while (vi && ((vname=vi->GetNext())!=0)) {
				if (!CMVariable::Find(vname) && !CMDefinitions::IsDefined(vname)) {
					if (!missingvars.Contains(vname)) {
						missingvars.Add(vname);
						CMNotifier::Notify(CMNotifier::ERROR, L"Missing variable or definition " + CMString(vname) + L" in definition of " + v->GetName());
					}
				}
			}
			CMString atype,aname;
			for (int i=0;v->GetAssociation(i,atype,aname);i++) {
				CMVariable* vassoc = CMVariable::Find(aname);
				if (vassoc) {
					vi = vassoc->CreateIterator();
					while (vi && ((vname = vi->GetNext()) != 0)) {
						if (!CMVariable::Find(vname)) {
							if (!missingvars.Contains(vname)) {
								missingvars.Add(vname);
								CMNotifier::Notify(CMNotifier::ERROR, L"Missing variable or definition " + CMString(vname) + L" in definition of " + v->GetName());
							}
						}
					}
				}
			}
		}
	}
	return missingvars.Count();
}

void CMSimulation::SetOptions(const CMOptions& op)
{
	for (unsigned i=0;i<op.Count();i++)
		SetOption(op.At(i)->GetName(),op.At(i)->GetValue());
}

void CMSimulation::SetOption(const CMString& opname,const CMString& opval)
{
	// return if simulation has been initialized and option is from
	// set of options that can't be changed after initialization
	//if (contains(opname.c_str(),forbidden_set_options,500,0)>=0 && (state&sInitialized))
	//	return;
	options.SetOption(opname,opval);
	get_data_from_options();
}

void CMSimulation::get_data_from_options()
{
	//m_strFileName = options.GetOption(L"simulationfile");
	CMString simname  = options.GetOption(L"simulationname");
	if (!simname.length()) simname = L"simulation";
	ntrials = options.GetOptionLong(L"numtrials");
	m_strOutputRoot = getabsolutepath(m_strProjectRoot.c_str(), options.GetOption(L"outputfolder").c_str());
	wchar_t lastchar = m_strOutputRoot[m_strOutputRoot.length() - 1];
	if (lastchar != L'\\' && lastchar != L'/')
		m_strOutputRoot += L"\\";
	m_strOutputRoot += simname;

	int result = _wmkdir(m_strOutputRoot.c_str());

	if (result == ENOENT)
		CMNotifier::Notify(CMNotifier::WARNING, L"Unable to create output directory " + m_strOutputRoot);

    name = simname + L" " + loadtime.GetString();

   //if (!simname.length()) simname = L"<no name>";
	randomseed = options.GetOptionLong(L"randomseed");
	if (randomseed<0) randomseed=0;
}

double CMSimulation::get_cost(int region)
{
	double ret = 0;
	for (unsigned i=0;i<costvars.Count();i++) {
		if (region<0 || costvars[i]->GetRegion()==region)
      	ret += costvars[i]->GetValue(timemachine);
   }
   return ret;
}


// Single-threaded Run Method
// Returns true on success and false on fail
BOOL CMSimulation::Run()
{
	time_t inittime = time(NULL);
	long initelapsed = elapsedtime;
	
	if (!(state&sInitialized))
		initialize();

	while ((trialno = timemachine->Count())<ntrials && !Fail() && !(state&(sStopped | sFromFile))) 
	{
		int atbeginning = timemachine->AtBeginning();

		if (atbeginning && (state&sPaused))
			break;

		state |= sRunning;

		if (atbeginning) {
			CMVariable::ResetTrial();
			if (trialno == 0 && pApp)
				CMNotifier::Notify(CMNotifier::LOGTIME, L"Start Simulation " + GetName());
			CMNotifier::Notify(CMNotifier::PROGRESS, L"", (int)(100 * (double)trialno / (double)ntrials));
		}

		if (script) script->Run(timemachine);

		for (int i = 0; i<outcomevars.Count(); i++)
			simarray->AddAt(*timemachine, i, trialno, (float)outcomevars[i]->GetValue(timemachine));
		for (int i = 0; i<summaryvars.Count(); i++)
			accumulator->AddAt(*timemachine, i, summaryvars[i]->GetValue(timemachine));

		if (reliability) reliability->Process(timemachine);

		elapsedtime = initelapsed + (long)time(NULL) - (long)inittime;
		
		//if (pApp) pApp->Synchronize(CMIrpApplication::SYNC_SIMULATION_UPDATE, this);

		timemachine->Step();
	}
	
	elapsedtime = initelapsed + (long)time(NULL) - (long)inittime;
	if (trialno >= ntrials) {
		CMNotifier::Notify(CMNotifier::PROGRESS, L"Completed", 100);
		CMNotifier::Notify(CMNotifier::LOGTIME, L"End Simulation " + GetName());
		state |= sStopped;
		trialno--;
		if (pApp) {
			CMString fileName;

			if (_saveArchive)
			{
				fileName = m_strOutputRoot + L"\\archive-" + GetId() + L".zip";
				CMNotifier::Notify(CMNotifier::LOGTIME, L"Writing archive to " + fileName);

				HZIP hz = CreateZip(fileName.c_str(), 0);

				wchar_t* tempFileName = _wtempnam(L".\\", L"irpsim_temp");
				wofstream os(tempFileName);

				os << L"Simulation: " << GetName() << endl;
				os << pApp->GetVersionInfo() << endl;
				if (pApp->CurrentScript() != NULL)
					os << L"Using Script " << pApp->CurrentScript()->GetName() << endl << endl;

				os << L"Files Used:" << endl << endl;

				for (int i = 0; i < pApp->LoadedFilesCount(); i++) {
					fileName = pApp->LoadedFile(i);
					os << strippath(fileName).c_str() << endl;
				}
				
				os << endl << this->options << endl << endl;

				os << L"#SCENARIO OutputVariables" << endl;

				for (unsigned i = 0; i < this->accumulator->Variables(); i++)
					os << this->accumulator->GetVariableName(i) << endl;

				os << "#END" << endl;

				os.close();

				ZipAdd(hz, L"README.TXT", tempFileName);

				if (tempFileName) {
					_wremove(tempFileName);
					free(tempFileName);
				}

				for (int i = 0; i < pApp->LoadedFilesCount(); i++)
				{
					fileName = pApp->LoadedFile(i);
					ZipAdd(hz, strippath(fileName).c_str(), fileName.c_str());
				}

				CloseZip(hz);
			}

			if (_saveOutcomes)
			{
				fileName = m_strOutputRoot + L"\\outcomes-" + GetId() + L".csv";
				CMNotifier::Notify(CMNotifier::LOGTIME, L"Writing Outcomes to " + fileName);
				pApp->WriteOutcomes(fileName, this);
			}

			if (_saveSummary)
			{
				fileName = m_strOutputRoot + L"\\summary-" + GetId() + L".csv";
				CMNotifier::Notify(CMNotifier::LOGTIME, L"Writing Summary to " + fileName);
				pApp->WriteSummary(fileName, this);
			}
		}
	}
	state &= ~sRunning;
	return !Fail();
}

