// irpapp.cpp : implementation file
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
#include "irpapp.h"
#include "accum.h"
#include "vtime.h"
#include "vartypes.h"
#include "varcol.h"
#include "interval.h"
#include "regions.h"
#include "defines.h"
#include "reliab.h"
#include "simulat.h"
#include "scenario.h"
#include "script.h"
#include "simarray.h"
#include "savesima.h"
#include "notify.h"

#include "cmlib.h"
#include "token.h"

#include <stdio.h>
#include <iomanip>
#include <ctype.h>

//#include <fstream>
//static wofstream sdebug(L"debug_irpapp.txt");

//template class _IRPCLASS CMPSSmallArray<CMVariable>;

CMIrpApplication::CMIrpApplication() :
options(),
scenarios(),
simulations(),
outputvars(),
loadedfiles(),
//attachedfiles(),
variables(0),
oldvariables(0),
currentscript(0),
currentscenario(0),
errorcode(0),
vartypes()
{
	casesensitiveflag = CMString::set_case_sensitive(0);
	skipwhitespaceflag = CMString::skip_whitespace(1);
    variables = new CMVariableCollection(1);
	oldvariables =	CMVariable::SetCollectionContext(variables);
}

CMIrpApplication::~CMIrpApplication()
{
	ResetApplication();
    CMVariable::SetCollectionContext(oldvariables);
    delete variables;
	CMString::set_case_sensitive(casesensitiveflag);
	CMString::skip_whitespace(skipwhitespaceflag);
}

unsigned CMIrpApplication::SimulationsCount()
{
	return simulations.Count();
}

void CMIrpApplication::OpenProject(const CMString& name)
{
	ResetApplication();
	CMVariable::SetCollectionContext(variables);
//	project_file = getfullpathname(name.c_str());
	CMString strName(name);
	int varsread = 0;
	m_strProjectFile = name;
	CMNotifier::Notify(CMNotifier::LOG, L"Opening Project: " + m_strProjectFile);
	read_file(name, varsread);
	if (varsread)
		update_variable_links();
	//AddFiles(&strName,1);
}

/*
int CMIrpApplication::IsApplicationFile(const CMString& filename)
{
	unsigned i;

	for (i=0;i<loadedfiles.Count();i++)
		if (loadedfiles[i] == filename)
			return FILE_DEFINITION;

	for (i=0;i<simulations.Count();i++)
		if (simulations[i]->GetFileName() == filename)
			return FILE_SIMULATION;
	
	for (i=0;i<attachedfiles.Count();i++)
		if (attachedfiles[i] == filename)
			return FILE_ATTACHED;
	
	return 0;
}
*/

struct _thread_data {
	CMIrpApplication* pApp;
	CMString* pFileNames;
	int nFiles;
	//DWORD dwThreadId;
};

//void CMIrpApplication::AddFiles(CMString *pNames,int nFiles)
//{
	//int varsread = 0;
	//read_file(pNames[0], varsread);
	
	/***TODO: Create threaded version
	static _thread_data td;
	if (!pNames || nFiles<=0)
		return;
	td.pApp = this;
	td.nFiles = nFiles;
	td.pFileNames = new CMString[nFiles];
	for (int i=0;i<nFiles;i++)
		td.pFileNames[i] = pNames[i];
	CreateThread(NULL,0L,add_file_proc,&td,0L,&td.dwThreadId);
	*/
//}

/*
void CMIrpApplication::DeleteFiles(CMString *pNames,int nFiles)
{
	CMString strProjFile(GetProjectFile());
	if (strProjFile.is_null())
		return;
	int nFilesDeleted = 0;

	for (int i=0;i<nFiles;i++) {
		switch (IsApplicationFile(pNames[i])) {
			case FILE_DEFINITION: loadedfiles.Detach((const CMString&)pNames[i]); nFilesDeleted++; break;
			case FILE_ATTACHED:   attachedfiles.Detach((const CMString&)pNames[i]); nFilesDeleted++; break;
		}
	}
	if (nFilesDeleted) {
		ResetApplication();
		AddFiles(&strProjFile,1);
	}
}
*/

/*
DWORD WINAPI CMIrpApplication::add_file_proc(LPVOID lpParameter)
{
	_thread_data* pTD = (_thread_data*)lpParameter;
	int varsread = 0;
   int err = 0;

	try 
	{
		for (int i=0;i<pTD->nFiles && !err;i++) {
			//if (!pTD->pApp->IsApplicationFile(pTD->pFileNames[i]))
				err = pTD->pApp->read_file(pTD->pFileNames[i],varsread);
		}

		if (!err && varsread)
			pTD->pApp->update_variable_links();
	}	
	catch (CMException& except) 
	{
		//::MessageBeep(MB_OK);
		pTD->pApp->ErrorMessage(except.What());
   }

	delete [] pTD->pFileNames;
	pTD->pApp->Synchronize(SYNC_FILE_ADDED,NULL);
	return (DWORD)(err ? -(pTD->pApp->errorcode=EReadingFile) : varsread);
}
*/

int CMIrpApplication::read_file(const CMString& name,int &varsread)
{
	static const wchar_t* delims = L"# \t\r\n";
	CMString messageheader = L"Reading ";
	CMString errorheader = L"problem reading file ";
	wifstream* stack[128];
	int current=0,currfile,oldfile;
	CMString currpath = name;

	stack[0] = new wifstream(name.c_str(),ios::in|IOS_BINARY);
	wifstream* s = stack[0];

	if (s->fail()) {
		CMNotifier::Notify(CMNotifier::ERROR, errorheader+name);
		return -1;
	}

	currfile = add_file_to_list(name);

	CMNotifier::Notify(CMNotifier::LOG, messageheader + name);

    CMTimeMachine tm(CM_MONTH,1);
	int err = 0;
	while (s && !err) {
		CMString line;
		long index = (long)s->tellg();
		line.read_line(*s);
		line = stripends(line);
		if (!line.is_null() && line[0]==L'#') {
			CMTokenizer next(line);
			CMString token = next(delims);
			if (token == L"include") {
				CMString fname = next(delims);
				currpath = getabsolutepath(name.c_str(), fname.c_str());

				s = stack[++current] = new wifstream(currpath.c_str(),ios::in|IOS_BINARY);
				if (s->fail()) {
					CMNotifier::Notify(CMNotifier::ERROR, errorheader + fname);
					err = -1;
				}
				else {
					oldfile=currfile;
					currfile = add_file_to_list(fname);
					CMNotifier::Notify(CMNotifier::LOG, messageheader + fname);
				}
			}
			/*
			else if (token == L"attach") {
				CMString fname = next(delims);
				add_file_to_list(fname,FILE_ATTACHED);
				CMNotifier::Notify(CMNotifier::INFO, messageheader + fname, true);
			}
			*/
			else if (token == L"define") {
				CMString s1 = next(delims);
				CMString s2 = next(delims);
				CMDefinitions::Add(s1,s2,currfile);
			}
			else if (token==L"options") {
				CMNotifier::Notify(CMNotifier::INFO, messageheader + L"options");
				options.SetApplicationId(currfile);
				s->seekg(index);
				*s >> options;
			}
			else if (token==L"intervals") {
				CMNotifier::Notify(CMNotifier::INFO, messageheader + L"intervals");
				CMInterval::Read(*s);
				CMInterval::SetApplicationIdAll(currfile);
			}
			else if (token==L"scenario") {
				s->seekg(index);
				CMScenario* sce = new CMScenario(currfile);
				*s >> *sce;
				scenarios.Add(sce);
				CMNotifier::Notify(CMNotifier::INFO, messageheader + sce->GetName());
			}
			else if (token==L"script") {
				s->seekg(index);
				CMScript* scr = new CMScript(currfile);
				*s >> *scr;
				scripts.Add(scr);
				CMNotifier::Notify(CMNotifier::INFO, messageheader + scr->GetName());
			}
			else if (token==L"category") {
				s->seekg(index);
				CMCategory* cat = new CMCategory(currfile);
				*s >> *cat;
				CMCategory::AddCategory(cat);
				CMNotifier::Notify(CMNotifier::INFO, messageheader + cat->GetName());
			}
			else if (token==L"vardef") {
				if (!varsread)
					variables->SetStateAll(CMVariable::vsLinksUpdated,FALSE);
				varsread++;
				s->seekg(index);
				CMVariable* v;
				*s >> v;
				if (v) {
					if (variables->Find(v->GetName()))
						delete v;
					else {
						variables->Add(v);
						v->SetApplicationId(currfile);
						CMNotifier::Notify(CMNotifier::INFO, messageheader + v->GetName());
					}
				}
			}
			else {
				CMNotifier::Notify(CMNotifier::WARNING, L"Unrecognized token: #" + token + L" in file " + currpath);
			}
		}
		if (s->eof()) {
			delete stack[current--];
			s = current>=0 ? stack[current] : 0;
			currfile=oldfile;
		}
	}

	if (current >= 0)
		for (int i=0;i<=current;i++)
			delete stack[i];

	return err;
}

void CMIrpApplication::update_variable_links()
{
	CMVariable* v;
	wchar_t buffer[128];
   CMString region,vtype,vname;
	const wchar_t* aggname;

	CMNotifier::Notify(CMNotifier::LOG, L"Updating variable types");

	// Destroy aggregate regional variables
	
	variables->DestroyIfState(CMVariable::vsRegional,0);
	
	// update regions
	CMRegions::Reset();
	CMVariableIterator iter;
	while ((v=iter())!=0)
   	v->SetRegion(CMRegions::GetRegionId(v->GetAssociation(L"region")));
	iter.Reset();
	while ((v=iter())!=0) {
		if (v->GetRegion()>=0) {
			region = CMRegions::GetRegionName(v->GetRegion());
   		for (int i=0;v->GetAssociation(i,vtype,vname);i++) {
				if (CMVariableTypes::VarIntFromString(vtype.c_str(),1)>=0) {
					CMVariable* vfound = CMVariable::Find(vname);
         		if (vfound) {
           	   	vfound->AddAssociation(L"region",region);
			     	   vfound->SetRegion(v->GetRegion());
					}
				}
     	   }
      }
	}

	if (CMRegions::RegionCount()>0) {
		swprintf_s(buffer,128,L"\r\n%d Regions Defined:\r\n",CMRegions::RegionCount());
		CMNotifier::Notify(CMNotifier::LOG, buffer);

		for (unsigned short i=0;i<CMRegions::RegionCount();i++)
			CMNotifier::Notify(CMNotifier::LOG, CMRegions::GetRegionName(i));
		CMNotifier::Notify(CMNotifier::LOG, L"");
	}

	// recreate aggregate regional variables

	for (int i=0;(aggname=CMVariableTypes::AggStringFromInt(i))!=0;i++) {
		for (unsigned short j=0;j<CMRegions::RegionCount();j++) {
			CMVariable* v  = new CMVariable((CMString)aggname + L"." + CMRegions::GetRegionName(j),CMVariable::vsAggregate | CMVariable::vsRegional);
			v->SetType(i-1000);
			if (CMVariableTypes::IsAggSum(i)) v->SetState(CMVariable::vsSum,TRUE);
			variables->Add(v);
		}
	}

	// update variable associations for nodes
	
	variables->UpdateVariableTypes();
		
	// find missing variables
	
	iter.Reset();
	CMNotifier::Notify(CMNotifier::LOG, L"\r\nLooking for missing variables:");
	iter.Reset();
	long vcount = variables->Count();
	for (long n=0;;n++) {
		if ((v=iter())==0)
			break;
		if (v->GetState(CMVariable::vsAggregate | CMVariable::vsSystem))
			continue;
		int pct = (int)(100*n/vcount);
		CMIrpObjectIterator* ni = v->CreateIterator();
		const wchar_t* vname;
		int first=1;
		while (ni && ((vname=ni->GetNext())!=0)) {
			if (!CMVariable::Find(vname) && !CMDefinitions::IsDefined(vname)) {
				if (first)
					CMNotifier::Notify(CMNotifier::LOG, CMString(L"\r\n") + v->GetName() + L"\r\n missing:");
				first=0;
				CMNotifier::Notify(CMNotifier::LOG, vname);
				CMNotifier::Notify(CMNotifier::ERROR, L"Missing variable or definition " + CMString(vname) + L" in definition of " + v->GetName());
			}
		}
	}
	CMNotifier::Notify(CMNotifier::LOG, L"\r\nFinished\r\n");
	CMNotifier::Notify(CMNotifier::INFO, L"");
}

/*
CMString CMIrpApplication::GetProjectName()
{
	size_t index;
	CMString ret;
	if (loadedfiles.Count())
		ret = strippath(loadedfiles[0]);
	if ((index=ret.find(L'.')) != CM_NPOS) ret.remove(index);

	return ret;
}

CMString CMIrpApplication::GetProjectFile() 
{
	CMString ret;
	if (loadedfiles.Count())
		ret = strippath(loadedfiles[0]);
	return ret;
}
*/

CMString CMIrpApplication::GetObjectFileName(CMIrpObject* pObject)
{
	CMString ret;
	if (pObject) ret = LoadedFile(pObject->GetApplicationId());
	return ret;
}

/*
void CMIrpApplication::SetOptions(const CMOptions& op)
{
	for (unsigned i=0;i<op.Count();i++)
		options.SetOption(op.At(i)->GetName(),op.At(i)->GetValue());
}
*/

/*
CMScenario* CMIrpApplication::AddThisScenario(const CMString& name)
{
	CMScenario* sce = NULL;
	try {

	CMVariable::SetCollectionContext(variables);
	sce = new CMScenario(name);
	for (unsigned i=0;i<options.Count();i++)
		sce->AddEntry(options.At(i)->GetName(),options.At(i)->GetValue(),1);
	CMVariableIterator next;
	CMVariable* v;
	while ((v=next())!=0) {
		if (v->GetState(CMVariable::vsSelected)) {
			CMString value;
			if (v->GetState(CMVariable::vsSaveOutcomes)) value += L"save";
			if (v->GetState(CMVariable::vsOutput)) value += L"write";
			sce->AddEntry(v->GetName(),value);
		}
	}
	scenarios.Add(sce);

   }
   catch (CMException& except) {
   	ErrorMessage(except.What());
   }
	return sce;
}
*/

CMScenario* CMIrpApplication::UseScenario(const CMString& name)
{
   try {

	CMVariable::SetCollectionContext(variables);
	unsigned short n;
    for (n=0;n<scenarios.Count();n++)
   	if (name == scenarios[n]->GetName())
      	break;
	if (n<scenarios.Count()) {
		currentscenario = scenarios[n];
		ResetOutputVariables();
		scenarios[n]->Use(options);
		options.SetOption(L"simulationname",scenarios[n]->GetName());
		for (unsigned i=0;i<scenarios[n]->Variables();i++) {
      	CMString varname = scenarios[n]->VariableName(i);
			outputvars.Add(varname);
         for (unsigned j=0;j<simulations.Count();j++) {
				CMSimulationArray* array = simulations[j]->SimArray();
				CMAccumulatorArray* accum = simulations[j]->Accumulator();
				if (array)
					array->SetVariableState(array->VariableIndex(varname),CMVariableDescriptor::vdOutput,TRUE);
				if (accum)
					accum->SetVariableState(accum->VariableIndex(varname),CMVariableDescriptor::vdOutput,TRUE);
			}
      }
	}

	}
   catch (CMException& except) {
	   CMNotifier::Notify(CMNotifier::ERROR, except.What());
   }
	return currentscenario;
}

/*
void CMIrpApplication::RemoveScenario(unsigned short n)
{
	scenarios.DetachAt(n,1);
}
*/

/*
void CMIrpApplication::AddScenario(CMScenario* sce)
{
	scenarios.Add(sce);
}
*/

CMScript* CMIrpApplication::FindScript(const CMString& name)
{
	for (unsigned short i=0;i<scripts.Count();i++)
   	if (name == scripts[i]->GetName())
      	return scripts[i];
   return 0;
}

CMScenario* CMIrpApplication::FindScenario(const CMString& name)
{
	for (unsigned short i=0;i<scenarios.Count();i++)
   	if (name == scenarios[i]->GetName())
      	return scenarios[i];
   return 0;
}

CMCategory* CMIrpApplication::FindCategory(const CMString& name)
{
	return CMCategory::FindCategory(name);	
}

CMIrpObject* CMIrpApplication::FindIrpObject(const CMString& name)
{
	CMIrpObject* pi = CMVariable::Find(name);	if (pi) return pi;
	
	pi = FindCategory(name); if (pi) return pi;

	pi = FindScript(name);	if (pi) return pi;

	pi = FindScenario(name); if (pi) return pi;

	return NULL;
}


CMScript* CMIrpApplication::UseScript(const CMString& name)
{
	return (currentscript = FindScript(name));
}

/*
void CMIrpApplication::RemoveScript(unsigned short n)
{
	scripts.DetachAt(n,1);
}

void CMIrpApplication::AddScript(CMScript* scr)
{
	scripts.Add(scr);
}
*/

/*
int CMIrpApplication::DestroyVariable(const CMString& vname)
{
	return variables ? variables->DestroyVariable(vname,0) : 0;
}
*/

/*
CMVariable* CMIrpApplication::AddVariable(const CMString& vdef)
{
	return variables ? variables->AddVariable(vdef) : 0;
}
*/

void CMIrpApplication::SetVariableStateAll(ULONG aState,BOOL action)
{
	if (variables) variables->SetStateAll(aState,action);
}

void CMIrpApplication::UpdateVariableLinksAll()
{
	if (variables) variables->UpdateVariableLinks();
}

void CMIrpApplication::UpdateVariableLinkStatusAll()
{
	if (variables) variables->UpdateLinkStatus();
}

void CMIrpApplication::ResetApplication()
{
//	LogMessage("Resetting application...");

	/*
	if (loadedfiles.Count() || attachedfiles.Count()) {	
		wofstream of(loadedfiles[0].c_str());
		unsigned short i;
		for (i=1;!of.fail() && i<loadedfiles.Count();i++)
			of << L"#include " << getrelativepath(loadedfiles[0].c_str(),loadedfiles[i].c_str()) << ENDL;
		for (i=0;!of.fail() && i<attachedfiles.Count();i++)
			of << L"#attach " << getrelativepath(loadedfiles[0].c_str(),attachedfiles[i].c_str()) << ENDL;

		// add any scenarios that were added manually or were contained in the project file
		for (i=0;Scenario(i)!=NULL;i++)
			if (Scenario(i)->GetApplicationId()<=0)
				of << ENDL << *Scenario(i);
	}
	*/

	//for (unsigned short i=0;i<simulations.Count();i++)
		//if (simulations[i] != 0) simulations[i]->Save();
	simulations.ResetAndDestroy(1);
	if (variables)
	   variables->ResetCollection();
	CMDefinitions::Reset();
	CMInterval::Reset();
	CMCategory::DestroyCategories();
	CMNode::DestroyNodes();
	scenarios.ResetAndDestroy(1);
	scripts.ResetAndDestroy(1);
	outputvars.Reset(1);
	loadedfiles.Reset(1);
	//attachedfiles.Reset(1);
	currentscript = NULL;
	currentscenario = NULL;
	m_strProjectFile = L"";
//	project_file = "";
//	LogMessage("done");
}

CMSimulation* CMIrpApplication::RunningSimulation()
{
	for (unsigned i=0;i<SimulationsCount();i++)
		if (simulations[i]->Running())
			return simulations[i];
	return NULL;
}


CMSimulation* CMIrpApplication::CreateSimulation()  //const wchar_t* pFileName)
{
	CMSimulation* newsim = NULL;

	try {
		CMVariable::SetCollectionContext(variables);
		//if (!pFileName) {
			if (!RunningSimulation()) {
				newsim = new CMSimulation(this);
				if (newsim->Fail()) {
  	   				delete newsim;
					CMNotifier::Notify(CMNotifier::ERROR, L"Unable to create simulation");
					newsim=NULL;
				}
			}
		//}
		
		/*
		else {
			newsim = new CMSimulation(pFileName,0,this);
			if (newsim->Fail()) {
	  			delete newsim;
				LogMessage(CMString(pFileName) + L" not a valid simulation file");
				newsim=NULL;
			}
		}
		*/

		if (newsim)
			simulations.Add(newsim);
   }
   
	catch (CMException& except) {
		CMNotifier::Notify(CMNotifier::ERROR, except.What());
   }

	return (newsim);
}

void CMIrpApplication::DeleteSimulation(CMSimulation* pSim,int save)
{
	try {
		for (unsigned i=0;i<simulations.Count();i++) {
			if (pSim==simulations[i]) {
				CMString fname = pSim->GetFileName();
				//if (save) pSim->Save();
				simulations.DetachAt(i,1);
				/*
				if (save) {
					if (add_file_to_list(fname,FILE_ATTACHED)>=0)
						Synchronize(SYNC_FILE_ATTACHED,NULL);
				}
				*/
				break;
			}
		}
   }
   catch (CMException& except) {
	   CMNotifier::Notify(CMNotifier::ERROR, except.What());
   }
}

BOOL CMIrpApplication::RunSimulation(CMSimulation* pSim)
{
	if (pSim==NULL || RunningSimulation())
		return FALSE;
	if (currentscript == NULL) {
		CMNotifier::Notify(CMNotifier::ERROR, L"Cannot run simulation: No script selected");
		return FALSE;
	}
	if (!pSim->Initialized()) {
		for (unsigned short i=0;i<scripts.Count();i++)
			scripts[i]->Parse(*this);
		pSim->SetScript(currentscript->GetName());
	}
	pSim->Run();
	return TRUE;
}


BOOL CMIrpApplication::PauseSimulation(CMSimulation* pSim,BOOL bAction)
{
	if (pSim) pSim->Pause(bAction);
	return TRUE;
}

/*
int CMIrpApplication::WriteOptions(const CMString& filename)
{
	wofstream of(filename.c_str(),IOS_BINARY);
	if (of.fail())	return -1;
	of << options;
	return 0;
}

int CMIrpApplication::WriteScenarios(const CMString& filename)
{
	wofstream of(filename.c_str(),IOS_BINARY);
	if (of.fail())	return -1;
	for (unsigned i=0;i<scenarios.Count();i++)
		of << *scenarios[i] << ENDL;
	return 0;
}

int CMIrpApplication::WriteScripts(const CMString& filename)
{
	unsigned short i;

	for (i=0;i<scripts.Count();i++)
	  	scripts[i]->Parse(*this);
	wofstream of(filename.c_str(),IOS_BINARY);
	if (of.fail())	return -1;
	for (i=0;i<scripts.Count();i++)
		of << *scripts[i] << ENDL;
	return 0;
}

int CMIrpApplication::WriteVariableDefs(const CMString& filename)
{
	wofstream fs(filename.c_str(),IOS_BINARY);
	if (fs.fail())
		return -(errorcode=EOpeningFile);

	CMVariable::SetCollectionContext(variables);

	CMString messageheader(L"Writing ");
	CMPSSmallArray<CMVariable> vars;
	CMVariableIterator next;
	CMVariable* v;
	InfoMessage(L"Sorting variables...");
	while ((v=next())!=0)
		vars.Add(v);
	vars.Sort();
	for (unsigned i=0;i<vars.Count();i++) {
		InfoMessage(messageheader+vars[i]->GetName());
		fs << *vars[i] << ENDL;
	}
	InfoMessage(L"");
	return 0;
}
*/

int CMIrpApplication::WriteOutcomes(const CMString& filename,CMSimulation* sim)
{
	if (!sim) return -1;
	CMSaveSimulationAscii report(*sim,this);
	for (unsigned i=0;i<outputvars.Count();i++)
   	report.AddOutputVariable(outputvars[i]);
	CMVariableIterator iter;
   CMVariable* v;
   while ((v=iter())!=0)
   	if (v->GetState(CMVariable::vsSelected) && !outputvars.Contains(v->GetName()))
	   	report.AddOutputVariable(v->GetName());
   return report.Outcomes(filename);
}

int CMIrpApplication::WriteSummary(const CMString& filename,CMSimulation* sim)
{
	if (!sim) return -1;
	CMSaveSimulationAscii report(*sim,this);
	for (unsigned i=0;i<outputvars.Count();i++)
   	report.AddOutputVariable(outputvars[i]);
	CMVariableIterator iter;
   CMVariable* v;
   while ((v=iter())!=0)
   	if (v->GetState(CMVariable::vsSelected) && !outputvars.Contains(v->GetName()))
	   	report.AddOutputVariable(v->GetName());
   return report.Summary(filename);
}

int CMIrpApplication::add_file_to_list(const CMString& name)
{
	int nRet = -1;
	loadedfiles.Add(getabsolutepath(m_strProjectFile.c_str(), name.c_str()));
	nRet = loadedfiles.Count() - 1;
	return nRet;
}

/*
	CMString strpath = getfullpathname(name.c_str());

	if (!strpath.length())
		return -1;

	if (IsApplicationFile(strpath))
		return -1;

	switch (nType) {
		case FILE_DEFINITION:
			loadedfiles.Add(strpath);
			nRet=loadedfiles.Count()-1;
			break;
		case FILE_ATTACHED:
			attachedfiles.Add(strpath);
			nRet=attachedfiles.Count()-1;
			break;
	}

	return nRet;
}
*/