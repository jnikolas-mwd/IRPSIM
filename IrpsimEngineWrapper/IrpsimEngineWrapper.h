// IrpsimEngineWrapper.h

#pragma once

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <irpapp.h>
#include <notify.h>
#include <defines.h>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections;
using namespace System::ComponentModel;
using namespace msclr::interop;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::ObjectModel;

namespace IrpsimEngineWrapper	
{
	public delegate int CMNotifierDelegate(int type, String^ msg, int data);
	typedef int(__stdcall  *CMNotifierCallback)(CMNotifier::ntype type, String^  msg, int data);

	public enum class NodeType { None, Demand, Supply, Storage, Cost };

	private class _CMNotifier : public CMNotifier 
	{
	private:
		IntPtr ip;
		CMNotifierCallback callback;
	public:
		_CMNotifier()
		{
			CMNotifier::SetNotifier(this);
		}
		
		void SetNotifierDelegate(CMNotifierDelegate^ d)
		{
			ip = Marshal::GetFunctionPointerForDelegate(d);
			callback = static_cast<CMNotifierCallback>(ip.ToPointer());
		}
		
		virtual int notify(CMNotifier::ntype type, const wchar_t* msg, int data)
		{
			if (callback)
				return callback(type, marshal_as<String^>(msg), data);
			return 0;
		}
	};

	public ref class CMWrappedNotifier
	{
	private:
		_CMNotifier* pNotifier = nullptr;
	public:
		CMWrappedNotifier(CMNotifierDelegate^ d)
		{
			pNotifier = new _CMNotifier();
			pNotifier->SetNotifierDelegate(d);
		}
	};
	
	public ref class CMWrappedIrpObject : INotifyPropertyChanged
	{
	protected:
		CMIrpObject* obj;
	public:
		virtual event PropertyChangedEventHandler^ PropertyChanged;
		
		void OnPropertyChanged(String^ info)
		{
			PropertyChanged(this, gcnew PropertyChangedEventArgs(info));
		}

		CMWrappedIrpObject(CMIrpObject* o)
		{
			obj = o;
		}
		
		property int FileId
		{
			int get() { return obj->GetApplicationId(); }
		}

		property String^ Name
		{
			String^ get() { return marshal_as<String^>(obj->GetName().c_str()); }
		}

		property String^ Type
		{
			String^ get() { return marshal_as<String^>(obj->IsA()); }
		}

		virtual String^ ToString() override { return Name; }
	};
	
	public ref class CMWrappedVariable : public CMWrappedIrpObject {
	private:
		NodeType _ntype;
	public:
		CMWrappedVariable(CMVariable* v) : CMWrappedIrpObject(v)
		{
			if (v->IsType(L"Demand")) _ntype = NodeType::Demand;
			else if (v->IsType(L"Supply")) _ntype = NodeType::Supply;
			else if (v->IsType(L"Storage")) _ntype = NodeType::Storage;
			else if (v->IsType(L"Cost")) _ntype = NodeType::Cost;
		}

		property String^ EType
		{
			String^ get() 
			{ 
				CMVariable* v = (CMVariable*)obj;
				return marshal_as<String^>(v->VariableType().c_str()); 
			}
		}

		property NodeType NType
		{
			NodeType get() { return _ntype; }
		}
	};

	public ref class CMWrappedDefinition : public CMWrappedIrpObject {
	private:
		double value;
	public:
		CMWrappedDefinition(CMDefinition* d) : CMWrappedIrpObject(d)
		{
			value = d->GetValue();
		}

		property double Value
		{
			double get() { return value; }
		}
	};

	public ref class CMLoadedFile : INotifyPropertyChanged 
	{
	private:
		int _id;
		String^ _path;
	public:
		virtual event PropertyChangedEventHandler^ PropertyChanged;

		void OnPropertyChanged(String^ info)
		{
			PropertyChanged(this, gcnew PropertyChangedEventArgs(info));
		}

		CMLoadedFile() : CMLoadedFile(0, gcnew String(L"")) {}

		CMLoadedFile(int id, String^ path)
		{
			_id = id;
			_path = path;
		}

		property int Id
		{
			int get() { return _id; }
		}

		property String^ Path
		{
			String^ get() { return _path; }
		}

		property String^ FileName
		{
			String^ get()
			{
				int index = Path->LastIndexOf(L"\\");
				return Path->Substring(index + 1);
			}
		}

		virtual String^ ToString() override { return FileName; }
	};

	public ref class CMWrappedIrpApplication
	{
	private:
		CMIrpApplication *app = nullptr;
		CMSimulation *sim = nullptr;
		ObservableCollection<CMWrappedVariable^>^ variableList = gcnew ObservableCollection<CMWrappedVariable^>();
		ObservableCollection<CMLoadedFile^>^ loadedFileList = gcnew ObservableCollection<CMLoadedFile^>();
		ObservableCollection<CMWrappedIrpObject^>^ definitionList = gcnew ObservableCollection<CMWrappedIrpObject^>();
		ObservableCollection<CMWrappedIrpObject^>^ scenarioList = gcnew ObservableCollection<CMWrappedIrpObject^>();
		ObservableCollection<CMWrappedIrpObject^>^ scriptList = gcnew ObservableCollection<CMWrappedIrpObject^>();
		ObservableCollection<CMWrappedIrpObject^>^ categoryList = gcnew ObservableCollection<CMWrappedIrpObject^>();

	public:
		CMWrappedIrpApplication()
		{
			app = new CMIrpApplication();
		}

#pragma region Properties

		property String^ ProjectName
		{
			String^ get() { return marshal_as<String^>(app->GetProjectFile().c_str()); }
		}

		property ObservableCollection<CMWrappedVariable^>^ Variables
		{
			ObservableCollection<CMWrappedVariable^>^ get() { return variableList; }
		}
		
		property ObservableCollection<CMWrappedIrpObject^>^ Definitions
		{
			ObservableCollection<CMWrappedIrpObject^>^ get() { return definitionList; }
		}

		property ObservableCollection<CMWrappedIrpObject^>^ Scenarions
		{
			ObservableCollection<CMWrappedIrpObject^>^ get() { return scenarioList; }
		}

		property ObservableCollection<CMWrappedIrpObject^>^ Scripts
		{
			ObservableCollection<CMWrappedIrpObject^>^ get() { return scriptList; }
		}

		property ObservableCollection<CMWrappedIrpObject^>^ Categories
		{
			ObservableCollection<CMWrappedIrpObject^>^ get() { return categoryList; }
		}

		property ObservableCollection<CMLoadedFile^>^ LoadedFiles
		{		
			ObservableCollection<CMLoadedFile^>^ get() { return loadedFileList; }
		}

#pragma endregion


#pragma region Commands

		void OpenProject(String^ fileName)
		{
			std::wstring str = marshal_as<std::wstring>(fileName);
			app->OpenProject(str.c_str());
		}

		void AfterOpenProject() 
		{
			CMVariableIterator iter;
			CMVariable* v;
			CMDefinition* d;
			int n;
		
			while ((v = iter()) != 0)
				variableList->Add(gcnew CMWrappedVariable(v));

			for (int i = 0; (d = CMDefinitions::GetDefinition(i)) != 0; i++)
				definitionList->Add(gcnew CMWrappedDefinition(d));

			n = app->ScenariosCount();
			for (int i = 0; i < n; i++)
				scenarioList->Add(gcnew CMWrappedIrpObject(app->Scenario(i)));

			n = app->ScriptsCount();
			for (int i = 0; i < n; i++)
				scriptList->Add(gcnew CMWrappedIrpObject(app->Script(i)));

			n = app->Categories();
			for (int i = 0; i < n; i++)
				categoryList->Add(gcnew CMWrappedIrpObject(app->Category(i)));

			n = app->LoadedFilesCount();
			for (int i = 0; i < n; i++)
			{
				loadedFileList->Add(gcnew CMLoadedFile(
					i,
					marshal_as<String^>(app->LoadedFile(i).c_str())));
			}
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

#pragma endregion


		// TODO: Add your methods for this class here.
	};
}
