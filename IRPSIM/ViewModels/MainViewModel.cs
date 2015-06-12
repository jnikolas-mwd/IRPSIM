using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Diagnostics;
using System.Collections.ObjectModel;
using IrpsimEngineWrapper;
using System.ComponentModel;
using System.Windows.Data;
using IRPSIM.ViewModels.Services;
using System.Windows;
using System.Globalization;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using System.Windows.Media;

namespace IRPSIM.ViewModels
{
    class MainViewModel : ViewModelBase
    {
        //*** TODO: Abstract out as an IFilesViewModel interface injected as a dependency through MainViewModel constructor
        private FilesViewModel filesViewModel;
         
        private CMNotifierDelegate _notifierDelegate;

        private BackgroundWorker backgroundWorkerOpenProject = new BackgroundWorker();
        private BackgroundWorker backgroundWorkerRunSimulation = new BackgroundWorker();

        private List<ErrorViewModel> _threadsafeerrors = new List<ErrorViewModel>();

        private IGetFileName _getFileNameService;
                               
        private int _notify(int ntype, String msg, int data)
        {
            if (ntype == 1 || ntype == 2)
                _threadsafeerrors.Add(new ErrorViewModel(ntype, msg));
            else if (ntype == 3 || ntype == 4)
                this.Log += (msg + System.Environment.NewLine);
            else if (ntype == 6)
                this.Progress = data;
            return 0;
        }

        private CMWrappedIrpApplication irpApp = new CMWrappedIrpApplication();
        private CMWrappedNotifier _notifier;

        public MainViewModel()
        {
            _getFileNameService = new OpenFileDialogService();
            filesViewModel = new FilesViewModel(_getFileNameService);

            _notifierDelegate = new CMNotifierDelegate(_notify);
            _notifier = new CMWrappedNotifier(_notifierDelegate);
    
            Errors = new ObservableCollection<ErrorViewModel>();

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
                    Debug.WriteLine(obj.FileIndex);
                RaisePropertyChanged("SelectedObject");
            }
        }

        public object TestSel
        {
            get { return null; }
            set
            {
                Debug.WriteLine(value);
            }
        }

        public ObservableCollection<CMWrappedVariable> Variables { get { return irpApp.Variables; } }

        public ObservableCollection<CMLoadedFile> LoadedFiles { get { return irpApp.LoadedFiles; } }

        public ObservableCollection<CMWrappedIrpObject> Definitions { get { return irpApp.Definitions; } }

        public ObservableCollection<CMWrappedIrpObject> Scenarios { get { return irpApp.Scenarios; } }

        public ObservableCollection<CMWrappedIrpObject> Scripts { get { return irpApp.Scripts; } }

        public ObservableCollection<CMWrappedIrpObject> Categories { get { return irpApp.Categories; } }

        public ObservableCollection<ErrorViewModel> Errors { get; set; }

        private string _log;
        public string Log
        {
            get { return _log; }
            set
            {
                _log = value;
                RaisePropertyChanged("Log");
            }
        }

        private int _progress;
        public int Progress
        {
            get { return _progress; }
            set
            {
                _progress = value;
                RaisePropertyChanged("Progress");
            }
        }

        private Boolean _haserrors = false;
        public Boolean HasErrors
        {
            get { return _haserrors; }
            set
            {
                if (value != _haserrors)
                {
                    _haserrors = value;
                    RaisePropertyChanged("HasErrors");
                }
            }
        }

        public string ProjectTitle
        {
            get { string ret = irpApp.ProjectFile; return ret == "" ? "IRPSIM" : ret; }
        }

        bool _isSimulationRunning = false;
        public bool IsSimulationRunning
        {
            get { return _isSimulationRunning; }
            set
            {
                if (value != _isSimulationRunning)
                {
                    _isSimulationRunning = value;
                    RaisePropertyChanged("IsSimulationRunning");
                }
            }
        }

        bool _canRunSimulation = false;
        public bool CanRunSimulation
        {
            get { return _canRunSimulation; }
            set
            {
                if (value != _canRunSimulation)
                {
                    _canRunSimulation = value;
                    RaisePropertyChanged("CanRunSimulation");
                    RunSimulationCommand.RaiseCanExecuteChanged();
                }
            }
        }
        
        bool _canOpenProject = true;
        public bool CanOpenProject
        {
            get { return _canOpenProject; }
            set
            {
                if (value != _canOpenProject)
                {
                    _canOpenProject = value;
                    RaisePropertyChanged("CanOpenProject");
                    ProjectOpenCommand.RaiseCanExecuteChanged();
                }
            }
        }
 
        #endregion

        #region Commands

        RelayCommand<object> _testCommand;
        public RelayCommand<object> TestCommand
        {
            get
            {
                if (_testCommand == null)
                    _testCommand = new RelayCommand<object>(testDelegate);
                return _testCommand;
            }
        }

        RelayCommand<object> _testCommand2;
        public RelayCommand<object> TestCommand2
        {
            get
            {
                if (_testCommand2 == null)
                    _testCommand2 = new RelayCommand<object>(testDelegate2);
                return _testCommand2;
            }
        }
        
        RelayCommand _projectOpenCommand;
        public RelayCommand ProjectOpenCommand
        {
            get {
                if (_projectOpenCommand==null)
                    _projectOpenCommand = new RelayCommand(openProjectDelegate, () => { return CanOpenProject; });
                return _projectOpenCommand;
            }
        }

        RelayCommand _projectCloseCommand;
        public RelayCommand ProjectCloseCommand
        {
            get
            {
                if (_projectCloseCommand == null)
                    _projectCloseCommand = new RelayCommand(closeProjectDelegate);
                return _projectCloseCommand;
            }
        }

        RelayCommand _projectReloadCommand;
        public RelayCommand ProjectReloadCommand
        {
            get
            {
                if (_projectReloadCommand == null)
                    _projectReloadCommand = new RelayCommand(reloadProjectDelegate);
                return _projectReloadCommand;
            }
        }
        
        RelayCommand _runSimulationCommand;
        public RelayCommand RunSimulationCommand
        {
            get
            {
                if (_runSimulationCommand == null)
                    _runSimulationCommand = new RelayCommand(runSimulationDelegate, () => { return CanRunSimulation; });
                return _runSimulationCommand;
            }
        }

        #endregion

        #region Command Delegates

        private void testDelegate(object param)
        {
            CMWrappedIrpObject obj = param as CMWrappedIrpObject;
            if (obj != null)
                obj.Selected = !obj.Selected;
        }

        private void testDelegate2(object param)
        {
            CMWrappedIrpObject obj = SelectedObject as CMWrappedIrpObject;
            if (obj == null)
                return;

            KeyEventArgs a = param as KeyEventArgs;

            if (a.Key == Key.Space)
                obj.Selected = !obj.Selected;
        }

        private void openProjectDelegate()
        {
            if (!CanOpenProject)
            {
                Debug.WriteLine("Can't open project");
                return;
            }

            string path = _getFileNameService.GetFileName();

            if (path != null)
            {
                CanOpenProject = false;
                CanRunSimulation = false;
                backgroundWorkerOpenProject.RunWorkerAsync(path);
            }
        }

        private void closeProjectDelegate()
        {
            Log = "";
            Errors.Clear();
            irpApp.CloseProject();
            CanOpenProject = true;
            CanRunSimulation = false;
        }

        private void reloadProjectDelegate()
        {
            string path = irpApp.ProjectFile;
            closeProjectDelegate();
            if (path != "")
            {
                CanOpenProject = false;
                CanRunSimulation = false;
                backgroundWorkerOpenProject.RunWorkerAsync(path);
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
            _threadsafeerrors.Clear();
            Errors.Clear();

            IsSimulationRunning = true;

            backgroundWorkerRunSimulation.RunWorkerAsync();
        }

        #endregion
        
        #region Methods

        public string IrpObjectType(object obj)
        {
            CMWrappedIrpObject wo = obj as CMWrappedIrpObject;

            if (wo == null)
                return "None";

            if (wo is CMWrappedVariable)
            {
                CMWrappedVariable v = wo as CMWrappedVariable;

                if (v.NType == IrpNodeType.Demand)
                    return "Demand";
                if (v.NType == IrpNodeType.Supply)
                    return "Supply";
                if (v.NType == IrpNodeType.Storage)
                    return "Storage";
                if (v.NType == IrpNodeType.Cost)
                    return "Cost";
                return (v.FileId >= 0 ? "UserVariable" : "SystemVariable");
            }

            return wo.Type.Substring(2);
        }

        #endregion

        private void backgroundWorkerOpenProject_DoWork(object sender, DoWorkEventArgs e)
        {
            irpApp.OpenProject(e.Argument as string); 
        }

        private void backgroundWorkerOpenProject_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            RaisePropertyChanged("ProjectTitle");
            irpApp.AfterOpenProject();
            CanOpenProject = true;

            foreach (ErrorViewModel m in _threadsafeerrors)
                Errors.Add(m);

            if (_threadsafeerrors.Count > 0)
            {
                HasErrors=true;
                CanRunSimulation = false;
            }
            else
                CanRunSimulation=true;
         }

        private void backgroundWorkerRunSimulation_DoWork(object sender, DoWorkEventArgs e)
        {
            irpApp.UseScenario("test");
            irpApp.UseScript("basemix-cra-ondemand");
            irpApp.RunSimulation();
        }

        private void backgroundWorkerRunSimulation_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            foreach (ErrorViewModel m in _threadsafeerrors)
                Errors.Add(m);
 
            if (_threadsafeerrors.Count>0)
                HasErrors = true;

            IsSimulationRunning = false;
        }

        private void openProject(string filename)
        {
            irpApp.OpenProject(filename); 
        }
    }
}
