// TestConsole.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <fstream>

#include <irpapp.h>
#include <notify.h>

static wofstream sdebug("debug_testconsole.txt",ios::binary);

class CMTestNotifier : public CMNotifier {
protected:
	int notify(ntype type, const wchar_t* msg, int data) { sdebug << msg << ENDL; return 0; }
};

using namespace std;

class testclass {
	int a;
	int b;
public:
	testclass() : a(1), b(2) {}
};

int _tmain(int argc, _TCHAR* argv[])
{
	try {

		//CMNotifier::SetNotifier(new CMTestNotifier());

		sdebug << "Size of wchar_t is " << sizeof(wchar_t) << endl;

		_CrtMemState s1, s2, s3;
		_CrtMemCheckpoint(&s1);

		if (1) {
			string a("This is a test");
		}

		//CMIrpApplication* app = new CMIrpApplication();

		//app->OpenProject(L"C:\\Users\\Casey\\Documents\\IRPSIM Project Test\\Inputs\\Configuration\\IRP Resource Strategy.cfg");

		//app->UseScenario(L"test");
		//app->UseScript(L"basemix-cra-ondemand");

		//CMSimulation* sim = app->CreateSimulation();
		//app->RunSimulation(sim);

		//delete app;
		
		_CrtMemCheckpoint(&s2);

		if (_CrtMemDifference(&s3, &s1, &s2)) {
			_CrtMemDumpStatistics(&s3);
		}
	}
	catch (exception &ex) {
		cout << "Exception: " << ex.what();
	}

	_CrtDumpMemoryLeaks();

	return 0;
}

