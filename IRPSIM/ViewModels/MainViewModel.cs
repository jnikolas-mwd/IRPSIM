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
    class ProgressToProgressVisibleConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            int n = int.Parse(value.ToString());
            return (n > 0 && n < 100 ? Visibility.Visible : Visibility.Hidden);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return DependencyProperty.UnsetValue;
        }
    }

    class SelectedToForegroundConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Debug.WriteLine(value);
            bool b = Boolean.Parse(value.ToString());
            return (b ? Brushes.Red : Brushes.Black);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return DependencyProperty.UnsetValue;
        }
    }

    class MainViewModel : ViewModelBase
    {
        //*** TODO: Abstract out as an IFilesViewModel interface injected as a dependency through MainViewModel constructor
        private FilesViewModel filesViewModel;
         
        private CMNotifierDelegate _notifierDelegate;

        private BackgroundWorker backgroundWorkerOpenProject = new BackgroundWorker();
        private BackgroundWorker backgroundWorkerRunSimulation = new BackgroundWorker();

        private List<ErrorViewModel> _errors = new List<ErrorViewModel>();

        private IGetFileName _getFileNameService;
                               
        private int _notify(int ntype, String msg, int data)
        {
            if (ntype == 1 || ntype == 2)
                _errors.Add(new ErrorViewModel(ntype, msg));
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
                    Debug.WriteLine(obj.FileId);
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
            get { string ret = irpApp.ProjectName; return ret == "" ? "IRPSIM" : ret; }
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

        public string IrpObjectType(object obj)
        {
            CMWrappedIrpObject wo = obj as CMWrappedIrpObject;

            if (wo == null)
                return "None";

            if (wo is CMWrappedVariable)
            {
                CMWrappedVariable v = wo as CMWrappedVariable;

                if (v.NType == NodeType.Demand)
                    return "Demand";
                if (v.NType == NodeType.Supply)
                    return "Supply";
                if (v.NType == NodeType.Storage)
                    return "Storage";
                if (v.NType == NodeType.Cost)
                    return "Cost";
                return (v.FileId >= 0 ? "UserVariable" : "SystemVariable");
            }

            return wo.Type.Substring(2);
        }


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

            if (path!=null)
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
            backgroundWorkerRunSimulation.RunWorkerAsync();
        }

        private void backgroundWorkerOpenProject_DoWork(object sender, DoWorkEventArgs e)
        {
            irpApp.OpenProject(e.Argument as string); 
        }

        private void backgroundWorkerOpenProject_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            RaisePropertyChanged("ProjectTitle");
            irpApp.AfterOpenProject();
            CanRunSimulation = true;
            CanOpenProject = true;
  
            foreach (ErrorViewModel m in _errors)
            {
                Errors.Add(m);
                HasErrors = true;
            }
         }

        private void backgroundWorkerRunSimulation_DoWork(object sender, DoWorkEventArgs e)
        {
            irpApp.UseScenario("test");
            irpApp.UseScript("basemix-cra-ondemand");
            irpApp.RunSimulation();
        }

        private void backgroundWorkerRunSimulation_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            foreach (ErrorViewModel m in _errors)
            {
                Errors.Add(m);
                HasErrors = true;
            }
        }

        private void openProject(string filename)
        {
            irpApp.OpenProject(filename); 
        }
    }
}
