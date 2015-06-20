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
        private IrpObjectViewModel _objectViewModel = null;
        private IrpFileViewModel _fileViewModel = null;

        public MainViewModel()
        {
            _getFileNameService = new OpenFileDialogService();
            _displayCollectionViewModel = new DisplayCollectionViewModel(_getFileNameService);

            _notifierDelegate = new CMNotifierDelegate(_notify);
            _notifier = new CMWrappedNotifier(_notifierDelegate);
    
            Errors = new ObservableCollection<ErrorViewModel>();
            _objectViewModel = new IrpObjectViewModel(irpApp);
            _fileViewModel = new IrpFileViewModel(irpApp);

            backgroundWorkerOpenProject.DoWork += backgroundWorkerOpenProject_DoWork;
            backgroundWorkerOpenProject.RunWorkerCompleted += backgroundWorkerOpenProject_RunWorkerCompleted;

            backgroundWorkerRunSimulation.DoWork += backgroundWorkerRunSimulation_DoWork;
            backgroundWorkerRunSimulation.RunWorkerCompleted += backgroundWorkerRunSimulation_RunWorkerCompleted;
        }

        #region Properties

        private DisplayCollectionViewModel _displayCollectionViewModel;
        public DisplayCollectionViewModel DisplayCollectionViewModel
        {
            get { return _displayCollectionViewModel; }
        }

        public object TestSel
        {
            get { return null; }
            set
            {
                Debug.WriteLine(value);
            }
        }

        public IrpObjectViewModel Objects { get { return _objectViewModel; } }

        public IrpFileViewModel Files { get { return _fileViewModel; } }

        public ObservableCollection<ErrorViewModel> Errors { get; set; }

        private string _log;
        public string Log
        {
            get { return _log; }
            set { Set("Log", ref _log, value); }
        }        
        
        private int _progress;
        public int Progress
        {
            get { return _progress; }
            set { Set("Progress", ref _progress, value);}
        }

        private Boolean _haserrors = false;
        public Boolean HasErrors
        {
            get { return _haserrors; }
            set { Set("HasErrors", ref _haserrors, value);}
        }

        public string ProjectTitle
        {
            get { string ret = irpApp.ProjectFile; return ret == "" ? "IRPSIM" : ret; }
        }

        bool _isSimulationRunning = false;
        public bool IsSimulationRunning
        {
            get { return _isSimulationRunning; }
            set { Set("IsSimulationRunning", ref _isSimulationRunning, value);}
        }

        bool _canRunSimulation = false;
        public bool CanRunSimulation
        {
            get { return _canRunSimulation; }
            set
            {
                if (Set("CanRunSimulation", ref _canRunSimulation, value))
                    RunSimulationCommand.RaiseCanExecuteChanged();
            }
        }
        
        bool _canOpenProject = true;
        public bool CanOpenProject
        {
            get { return _canOpenProject; }
            set
            {
                if (Set("CanOpenProject", ref _canOpenProject, value))
                    ProjectOpenCommand.RaiseCanExecuteChanged();
            }
        }
 
        #endregion

        #region Commands

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

            HasErrors = false;
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
