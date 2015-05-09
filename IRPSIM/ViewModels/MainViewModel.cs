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

namespace IRPSIM.ViewModels
{
    class MainViewModel : ObservableObject
    {
        private ObservableCollection<FileViewModel> openFiles = new ObservableCollection<FileViewModel>();
        private int openFileIndex;
        private CMWrappedIrpApplication irpApp = new CMWrappedIrpApplication();
        private Thread workerThread;
        private string fileName;

        public MainViewModel()
        {
            // irpApp = new CMWrappedIrpApplication();
            //  irpApp.OpenProject("C:\\Users\\Casey\\Documents\\IRPSIM Project Test\\Inputs\\Configuration\\IRP Resource Strategy.cfg");
        }

        public ICommand FileOpenCommand
        {
            get { return new DelegateCommand(openFileDelegate); }
        }

        public ICommand ProjectOpenCommand
        {
            get { return new DelegateCommand(openProjectDelegate); }
        }

        public ICommand RunSimulationCommand
        {
            get { return new DelegateCommand(runSimulationDelegate); }
        }

        public ObservableCollection<FileViewModel> OpenFiles
        {
            get { return openFiles; }
        }

        public int OpenFileIndex
        {
            get { return openFileIndex; }
            set
            {
                openFileIndex = value;
                RaisePropertyChangedEvent("OpenFileIndex");
            }
        }

        private void openFileDelegate()
        {
            OpenFileDialog dlg = new OpenFileDialog();

            if (dlg.ShowDialog() == true)
            {
                OpenFiles.Add(new FileViewModel(dlg.FileName));
                OpenFileIndex = OpenFiles.Count - 1;
            }
        }

        private void runSimulationDelegate()
        {
            irpApp.UseScenario("dsm_input");
            irpApp.UseScript("basemix-cra-ondemand");
            workerThread = new Thread(new ThreadStart(this.taskRunSimulation));
            workerThread.IsBackground = true;
            workerThread.Start();
        }

        private void openProjectDelegate()
        {
            OpenFileDialog dlg = new OpenFileDialog();

            if (dlg.ShowDialog() == true)
            {
                fileName = dlg.FileName;
                workerThread = new Thread(new ThreadStart(this.taskOpenProject));
                workerThread.IsBackground = true;
                workerThread.Start();
            }
        }

        #region Thread Tasks

        private void taskOpenProject()
        {
            irpApp.OpenProject(fileName);
            Debug.WriteLine("Project Loaded");
        }
       
        private void taskRunSimulation()
        {
            irpApp.RunSimulation();
            Debug.WriteLine("Simulation Completed");
        }
        #endregion
    }
}
