using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Diagnostics;
using System.Collections.ObjectModel;
using Microsoft.Win32;
using IrpsimEngineWrapper;
using System.ComponentModel;
using System.Windows.Data;

namespace IRPSIM.ViewModels
{
    class MainViewModel : ObservableObject
    {
        private FilesViewModel filesViewModel = new FilesViewModel();
         
        private CMNotifierDelegate del;

        private BackgroundWorker backgroundWorkerOpenProject = new BackgroundWorker();
        private BackgroundWorker backgroundWorkerRunSimulation = new BackgroundWorker();

        private Dictionary<int, string> _notifyTypes = new Dictionary<int,string>();
                               
        private int test(int ntype, String msg, int data)
        {
            if (ntype == 1 || ntype == 2)
                this.Errors += (msg + System.Environment.NewLine);
            else if (ntype == 3 || ntype == 4)
                this.Log += (msg + System.Environment.NewLine);
            else if (ntype == 6)
                this.Progress = data;
            return 0;
        }

        private CMWrappedIrpApplication irpApp = new CMWrappedIrpApplication();
        private CMWrappedNotifier notifier;

        public MainViewModel()
        {
           del = new CMNotifierDelegate(test);
           notifier = new CMWrappedNotifier(del);
            _notifyTypes.Add(1, "ERROR");
            _notifyTypes.Add(2, "WARNING");
            _notifyTypes.Add(3, "LOG");
            _notifyTypes.Add(4, "LOGTIME");
            _notifyTypes.Add(5, "INFO");
            _notifyTypes.Add(6, "PROGRESS");

            SortDescription sd = new SortDescription("Name", ListSortDirection.Ascending);

            this.SupplyVariables = new CollectionViewSource();
            this.SupplyVariables.Source = irpApp.Variables;
            this.SupplyVariables.Filter += new FilterEventHandler(delegate(object item, FilterEventArgs e) { e.Accepted = (((CMWrappedVariable)e.Item).NType == NodeType.Supply); });
            this.SupplyVariables.SortDescriptions.Add(sd);

            this.DemandVariables = new CollectionViewSource();
            this.DemandVariables.Source = irpApp.Variables;
            this.DemandVariables.Filter += new FilterEventHandler(delegate(object item, FilterEventArgs e) { e.Accepted = (((CMWrappedVariable)e.Item).NType == NodeType.Demand); });
            this.DemandVariables.SortDescriptions.Add(sd);

            this.StorageVariables = new CollectionViewSource();
            this.StorageVariables.Source = irpApp.Variables;
            this.StorageVariables.Filter += new FilterEventHandler(delegate(object item, FilterEventArgs e) { e.Accepted = (((CMWrappedVariable)e.Item).NType == NodeType.Storage); });
            this.StorageVariables.SortDescriptions.Add(sd);

            this.CostVariables = new CollectionViewSource();
            this.CostVariables.Source = irpApp.Variables;
            this.CostVariables.Filter += new FilterEventHandler(delegate(object item, FilterEventArgs e) { e.Accepted = (((CMWrappedVariable)e.Item).NType == NodeType.Cost); });
            this.CostVariables.SortDescriptions.Add(sd);

            this.SupportingVariables = new CollectionViewSource();
            this.SupportingVariables.Source = irpApp.Variables;
            this.SupportingVariables.Filter += new FilterEventHandler(delegate(object item, FilterEventArgs e) { e.Accepted = (((CMWrappedVariable)e.Item).NType == NodeType.None && ((CMWrappedVariable)e.Item).FileId>=0); });
            this.SupportingVariables.SortDescriptions.Add(sd);

            this.LoadedFiles = new CollectionViewSource();
            this.LoadedFiles.Source = irpApp.LoadedFiles;
            this.LoadedFiles.SortDescriptions.Add(new SortDescription("FileName", ListSortDirection.Ascending));

            this.Definitions = new CollectionViewSource();
            this.Definitions.Source = irpApp.Definitions;
            this.Definitions.SortDescriptions.Add(sd);

            this.Scenarios = new CollectionViewSource();
            this.Scenarios.Source = irpApp.Scenarions;
            this.Scenarios.SortDescriptions.Add(sd);

            this.Scripts = new CollectionViewSource();
            this.Scripts.Source = irpApp.Scripts;
            this.Scripts.SortDescriptions.Add(sd);
            
            this.Categories = new CollectionViewSource();
            this.Categories.Source = irpApp.Categories;
            this.Categories.SortDescriptions.Add(sd);
            
            backgroundWorkerOpenProject.DoWork += backgroundWorkerOpenProject_DoWork;
            backgroundWorkerOpenProject.RunWorkerCompleted += backgroundWorkerOpenProject_RunWorkerCompleted;

            backgroundWorkerRunSimulation.DoWork += backgroundWorkerRunSimulation_DoWork;
            backgroundWorkerRunSimulation.RunWorkerCompleted += backgroundWorkerRunSimulation_RunWorkerCompleted;
        }

        #region Properties

        public FilesViewModel FilesViewModel
        {
            get { return filesViewModel; }
        }

        private object _selectedObject;
        public object SelectedObject
        {
            get { return _selectedObject; }
            set
            {
                _selectedObject = value;
                CMWrappedIrpObject obj = value as CMWrappedIrpObject;
                if (obj != null)
                    Debug.WriteLine(obj.FileId);
                RaisePropertyChangedEvent("SelectedObject");
            }
        }

        public CollectionViewSource SupplyVariables { get; private set; }

        public CollectionViewSource DemandVariables { get; private set; }

        public CollectionViewSource StorageVariables { get; private set; }

        public CollectionViewSource CostVariables { get; private set; }

        public CollectionViewSource SupportingVariables { get; private set; }

        public CollectionViewSource LoadedFiles { get; private set; }

        public CollectionViewSource Definitions { get; private set; }

        public CollectionViewSource Scripts { get; private set; }

        public CollectionViewSource Scenarios { get; private set; }

        public CollectionViewSource Categories { get; private set; }
 
        /*
        public ObservableCollection<CMWrappedVariable> Variables
        {
            get { return irpApp.GetVariables(); }
        }
        */

        /*
        public ObservableCollection<CMLoadedFile> LoadedFiles
        {
            get { return irpApp.GetLoadedFiles(); }
        }
        */

        private string _log;
        public string Log
        {
            get { return _log; }
            set
            {
                _log = value;
                RaisePropertyChangedEvent("Log");
            }
        }

        private string _errors;
        public string Errors
        {
            get { return _errors; }
            set
            {
                _errors = value;
                RaisePropertyChangedEvent("Errors");
            }
        }

        private int _progress;
        public int Progress
        {
            get { return _progress; }
            set
            {
                _progress = value;
                RaisePropertyChangedEvent("Progress");
            }
        }

        public string ProjectTitle
        {
            get { string ret = irpApp.ProjectName; return ret == "" ? "IRPSIM" : ret; }
        }
 
        #endregion

        public ICommand ProjectOpenCommand
        {
            get { return new DelegateCommand(param => openProjectDelegate()); }
        }

        public ICommand RunSimulationCommand
        {
            get { return new DelegateCommand(param => runSimulationDelegate()); }
        }

        public ICommand DoSomethingCommand
        {
            get { return new DelegateCommand(param => doSomethingDelegate((MouseEventArgs)param)); }
        }

        private void doSomethingDelegate(MouseEventArgs e)
        {
            Debug.WriteLine("Do Something!");
        }

        private void openProjectDelegate()
        {
            if (backgroundWorkerOpenProject.IsBusy)
            {
                Debug.WriteLine("Project is being opened");
                return;
            }

            OpenFileDialog dlg = new OpenFileDialog();

            if (dlg.ShowDialog() == true)
            {
                string filename = dlg.FileName;
                backgroundWorkerOpenProject.RunWorkerAsync(filename);
            }
        }


        private void runSimulationDelegate()
        {
            /*
            if (m_RunSimulationThread != null && m_RunSimulationThread.IsAlive)
            {
                Debug.WriteLine("Simulation is running");
                return;
            }
            */
            backgroundWorkerRunSimulation.RunWorkerAsync();
        }

        private void backgroundWorkerOpenProject_DoWork(object sender, DoWorkEventArgs e)
        {
            irpApp.OpenProject(e.Argument as string); 
        }

        private void backgroundWorkerOpenProject_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            RaisePropertyChangedEvent("ProjectTitle");
            irpApp.AfterOpenProject();
        }

        private void backgroundWorkerRunSimulation_DoWork(object sender, DoWorkEventArgs e)
        {
            irpApp.UseScenario("dsm_input");
            irpApp.UseScript("basemix-cra-ondemand");
            irpApp.RunSimulation();
        }

        private void backgroundWorkerRunSimulation_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            //TODO
        }

        private void openProject(string filename)
        {
            irpApp.OpenProject(filename); 
        }
    }
}
