// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>

#include <irpapp.h>
#include <notify.h>
//#include <IrpsimEngineWrapper.h>

using namespace IrpsimEngineWrapper;

static wofstream sdebug("debug_testconsole.txt",ios::binary);


class CMTestNotifier : public CMNotifier {
protected:
	int notify(ntype type, const wchar_t* msg, int data) { sdebug << msg << ENDL; return 0; }
};


using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	try {

		CMNotifier::SetNotifier(new CMTestNotifier());

		CMWrappedIrpApplication^ app = gcnew CMWrappedIrpApplication();
		//CMIrpApplication *app = new CMIrpApplication();

		app->OpenProject(L"C:\\Users\\Casey\\Documents\\IRPSIM Project Test\\Inputs\\Configuration\\IRP Resource Strategy.cfg");
	

		//sdebug << "Scenarios: " << app->ScenariosCount() << ENDL;

		//sdebug << "Scripts: " << app->ScriptsCount() << ENDL;

		app->UseScenario(L"test");
		app->UseScript(L"basemix-cra-ondemand");

		app->RunSimulation();
		//app->Script(0)->Parse(*app);
		//debug << *app->Script(0) << ENDL;

		//for (int i = 0; i < app->LoadedFilesCount(); i++)
			//sdebug << app->LoadedFile(i) << ENDL;

		//CMSimulation* pSim = app->CreateSimulation();
		//app->RunSimulation(pSim);	
		
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

