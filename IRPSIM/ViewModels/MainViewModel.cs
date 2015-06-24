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
using IRPSIM;
using IRPSIM.Services;
using IRPSIM.Models;
using System.Windows;
using System.Globalization;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using System.Windows.Media;

namespace IRPSIM.ViewModels
{
    public class MainViewModel : ViewModelBase
    {
        private ICoreApplicationService _coreService;
        private IChooseFileNameService _chooseFileNameService;

        public MainViewModel(ICoreApplicationService coreService, IChooseFileNameService chooseFileNameService)
        {
            _coreService = coreService;
            _chooseFileNameService = chooseFileNameService;

            _coreService.IrpSimulationProgress += (s,e) => Progress=e.Value;
            _coreService.IrpProjectLoaded += (s, e) => OnProjectLoaded(e.Value);
            _coreService.IrpSimulationCompleted += (s,e) => OnSimulationCompleted(e.Value);
        }

        void OnProjectLoaded(bool success)
        {
            RaisePropertyChanged("ProjectTitle");
            CanOpenProject = true;
  
            if (_coreService.HasErrors)
            {
                CanRunSimulation = false;
            }
            else
                CanRunSimulation = true;
        }

       void OnSimulationCompleted(bool success)
        {
            IsSimulationRunning = false;
        }
        
        #region Properties

 
        private int _progress;
        public int Progress
        {
            get { return _progress; }
            set { Set("Progress", ref _progress, value); }
        }

        public string ProjectTitle
        {
            get { string ret = _coreService.ProjectFile; return ret == "" ? "IRPSIM" : ret; }
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

            string path = _chooseFileNameService.ChooseFileName();

            if (path != null)
            {
                CanOpenProject = false;
                CanRunSimulation = false;
                _coreService.OpenProject(path);
            }
        }

        private void closeProjectDelegate()
        {
            _coreService.CloseProject();
            CanOpenProject = true;
            CanRunSimulation = false;
        }

        private void reloadProjectDelegate()
        {
            string path = _coreService.ProjectFile;
            _coreService.CloseProject();
            if (path != "")
            {
                CanOpenProject = false;
                CanRunSimulation = false;
                _coreService.OpenProject(path);
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
            IsSimulationRunning = true;
            _coreService.RunSimulation();
        }

        #endregion
        
    }
}
