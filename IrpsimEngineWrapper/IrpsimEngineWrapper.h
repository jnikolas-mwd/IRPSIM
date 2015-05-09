// IrpsimEngineWrapper.h

#pragma once

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <irpapp.h>
#include <notify.h>
#include <msclr\marshal_cppstd.h>

static wofstream sdebug("debug_wrapper.txt", ios::binary);

using namespace System;
using namespace msclr::interop;

namespace IrpsimEngineWrapper	
{

	private class _CMIrpApplication : public CMIrpApplication
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
		virtual BOOL Synchronize(int syncType, const void* lpParameter) { return FALSE; }
	};
	
	public class CMWrappedNotifier : public CMNotifier
	{
		virtual int notify(ntype type, const CMString& msg) 
		{
			sdebug << msg << ENDL; return 0;
		}
	};

	public ref class CMWrappedIrpApplication
	{
	private:
		_CMIrpApplication *app;
		CMSimulation *sim = nullptr;
	public:
		CMWrappedIrpApplication()
		{
			app = new _CMIrpApplication();
			CMNotifier::SetNotifier(new CMWrappedNotifier());
		}

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
