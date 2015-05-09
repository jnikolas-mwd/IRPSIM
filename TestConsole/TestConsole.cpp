// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>

#include <irpapp.h>
#include <notify.h>

static wofstream sdebug("debug_testconsole.txt",ios::binary);

class CMTestNotifier : public CMNotifier
{
	virtual int notify(ntype type, const CMString& msg) { 
		sdebug << msg << ENDL; return 0; 
	}
};


class CMTestApp : public CMIrpApplication
{
protected:
	virtual void log_message(const CMString& msg) { sdebug << L"log_message: " << msg << ENDL; }
	virtual CMString get_file_info(const wifstream& fname) { sdebug << L"get_file_info called" << ENDL; return L""; };

public:
	// use InfoMessage for "quick and dirty" messages
	/*
	virtual void InfoMessage(const CMString& message)
	{
		sdebug << L"InfoMessage: " << message << ENDL;
	}
	virtual void ErrorMessage(const CMString& msg) {
		sdebug << L"ErrorMessage: " << msg << ENDL;
	}
	*/
	// use ProgressMessage to add to previous messages. Call with message=0 to terminate

	// Synchronize is called by various threads for various reasons
	// lpParameter is set depending on syncType
	// return TRUE if user has cancelled operation
	virtual BOOL Synchronize(int syncType, const void* lpParameter) {return FALSE; }
};

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	try {

		CMNotifier::SetNotifier(new CMTestNotifier());

		CMTestApp *app = new CMTestApp();

		app->OpenProject(L"C:\\Users\\Casey\\Documents\\IRPSIM Project Test\\Inputs\\Configuration\\IRP Resource Strategy.cfg");
	

		sdebug << "Scenarios: " << app->ScenariosCount() << ENDL;

		sdebug << "Scripts: " << app->ScriptsCount() << ENDL;

		app->UseScenario(L"dsm_input");
		app->UseScript(L"basemix-cra-ondemand");
		app->Script(0)->Parse(*app);
		sdebug << *app->Script(0) << ENDL;

		for (int i = 0; i < app->LoadedFilesCount(); i++)
			sdebug << app->LoadedFile(i) << ENDL;

		CMSimulation* pSim = app->CreateSimulation();
		app->RunSimulation(pSim);	
		
		/*
		CMVariableIterator iter(app->VariableCollection());

		CMVariable* v;

		while (TRUE) {
			v = iter();
			if (!v)
				break;
			sdebug << *v << ENDL;
		}
		*/
		
		
		//CMExpression ex(L"min(total_20x2020, max((_cyear-2010-delay_consv),0)*total_20x2020/10)");

		//wcout << ex.GetString() << ENDL;

	}
	catch (exception &ex) {
		cout << "Exception: " << ex.what();
	}
	return 0;
}

