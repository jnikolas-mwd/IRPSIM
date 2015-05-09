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
#include "simulat.h"
#include "irpapp.h"
#include "regions.h"
#include "variable.h"
#include "varcol.h"
#include "vartypes.h"
#include "irpapp.h"
#include "expresn.h"
#include "reliab.h"
#include "node.h"
#include "vardesc.h"
#include "notify.h"

#include "token.h"
#include "random.h"
#include "cmlib.h"
#include <time.h>
#include <fstream>
using namespace std;
//static ofstream sdebug("f:\\simulat.deb");

#define CM_BIGTIME 10000000L

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

const wchar_t* CMSimulation::file_header = L"#BEGINFILE";
const wchar_t* CMSimulation::file_footer = L"#ENDFILE";
CMSimulation* CMSimulation::active_simulation = 0;

CMSimulation::CMSimulation(CMIrpApplication* a) :
pApp(a),
script(0),
options(a->Options()),
comments(),
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
    timemachine = new CMTimeMachine(CMTime::StringToTimeUnit(options.GetOption(L"siminterval")),1);
	get_data_from_options();
	ntrials = options.GetOptionLong(L"numtrials");
}

/*
CMSimulation::CMSimulation(const CMString& fname,short mode,CMIrpApplication* a) :
filename(fname),
pApp(a),
script(0),
options(),
comments(),
costvars(),
outcomevars(),
summaryvars(),
missingvars(),
timemachine(0),
simarray(0),
accumulator(0),
reliability(0),
trialno(0),
state(sInitialized|sFromFile|sPaused|mode),
elapsedtime(0),
begintime()
//hRunEvent(NULL),
//dwThreadId(0L)

{
	LoadSimulationFile(fname,mode);
}
*/

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

/*
BOOL CMSimulation::LoadSimulationFile(const CMString& fname,short mode)
{
	if (Running())
		return FALSE;

	set_variables_inuse(FALSE);
	if (timemachine) delete timemachine;
	timemachine = 0;
	if (simarray) 	  delete simarray;
	simarray = 0;
	if (accumulator) delete accumulator;
	accumulator = 0;
	if (reliability) delete reliability;
	reliability = 0;
   //if (tempfile.length()) removefile(tempfile);
   if (active_simulation==this)
   	active_simulation=0;
	
	filename = fname;
	script = NULL;
	comments = L"";
	costvars.Reset(1);
	outcomevars.Reset(1);
	summaryvars.Reset(1);
	missingvars.Reset(1);
	trialno=0;
	state=sInitialized|sFromFile|sPaused|mode;
	elapsedtime=0;
	//tempfile = createtempfile(L"irp",0);
	simarray = new CMSimulationArray(fname.c_str());
	simarray->DeleteFileOnClose(0);
	if (simarray->Fail()) {
		state |= sBadBinaryFile;
		return FALSE;
	}
	else {
		short flag;
		wfstream* file = simarray->File();
		file->seekg(simarray->BinarySize(),ios::beg);
		file->read((wchar_t*)&flag, sizeof(flag));
		if (flag) {
			accumulator = new CMAccumulatorArray();
			accumulator->ReadBinary(*file);
		}
		file->read((wchar_t*)&flag, sizeof(flag));
		if (flag) {
			reliability = new CMReliability();
			reliability->ReadBinary(*file);
		}
		begintime.Read(*file,1);
		options.ReadBinary(*file);
		unsigned short nfiles;
		file->read((wchar_t*)&nfiles, sizeof(nfiles));
		for (unsigned i=0;i<nfiles;i++)
			loadedfiles.Add(readstringbinary(*file));
		readstringbinary(comments,*file);
		//wofstream os(tempfile.c_str(),IOS_BINARY);
  	   //if (!os.fail()) *file >> os.rdbuf();
		get_data_from_options();
		timemachine = new CMTimeMachine(simarray->TimeMachine());
		ntrials = accumulator->Trials();
		trialno = ntrials ? ntrials-1 : 0;
	}
	return TRUE;
}
*/

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
		if (v->GetState(CMVariable::vsSelected)==TRUE) {
			summaryvars.Add(v);
         if (v->IsType(L"cost"))
            costvars.Add(v);
			if (v->GetState(CMVariable::vsSaveOutcomes)==TRUE)
				outcomevars.Add(v);
		}
	}

	if (find_missing_variables())
		state |= sMissingVariables;

	summaryvars.Sort();
	outcomevars.Sort();

	long incsize = options.GetOptionLong(L"saveincrementsize")<<10;

	accumulator = new CMAccumulatorArray(*timemachine,summaryvars.Count());
	simarray = new CMSimulationArray(*timemachine,outcomevars.Count(),filename.c_str(),incsize);

	if (!simarray || simarray->Fail())
		state |= sCantOpenBinaryFile;
	for (i=0;i<summaryvars.Count() && accumulator;i++)
		accumulator->AssignVariable(i,summaryvars[i]->GetName(),summaryvars[i]->GetSpecialType(),get_vardesc_state(summaryvars[i]));
	for (i=0;i<outcomevars.Count() && simarray;i++)
		simarray->AssignVariable(i,outcomevars[i]->GetName(),outcomevars[i]->GetSpecialType(),get_vardesc_state(outcomevars[i]));
	simarray->DeleteFileOnClose(1);
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
		simarray->DeleteFileOnClose(1);
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
	CMVNameIterator* vi;
	while ((v=iter())!=0) {
		if (v->GetState(CMVariable::vsSelected)) {
			vi = v->CreateIterator();
			const wchar_t* vname;
			while (vi && ((vname=vi->GetNext())!=0)) {
				if (!CMVariable::Find(vname))
					if (!missingvars.Contains(vname))
						missingvars.Add(vname);
			}
			CMString atype,aname;
			for (int i=0;v->GetAssociation(i,atype,aname);i++) {
				CMVariable* vassoc = CMVariable::Find(aname);
				if (vassoc) {
					vi = vassoc->CreateIterator();
					while (vi && ((vname=vi->GetNext())!=0)) {
						if (!CMVariable::Find(vname))
						if (!missingvars.Contains(vname))
							missingvars.Add(vname);
					}
				}
			}
		}
	}
	return missingvars.Count();
}

/*
void CMSimulation::save_simulation_tail(wostream& s,int startpoint)
{
   if (!s.fail()) {
		if (startpoint==0) {
			short flag = accumulator ? 1 : 0;
			s.write((const wchar_t*)&flag, sizeof(flag));
			if (flag) accumulator->WriteBinary(s);
			flag = reliability ? 1 : 0;
			s.write((const wchar_t*)&flag, sizeof(flag));
			if (flag) reliability->WriteBinary(s);
			begintime.Write(s,1);
      }
		options.WriteBinary(s);
		unsigned short nfiles = loadedfiles.Count();
		s.write((const wchar_t*)&nfiles, sizeof(nfiles));
		for (unsigned i=0;i<loadedfiles.Count();i++)
			writestringbinary(loadedfiles[i],s);
		writestringbinary(comments,s);
		//wifstream is(tempfile.c_str(),IOS_BINARY);
       //if (!is.fail()) is >> s.rdbuf();
	}
}
*/

/*
int CMSimulation::SaveTo(const CMString& fname)
{
	wfstream s(fname.c_str(),ios::out|IOS_BINARY);
	if (!s.fail()) {
		if (pApp) pApp->LogMessage(CMString(L"Saving Simulation ") + GetName() + CMString(L" to ") + fname, 1);
		simarray->WriteBinary(s);
      save_simulation_tail(s,0);
	}
   return (s.fail() ? -1 : 0);
}

void CMSimulation::Save()
{
	if (!simarray || Fail())
   	return;
	if (state&sFromFile) {
		wfstream* s = simarray->File();
		s->seekp(simarray->BinarySize()+2*sizeof(short)+begintime.BinarySize(),ios::beg);
		if (accumulator) s->seekp(accumulator->BinarySize(),ios::cur);
		if (reliability) s->seekp(reliability->BinarySize(),ios::cur);
      save_simulation_tail(*s,1);
	}
	else if (!(state&sRunning) && timemachine->AtBeginning()) {
		simarray->DeleteFileOnClose(0);
		CMString msg = CMString(L"Saving Simulation ") + GetName();
		if (pApp) pApp->LogMessage(msg, 1);
		if (pApp) pApp->InfoMessage(msg);
		simarray->WriteBinary();
      save_simulation_tail(*simarray->File(),0);
	  if (pApp) pApp->InfoMessage(L"");
	}
}
*/

void CMSimulation::SetOptions(const CMOptions& op)
{
	for (unsigned i=0;i<op.Count();i++)
		SetOption(op.At(i)->GetName(),op.At(i)->GetValue());
}

void CMSimulation::SetOption(const CMString& opname,const CMString& opval)
{
	// return if simulation has been initialized and option is from
	// set of options that can't be changed after initialization
	if (contains(opname.c_str(),forbidden_set_options,500,0)>=0 && (state&sInitialized))
		return;
	options.SetOption(opname,opval);
	get_data_from_options();
}

void CMSimulation::get_data_from_options()
{
	filename = options.GetOption(L"simulationfile");
	simname  = options.GetOption(L"simulationname");
   if (!simname.length()) simname = L"<no name>";
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

/*
int CMSimulation::RebuildFiles(const CMString& path)
{
	int oldcase = CMString::set_case_sensitive(0);
	CMString dir(path);
   if (dir[dir.length()-1] != L'\\') dir += L"\\";
   wifstream is(tempfile.c_str(),IOS_BINARY);

   wofstream os;
	CMString line,token;
   int reading = 0;

   while (!is.fail() && !is.eof()) {
		line.read_line(is);
		if (is.eof())
      	break;
		line = stripends(line);
      CMTokenizer next(line);
		token = next(L" \t\r\n");
      if (token == file_footer) {
      	reading = 0;
         os.close();
      }
      else if (reading) {
      	if (!os.fail()) os << line << ENDL;
      }
      else if (token == file_header) {
      	token = dir + strippath(next(L" \t\r\n"));
		if (pApp) pApp->InfoMessage(CMString(L"Rebuilding file ") + token);
         os.open(token.c_str(),ios::out|IOS_BINARY);
         reading = 1;
      }
	}

   if (pApp) pApp->InfoMessage(L"");
	CMString::set_case_sensitive(oldcase);
	return 1;
}
*/

/*
DWORD WINAPI CMSimulation::SimRunProc(LPVOID lpParameter)
{
   unsigned i;
	CMSimulation* sim = (CMSimulation*)lpParameter;

	while (TRUE) 
	{
		WaitForSingleObject(sim->hRunEvent,INFINITE);

		CMIrpApplication* pApp = sim->pApp;

		CMScript* script = sim->script;
		CMTimeMachine* timemachine = sim->timemachine;
		CMSimulationArray* simarray = sim->simarray;
		CMAccumulatorArray* accumulator = sim->accumulator;
		CMReliability* reliability = sim->reliability;

	   time_t inittime = time(NULL);
		long initelapsed = sim->elapsedtime;

		while ((sim->trialno=timemachine->Count())<sim->ntrials && !sim->Fail() && !(sim->state&(sStopped|sFromFile))) {
			int atbeginning = timemachine->AtBeginning();

			if (atbeginning && (sim->state&sPaused))
				break;

			sim->state |= sRunning;

			if (atbeginning) {
				CMVariable::ResetTrial();
				if (sim->trialno == 0 && pApp)
					pApp->LogMessage(CMString(L"Start Simulation ") + sim->GetName(), 1);
			}

			if (script) script->Run(timemachine);

			for (i=0;i<sim->outcomevars.Count();i++)
				simarray->AddAt(*timemachine,i,sim->trialno,(float)sim->outcomevars[i]->GetValue(timemachine));
			for (i=0;i<sim->summaryvars.Count();i++)
				accumulator->AddAt(*timemachine,i,sim->summaryvars[i]->GetValue(timemachine));

			if (reliability) reliability->Process(timemachine);

			sim->elapsedtime = initelapsed + (long)time(NULL) - (long)inittime;
			if (pApp) pApp->Synchronize(CMIrpApplication::SYNC_SIMULATION_UPDATE, sim);

			timemachine->Step();
		}
		sim->elapsedtime = initelapsed + (long)time(NULL) - (long)inittime;
		if (sim->trialno >= sim->ntrials) {
			sim->state |= sStopped;
			sim->trialno--;
			if (pApp) {
				CMString msg(L"End Simulation ");
				pApp->LogMessage(msg + sim->GetName(), 1);
				if (sim->options.GetOption(L"autooutcomes")==L"yes")
					pApp->WriteOutcomes(sim->options.GetOption(L"outcomefile"), sim);
				if (sim->options.GetOption(L"autosummary")==L"yes")
					pApp->WriteSummary(sim->options.GetOption(L"summaryfile"), sim);
			}
		}
		sim->state &= ~sRunning;
	}

	return 0L;
}
*/

/*
BOOL CMSimulation::Run()
{
	if (state&sFromFile || (app && app->RunningSimulation()!=NULL))
		return FALSE;

	if (!hRunEvent) {
		hRunEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		CreateThread(NULL,0L,SimRunProc,this,0L,&dwThreadId);
	}

	if (!(state&sInitialized))
		initialize();

	if ((ULONG)begintime == CM_BIGTIME) {
		RandomVariable::randomize(randomseed);
		timemachine->Reset();
		begintime = CMTime();
		elapsedtime = 0;
	}

   active_simulation = this;
	state &= ~sPaused;

	SetEvent(hRunEvent);

	return TRUE;
}
*/

// Single-threaded Run Method
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
		}

		if (script) script->Run(timemachine);

		for (int i = 0; i<outcomevars.Count(); i++)
			simarray->AddAt(*timemachine, i, trialno, (float)outcomevars[i]->GetValue(timemachine));
		for (int i = 0; i<summaryvars.Count(); i++)
			accumulator->AddAt(*timemachine, i, summaryvars[i]->GetValue(timemachine));

		if (reliability) reliability->Process(timemachine);

		elapsedtime = initelapsed + (long)time(NULL) - (long)inittime;
		if (pApp) pApp->Synchronize(CMIrpApplication::SYNC_SIMULATION_UPDATE, this);

		timemachine->Step();
	}
	
	elapsedtime = initelapsed + (long)time(NULL) - (long)inittime;
	if (trialno >= ntrials) {
		state |= sStopped;
		trialno--;
		if (pApp) {
			CMNotifier::Notify(CMNotifier::LOGTIME, L"End Simulation " + GetName());
			if (options.GetOption(L"autooutcomes") == L"yes")
				pApp->WriteOutcomes(options.GetOption(L"outcomefile"), this);
			if (options.GetOption(L"autosummary") == L"yes")
				pApp->WriteSummary(options.GetOption(L"summaryfile"), this);
		}
	}
	state &= ~sRunning;
	return TRUE;
}

/*

Old, single-threaded version of Run()  
	 
void CMSimulation::Run()
{
   unsigned i;
	if (state&sFromFile)
		return;

	if (!(state&sInitialized))
		initialize();
	
	if ((ULONG)begintime == CM_BIGTIME) {
		RandomVariable::randomize(randomseed);
		timemachine->Reset();
		begintime = CMTime();
		elapsedtime = 0;
	}

	if (pApp && (pApp->RunningSimulation()>=0))
		return;

   active_simulation = this;

   time_t inittime = time(NULL);
   long initelapsed = elapsedtime;

	while ((trialno=timemachine->Count())<ntrials && !Fail() && !(state&(sStopped|sFromFile))) {
		int atbeginning = timemachine->AtBeginning();

		if (atbeginning && (state&sPaused))
			break;

		state |= sRunning;

		if (atbeginning) {
			CMVariable::ResetTrial();
			if (trialno==0 && pApp)
				pApp->LogMessage(CMString("Start Simulation ") + GetName(),1);
		}

		if (script) script->Run(timemachine);

		for (i=0;i<outcomevars.Count();i++)
			simarray->AddAt(*timemachine,i,trialno,(float)outcomevars[i]->GetValue(timemachine));
		for (i=0;i<summaryvars.Count();i++)
			accumulator->AddAt(*timemachine,i,summaryvars[i]->GetValue(timemachine));

		if (reliability) reliability->Process(timemachine);

		elapsedtime = initelapsed + time(NULL) - inittime;
		if (pApp) pApp->ProcessWhileRunning(this);
		timemachine->Step();
	}
	elapsedtime = initelapsed + time(NULL) - inittime;
	if (trialno >= ntrials) {
		state |= sStopped;
		trialno--;
		if (pApp) {
			CMString msg("End Simulation ");
			pApp->LogMessage(msg+GetName(),1);
			if (options.GetOption("autooutcomes")=="yes")
				pApp->WriteOutcomes(options.GetOption("outcomefile"),this);
			if (options.GetOption("autosummary")=="yes")
				pApp->WriteSummary(options.GetOption("summaryfile"),this);
		}
	}
	state &= ~sRunning;
}

*/
