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

namespace IRPSIM.ViewModels
{
    class MainViewModel : ObservableObject
    {
        private FilesViewModel filesViewModel = new FilesViewModel();
         
        private CMNotifierDelegate del;

        private BackgroundWorker backgroundWorkerOpenProject = new BackgroundWorker();
        private BackgroundWorker backgroundWorkerRunSimulation = new BackgroundWorker();

        private Dictionary<int, string> _notifyTypes = new Dictionary<int,string>();
        private string _logMessage;
        private int _progress = 0;
                        
        private int test(int ntype, String msg, int data)
        {
            if (ntype == 3 || ntype == 4)
                this.Log += (msg + System.Environment.NewLine);
            if (ntype == 6)
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

        public ObservableCollection<CMWrappedVariable> Variables
        {
            get { return irpApp.GetVariables(); }
        }

        public ObservableCollection<CMLoadedFile> LoadedFiles
        {
            get { return irpApp.GetLoadedFiles(); }
        }

        public string Log
        {
            get { return _logMessage; }
            set
            {
                _logMessage = value;
                RaisePropertyChangedEvent("Log");
            }
        }

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
            get { string ret = irpApp.GetProjectName(); return ret == "" ? "IRPSIM" : ret; }
        }
 
        #endregion

 
        public ICommand ProjectOpenCommand
        {
            get { return new DelegateCommand(param => openProjectDelegate()); }
        }

        public ICommand RunSimulationCommand
        {
            get { return new DelegateCommand(parap => runSimulationDelegate()); }
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
