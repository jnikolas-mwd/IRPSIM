using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IRPSIM.Models;
using IrpsimEngineWrapper;
using System.Diagnostics;

namespace IRPSIM.Services
{
    public class BoolEventArgs : EventArgs 
    {
        public BoolEventArgs(bool val) : base() { Value = val; }
        public bool Value { get; set; }
    }

    public class IntEventArgs : EventArgs
    {
        public IntEventArgs(int val) : base() { Value = val; }
        public int Value { get; set; }
    }

    public class StringEventArgs : EventArgs
    {
        public StringEventArgs(String val) : base() { Value = val; }
        public String Value { get; set; }
    }

    public class ErrorModelEventArgs : EventArgs
    {
        public ErrorModelEventArgs(ErrorModel val) : base() { Value = val; }
        public ErrorModelEventArgs(int severity, string msg) : base() { Value = new ErrorModel(severity,msg); }
        public ErrorModel Value { get; set; }
    }

    public delegate void IrpLogEventHandler(object sender, StringEventArgs e);
    public delegate void IrpErrorEventHandler(object sender, ErrorModelEventArgs e);
    public delegate void IrpSimulationProgressEventHandler(object sender, IntEventArgs e);
    public delegate void IrpProjectLoadedEventHandler(object sender, BoolEventArgs e);
    public delegate void IrpSimulationCompletedEventHandler(object sender, BoolEventArgs e);

    public interface ICoreApplicationService
    {
        event IrpLogEventHandler IrpLog;
        event IrpErrorEventHandler IrpError;
        event IrpSimulationProgressEventHandler IrpSimulationProgress;
        event IrpProjectLoadedEventHandler IrpProjectLoaded;
        event EventHandler IrpProjectClosed;
        event IrpSimulationCompletedEventHandler IrpSimulationCompleted;
 
        String ProjectFile { get; }

        IrpObjectDictionary VariableDictionary { get; }

        IrpObjectCollection Variables { get; }

        IrpObjectCollection AggregateVariables { get; }

        IrpObjectCollection DemandVariables { get; }

        IrpObjectCollection SupplyVariables { get; }

        IrpObjectCollection StorageVariables { get; }

        IrpObjectCollection CostVariables { get; }

        IrpObjectCollection Definitions { get; }

        IrpObjectCollection Scenarios { get; }

        IrpObjectCollection Scripts { get; }

        IrpObjectCollection Categories { get; }

        IrpObjectCollection Options { get; }

        IrpObjectCollection Simulations { get; }

        LoadedFileCollection LoadedFiles { get; }

        Boolean HasErrors { get; }

        String GetFilePath(int id);

        void OpenProject(String path);

        void CloseProject();

        void UseScenario(String name);

        void UseScript(String name);

        void SetOption(string name, string value);

        String GetOption(string name);

        void RunSimulation();
    }

    public class CoreApplicationService : ICoreApplicationService
    {
        private CMWrappedIrpApplication _app = new CMWrappedIrpApplication();
        private CMNotifierDelegate _notifierDelegate;
        private CMWrappedNotifier _notifier;

        private int _notify(int ntype, String msg, int data)
        {
            if (ntype == 1 || ntype == 2) {
                if (IrpError != null) IrpError(this, new ErrorModelEventArgs(ntype,msg));
                if (ntype == 1) _hasErrors = true;
            }
            else if (ntype == 3 || ntype == 4) {
                if (IrpLog != null) IrpLog(this, new StringEventArgs(msg));
            }
            else if (ntype == 6) {
                if (IrpSimulationProgress != null) IrpSimulationProgress(this, new IntEventArgs(data));
            }
            return 0;
        }

        public event IrpLogEventHandler IrpLog;
        public event IrpErrorEventHandler IrpError;
        public event IrpSimulationProgressEventHandler IrpSimulationProgress;
        public event IrpProjectLoadedEventHandler IrpProjectLoaded;
        public event EventHandler IrpProjectClosed;
        public event IrpSimulationCompletedEventHandler IrpSimulationCompleted;

        public CoreApplicationService()
        {
            _notifierDelegate = new CMNotifierDelegate(_notify);
            _notifier = new CMWrappedNotifier(_notifierDelegate);
        }
        
        public String ProjectFile { get { return _app.ProjectFile; } }

        public IrpObjectDictionary VariableDictionary { get { return _app.VariableDictionary; } }

        public IrpObjectCollection Variables { get { return _app.Variables; } }

        public IrpObjectCollection AggregateVariables { get { return _app.AggregateVariables; } }

        public IrpObjectCollection DemandVariables { get { return _app.DemandVariables; } }

        public IrpObjectCollection SupplyVariables { get { return _app.SupplyVariables; } }

        public IrpObjectCollection StorageVariables { get { return _app.StorageVariables; } }

        public IrpObjectCollection CostVariables { get { return _app.CostVariables; } }

        public IrpObjectCollection Definitions { get { return _app.Definitions; } }

        public IrpObjectCollection Scenarios { get { return _app.Scenarios; } }

        public IrpObjectCollection Scripts { get { return _app.Scripts; } }

        public IrpObjectCollection Categories { get { return _app.Categories; } }

        public IrpObjectCollection Options { get { return _app.Options; } }

        public IrpObjectCollection Simulations { get { return _app.Simulations; } }

        public LoadedFileCollection LoadedFiles { get { return _app.LoadedFiles; } }

        private Boolean _hasErrors;
        public Boolean HasErrors { get { return _hasErrors; } }

        public String GetFilePath(int id) { return _app.GetFilePath(id); }

        public void OpenProject(String path)
        {
            BackgroundWorker bw = new BackgroundWorker();
            bw.DoWork += (s, e) => _app.OpenProject(e.Argument as string);
            bw.RunWorkerCompleted += (s, e) => { _app.AfterOpenProject(); if (IrpProjectLoaded != null) IrpProjectLoaded(this, new BoolEventArgs(true)); };
            bw.RunWorkerAsync(path);
        }

        public void CloseProject()
        {
            _app.CloseProject();
            _hasErrors = false;
            if (IrpProjectClosed != null) IrpProjectClosed(this, EventArgs.Empty);
        }

        public void UseScenario(String name) { _app.UseScenario(name); }

        public void UseScript(String name) { _app.UseScript(name); }

        public void SetOption(string name, string value) { _app.SetOption(name, value); }

        public String GetOption(string name) { return _app.GetOption(name); }

        public void RunSimulation()
        {
            _hasErrors = false;
            BackgroundWorker bw = new BackgroundWorker();
            bw.DoWork += (s, e) => _app.RunSimulation();
            bw.RunWorkerCompleted += (s, e) => { _app.AfterRunSimulation(); if (IrpSimulationCompleted != null) IrpSimulationCompleted(this, new BoolEventArgs(true)); };
            bw.RunWorkerAsync();
        }
    }
}

