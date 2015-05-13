// IrpsimEngineWrapper.h

#pragma once

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <irpapp.h>
#include <notify.h>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;
//using namespace System::Runtime::InteropServices;

namespace IrpsimEngineWrapper	
{
	/*
	private class _CMIrpApplication : public CMIrpApplication
	{

	};
	*/
	/*
	public delegate int CMNotifyDelegate(int, byte* , int);
	*/

	public ref class CMWrappedIrpApplication
	{
	private:
		CMIrpApplication *app = nullptr;
		CMSimulation *sim = nullptr;
		//CMNotifyDelegatePointer pDelegate;
	public:
		CMWrappedIrpApplication()
		{
			app = new CMIrpApplication();
			CMNotifier::SetDelegate(nullptr);
		}
			
		/*
		CMWrappedIrpApplication(CMNotifyDelegate^ del)
		{
			app = new _CMIrpApplication();
			IntPtr delegatePointer = Marshal::GetFunctionPointerForDelegate(del);
			pDelegate = reinterpret_cast<CMNotifyDelegatePointer>(delegatePointer.ToPointer());
			CMNotifier::SetDelegate(pDelegate);
		}
		*/

		void OpenProject(String^ fileName)
		{
			std::wstring str = marshal_as<std::wstring>(fileName);

			app->OpenProject(str.c_str());
		}

		void UseScenario(String^ name)
		{
			std::wstring str = marshal_as<std::wstring>(name);
			app->UseScenario(str.c_str());
		}

		void UseScript(String^ name)
		{
			std::wstring str = marshal_as<std::wstring>(name);
			app->UseScript(str.c_str());
		}

		void RunSimulation()
		{
			sim = app->CreateSimulation();
			app->RunSimulation(sim);
		}


		// TODO: Add your methods for this class here.
	};
}
