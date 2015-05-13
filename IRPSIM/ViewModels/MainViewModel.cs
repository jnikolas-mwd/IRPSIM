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

        /*
        private int test(int ntype, byte* msg, int data)
        {
           // Debug.WriteLine(msg);
            return 0;
        }
        */

        private CMWrappedIrpApplication irpApp = null;
 
        public MainViewModel()
        {
             irpApp = new CMWrappedIrpApplication(); 
            
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
            new Thread(() => { irpApp.RunSimulation(); Debug.WriteLine("Simulation Completed"); }).Start();     
        }

        private void openProjectDelegate()
        {
            OpenFileDialog dlg = new OpenFileDialog();

            if (dlg.ShowDialog() == true)
            {
                string filename = dlg.FileName;
                new Thread(() => { irpApp.OpenProject(filename); Debug.WriteLine("Loaded"); }).Start();              
            }
        }
    }
}
