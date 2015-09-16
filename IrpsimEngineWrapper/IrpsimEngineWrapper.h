// IrpsimEngineWrapper.h
#pragma once

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <irpapp.h>
#include <notify.h>
#include <defines.h>
#include <msclr\marshal_cppstd.h>

//#include <fstream>
//static wofstream sdebug("debug_wrapper.txt");

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;
using namespace msclr::interop;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::ObjectModel;

namespace IrpsimEngineWrapper	
{
	public delegate int CMNotifierDelegate(int type, String^ msg, int data);
	typedef int(__stdcall  *CMNotifierCallback)(CMNotifier::ntype type, String^  msg, int data);

	public enum class IrpNodeType { None, Demand, Supply, Storage, Cost };

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
		bool _selected;
		bool _chosen;
	public:
		virtual event PropertyChangedEventHandler^ PropertyChanged;
		
		void OnPropertyChanged(String^ info)
		{
			PropertyChanged(this, gcnew PropertyChangedEventArgs(info));
		}

		CMWrappedIrpObject(CMIrpObject* o)
		{
			obj = o;
			_selected = false;
		}
		
		property int FileId
		{
			int get() { return obj->GetApplicationId(); }
		}

		property long FileIndex
		{
			long get() { return obj->GetApplicationIndex(); }
		}

		/*
		virtual property bool Selected
		{
			bool get() { return _selected; }
			void set(bool value) {
				if (_selected != value) {
					_selected = value;
					OnPropertyChanged(L"Selected");
				}
			}
		}
		*/

		property bool Chosen
		{
			bool get() { return _chosen; }
			void set(bool value) {
				if (_chosen != value) {
					_chosen = value;
					OnPropertyChanged(L"Chosen");
				}
			}
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

		property CMIrpObject* UnmanagedObject
		{
			CMIrpObject* get() { return obj; }
		}
	};
	
	public ref class CMWrappedOption : public CMWrappedIrpObject
	{
	public:
		CMWrappedOption(CMOption* o) : CMWrappedIrpObject(o)
		{
		}

		property String^ Value
		{
			String^ get() 
			{
				CMOption* o = (CMOption*)obj;
				return marshal_as<String^>(o->GetValue().c_str());
			}
			void set(String^ value)
			{
				std::wstring str = marshal_as<std::wstring>(value);
				CMOption* o = (CMOption*)obj;
				o->SetValue(str.c_str());
			}
		}
	};

	public ref class CMWrappedVariable : public CMWrappedIrpObject {
	private:
		bool _saveOutcomes = false;
		IrpNodeType _ntype = IrpNodeType::None;
		Collection<CMWrappedVariable^>^ _associatedVariables = gcnew Collection<CMWrappedVariable^>();
	public:
		CMWrappedVariable(CMVariable* v) : CMWrappedIrpObject(v)
		{
			if (v->IsType(L"Demand")) _ntype = IrpNodeType::Demand;
			else if (v->IsType(L"Supply")) _ntype = IrpNodeType::Supply;
			else if (v->IsType(L"Storage")) _ntype = IrpNodeType::Storage;
			else if (v->IsType(L"Cost")) _ntype = IrpNodeType::Cost;
		}

		property Collection<CMWrappedVariable^>^ AssociatedVariables
		{
			Collection<CMWrappedVariable^>^ get() { return _associatedVariables; }
		}

		property String^ EType
		{
			String^ get() 
			{ 
				CMVariable* v = (CMVariable*)obj;
				return marshal_as<String^>(v->VariableType().c_str()); 
			}
		}

		property IrpNodeType NType
		{
			IrpNodeType get() { return _ntype; }
		}

		property BOOL IsNode
		{
			BOOL get() { return NType != IrpNodeType::None; }
		}

		property BOOL IsAggregate
		{
			BOOL get() { return ((CMVariable*)UnmanagedObject)->IsAggregate(); }
		}

		property BOOL IsRegional
		{
			BOOL get() { return ((CMVariable*)UnmanagedObject)->IsRegional(); }
		}

		property BOOL IsSystem
		{
			BOOL get() { return ((CMVariable*)UnmanagedObject)->IsSystem(); }
		}

		property bool SaveOutcomes 
		{
			bool get() { return _saveOutcomes; }
			void set(bool value) {
				((CMVariable*)obj)->SetState(CMVariable::vsSelected | CMVariable::vsSaveOutcomes, value);
				if (_saveOutcomes != value) {
					_saveOutcomes = value;
					OnPropertyChanged(L"SaveOutcomes");
				}
			}
		}

		void ToggleSaveOutcomes()
		{
			SaveOutcomes = !SaveOutcomes;
		}

		void AddAssociatedVariable(CMWrappedVariable^ v)
		{
			//sdebug << "Adding associated variable to " << UnmanagedObject->GetName() << " " << v->UnmanagedObject->GetName() << endl;
			_associatedVariables->Add(v);
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

	public ref class CMVariableSummary
	{
	private:
		double _mean, _min, _max, _stderr;

	public:
		CMVariableSummary() {}
		CMVariableSummary(double mean, double min, double max, double se) : _mean(mean), _min(min), _max(max), _stderr(se) {}

		property double Mean
		{
			double get() { return _mean; }
			void set(double value) { _mean = value; }
		}

		property double Minimum
		{
			double get() { return _min; }
			void set(double value) { _min = value; }
		}

		property double Maximum
		{
			double get() { return _max; }
			void set(double value) { _max = value; }
		}

		property double StandardError
		{
			double get() { return _stderr; }
			void set(double value) { _stderr = value; }
		}
	};

	public ref class CMWrappedSimulation : public CMWrappedIrpObject 
	{
	private:
		List<String^>^ _variables = gcnew List<String^>();
		List<CMWrappedOption^>^ _options = gcnew List<CMWrappedOption^>();
	public:
		CMWrappedSimulation(CMSimulation* s) : CMWrappedIrpObject(s)
		{
			CMAccumulatorArray *accum = s->Accumulator();
			for (unsigned i = 0; i < accum->Variables(); i++) 
				_variables->Add(gcnew String(accum->GetVariableName(i).c_str()));
			const CMOptions& options = s->GetOptions();
			for (unsigned i = 0; i < options.Count(); i++)
				_options->Add(gcnew CMWrappedOption(options.At(i)));
		}

		List<CMVariableSummary^>^ GetSummaries(String^ variableName)
		{
			CMSimulation* sim = (CMSimulation*)obj;
			CMAccumulatorArray* accumulator = sim->Accumulator();
			std::wstring _variableName = marshal_as<std::wstring>(variableName);
			int variableindex = accumulator->VariableIndex(_variableName.c_str());

			List<CMVariableSummary^>^ list = gcnew List<CMVariableSummary^>();

			unsigned timesteps = accumulator->TimeSteps();

			for (unsigned i = 0; i < timesteps; i++) 
			{
				CMVariableSummary^ summary = gcnew CMVariableSummary(accumulator->Mean(i, variableindex),
					accumulator->Min(i, variableindex),
					accumulator->Max(i, variableindex),
					accumulator->StdDev(i, variableindex));
				list->Add(summary);
			}

			return list;
		}

		property List<String^>^ Variables
		{
			List<String^>^ get() { return _variables; }
		}

		property List<CMWrappedOption^>^ Options
		{
			List<CMWrappedOption^>^ get() { return _options; }
		}

		property int TimeSteps
		{
			int get() { return ((CMSimulation*)obj)->Accumulator()->TimeSteps(); }
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

		String^ ToString() override { return FileName; }
	};
	
	public ref class LoadedFileCollection : public ObservableCollection < CMLoadedFile^ >
	{

	};
	
	public ref class IrpObjectCollection : public ObservableCollection < CMWrappedIrpObject^ >
	{
	public:
		IrpObjectCollection() : ObservableCollection < CMWrappedIrpObject^ >()
		{
		}
	};

	public ref class IrpObjectDictionary : public Dictionary < String^, CMWrappedIrpObject^ >
	{
	public:
		IrpObjectDictionary() : Dictionary<String^, CMWrappedIrpObject^>()
		{
		}
	};

	public ref class CMWrappedIrpApplication
	{
	private:
		CMIrpApplication *app = nullptr;
		CMSimulation *currentSimulation = nullptr;
		CMWrappedSimulation^ currentWrappedSimulation = nullptr;

		IrpObjectDictionary^ variableDictionary = gcnew IrpObjectDictionary();

		IrpObjectCollection^ variableList = gcnew IrpObjectCollection();
		IrpObjectCollection^ aggregateList = gcnew IrpObjectCollection();
		IrpObjectCollection^ systemVariableList = gcnew IrpObjectCollection();
		IrpObjectCollection^ demandList = gcnew IrpObjectCollection();
		IrpObjectCollection^ supplyList = gcnew IrpObjectCollection();
		IrpObjectCollection^ storageList = gcnew IrpObjectCollection();
		IrpObjectCollection^ costList = gcnew IrpObjectCollection();
		
		LoadedFileCollection^ loadedFileList = gcnew LoadedFileCollection();
		IrpObjectCollection^ definitionList = gcnew IrpObjectCollection();
		IrpObjectCollection^ scenarioList = gcnew IrpObjectCollection();
		IrpObjectCollection^ scriptList = gcnew IrpObjectCollection();
		IrpObjectCollection^ categoryList = gcnew IrpObjectCollection();
		IrpObjectCollection^ simulationList = gcnew IrpObjectCollection();
		IrpObjectCollection^ optionsList = gcnew IrpObjectCollection();

	public:

		CMWrappedIrpApplication()
		{
			app = new CMIrpApplication();
		}

#pragma region Properties

		property String^ ProjectFile
		{
			String^ get() { return marshal_as<String^>(app->GetProjectFile().c_str()); }
		}

		property CMWrappedSimulation^ CurrentSimulation
		{
			CMWrappedSimulation^ get() { return currentWrappedSimulation; }
		}

	    property IrpObjectDictionary^ VariableDictionary
		{
			IrpObjectDictionary^ get() { return variableDictionary; }
		}

		property IrpObjectCollection^ Variables
		{
			IrpObjectCollection^ get() { return variableList; }
		}

		property IrpObjectCollection^ SystemVariables
		{
			IrpObjectCollection^ get() { return systemVariableList; }
		}
		
		property IrpObjectCollection^ AggregateVariables
		{
			IrpObjectCollection^ get() { return aggregateList; }
		}

		property IrpObjectCollection^ DemandVariables
		{
			IrpObjectCollection^ get() { return demandList; }
		}

		property IrpObjectCollection^ SupplyVariables
		{
			IrpObjectCollection^ get() { return supplyList; }
		}

		property IrpObjectCollection^ StorageVariables
		{
			IrpObjectCollection^ get() { return storageList; }
		}

		property IrpObjectCollection^ CostVariables
		{
			IrpObjectCollection^ get() { return costList; }
		}
		
		property IrpObjectCollection^ Definitions
		{
			IrpObjectCollection^ get() { return definitionList; }
		}

		property IrpObjectCollection^ Scenarios
		{
			IrpObjectCollection^ get() { return scenarioList; }
		}

		property IrpObjectCollection^ Scripts
		{
			IrpObjectCollection^ get() { return scriptList; }
		}

		property IrpObjectCollection^ Categories
		{
			IrpObjectCollection^ get() { return categoryList; }
		}

		property IrpObjectCollection^ Options
		{
			IrpObjectCollection^ get() { return optionsList; }
		}

		property IrpObjectCollection^ Simulations
		{
			IrpObjectCollection^ get() { return simulationList; }
		}

		property LoadedFileCollection^ LoadedFiles
		{		
			LoadedFileCollection^ get() { return loadedFileList; }
		}

#pragma endregion


#pragma region Methods

		void SetSaveArchive(bool value) { CMSimulation::SetSaveArchive(value); }

		void SetSaveOutcomes(bool value) { CMSimulation::SetSaveOutcomes(value); }

		void SetSaveSummary(bool value) { CMSimulation::SetSaveSummary(value); }

		void OpenProject(String^ fileName)
		{
			std::wstring str = marshal_as<std::wstring>(fileName);
			app->OpenProject(str.c_str());
		}

		String^ GetFilePath(int id)
		{
			return marshal_as<String^>(app->LoadedFile(id).c_str());
		}

		void CloseProject() {
			AggregateVariables->Clear();
			DemandVariables->Clear();
			SupplyVariables->Clear();
			StorageVariables->Clear();
			CostVariables->Clear();
			Variables->Clear();
			Definitions->Clear();
			Scenarios->Clear();
			Scripts->Clear();
			Categories->Clear();
			Options->Clear();
			LoadedFiles->Clear();
			VariableDictionary->Clear();
			Simulations->Clear();
			app->ResetApplication();
		}

		void AfterOpenProject() 
		{
			CMVariableIterator iter;
			CMVariable* v;
			CMDefinition* d;
			int n;
		
			while ((v = iter()) != 0) {
				CMWrappedVariable^ wv = gcnew CMWrappedVariable(v);
				VariableDictionary->Add(wv->Name, wv);
				if (wv->NType == IrpNodeType::Demand) DemandVariables->Add(wv);
				else if (wv->NType == IrpNodeType::Supply) SupplyVariables->Add(wv);
				else if (wv->NType == IrpNodeType::Storage) StorageVariables->Add(wv);
				else if (wv->NType == IrpNodeType::Cost) CostVariables->Add(wv);
				else if (wv->IsAggregate && !wv->IsRegional) AggregateVariables->Add(wv);
				else if (wv->IsSystem) SystemVariables->Add(wv);
				else if (wv->FileId>=0) Variables->Add(wv);
			}

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

			CMOptions* options = app->Options();
			n = options->Count();
			for (unsigned i = 0; i < options->Count(); i++)
				optionsList->Add(gcnew CMWrappedOption(options->At(i)));

			for each (CMWrappedVariable^ wv in VariableDictionary->Values) 
			{
				int i = wv->Name->IndexOf('.');
				if (i > 0) {
					String^ prefix = wv->Name->Substring(0, i);
					if (VariableDictionary->ContainsKey(prefix))
					{
						CMWrappedVariable^ found = (CMWrappedVariable^)variableDictionary[prefix];
						if (found)
							found->AddAssociatedVariable(wv);
					}
				}
			}

			for each (CMWrappedVariable^ wv in VariableDictionary->Values)
			{
				if (wv->IsNode) {
					CMVariable* vUnmanaged = (CMVariable*)wv->UnmanagedObject;
					CMString name, value;
					for (int i = 0; vUnmanaged->GetAssociation(i, name, value) > 0; i++) {
						if (_wcsicmp(name.c_str(), L"region") && _wcsicmp(name.c_str(), L"category")) {
							String^ strValue = gcnew String(value.c_str());
							if (VariableDictionary->ContainsKey(strValue)) {
								CMWrappedVariable^ found = (CMWrappedVariable^)variableDictionary[strValue];
								if (found)
									wv->AddAssociatedVariable(found);
							}
						}
					}
				}
			}
		}

		void UseScenario(String^ name)
		{
			std::wstring str = marshal_as<std::wstring>(name);
			app->UseScenario(str.c_str());

			for each (CMWrappedVariable^ var in VariableDictionary->Values)
			{
				CMVariable* v = (CMVariable*)var->UnmanagedObject;
				var->SaveOutcomes = (v->GetState() & CMVariable::vsSaveOutcomes) ? true : false;
			}
		}

		void UseScript(String^ name)
		{
			std::wstring str = marshal_as<std::wstring>(name);
			app->UseScript(str.c_str());
		}

		void SetOption(String^ name, String^ value)
		{
			std::wstring cname = marshal_as<std::wstring>(name);
			std::wstring cval = marshal_as<std::wstring>(value);
			app->SetOption(cname.c_str(), cval.c_str());
		}

		String^ GetOption(String^ name)
		{
			std::wstring cname = marshal_as<std::wstring>(name);
			return marshal_as<String^>(app->GetOption(cname.c_str()).c_str());
		}

		void RunSimulation()
		{
			currentSimulation = app->CreateSimulation();
			if (currentSimulation == NULL)
				return;
			
			BOOL val = app->RunSimulation(currentSimulation);
		}

		void AfterRunSimulation()
		{
			if (currentSimulation != NULL) {
				currentWrappedSimulation = gcnew CMWrappedSimulation(currentSimulation);
				simulationList->Add(currentWrappedSimulation);
			}
		}

		#pragma endregion
	};
}
