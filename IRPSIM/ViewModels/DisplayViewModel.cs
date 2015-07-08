using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows;
using System.Windows.Input;
using IRPSIM.Services;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using System.Diagnostics;
using System.ComponentModel;
using IrpsimEngineWrapper;

namespace IRPSIM.ViewModels
{
    public class DisplayCollectionViewModel : ViewModelBase
    {
        private ObservableCollection<ClosableViewModel> _models = new ObservableCollection<ClosableViewModel>();
        private IChooseFileNameService _chooseFileNameService;
        private IOpenFileService _openFileService;
        private IOpenSimulationService _openSimulationService;
        private ICanCloseService _canCloseService;
        private ISaveFileService _saveFileService;

        public DisplayCollectionViewModel(IChooseFileNameService chooseFileNameService, 
            IOpenFileService openFileService,
            IOpenSimulationService openSimulationService,
            ICanCloseService canCloseService,
            ISaveFileService saveFileService)
        {
            _chooseFileNameService = chooseFileNameService;
            _openFileService = openFileService;
            _openSimulationService = openSimulationService;
            _canCloseService = canCloseService;
            _saveFileService = saveFileService;
            _openFileService.OpenFile += (s, e) => openFile(e.Path, e.Index, e.IndexIsObjectIndex);
            _openSimulationService.OpenSimulation += (s, e) => openSimulation(e.Simulation);
            _canCloseService.CloseMe += (s, e) => Models.Remove(s as ClosableViewModel);
        }

        public ObservableCollection<ClosableViewModel> Models
        {
            get { return _models; }
        }

        ClosableViewModel _selectedItem;
        public ClosableViewModel SelectedItem
        {
            get { return _selectedItem; }
            set { Set("SelectedItem", ref _selectedItem, value); }
        }

        RelayCommand _fileOpenCommand;
        public RelayCommand FileOpenCommand
        {
            get
            {
                if (_fileOpenCommand == null)
                    _fileOpenCommand = new RelayCommand(openFileDelegate);
                return _fileOpenCommand;
            }
        }

        private void openFileDelegate()
        {
            string path = _chooseFileNameService.ChooseFileName();

            if (path != null)
                openFile(path);
        }

        private void openFile(string path, int index = 0, bool indexIsObjectIndex=false)
        {
            if (path != null && path.Length > 0)
            {
                ClosableViewModel m = getModelHavingPath(path);
                if (m == null)
                {
                    m = new FileViewModel(_canCloseService, _saveFileService);
                    (m as FileViewModel).PathName = path;
                    Models.Add(m);
                }
                SelectedItem = m;
                if (indexIsObjectIndex)
                    (m as FileViewModel).SelectedObjectIndex = index;
                else
                    (m as FileViewModel).SelectedLine = index;
            }
        }

        private void openSimulation(CMWrappedSimulation sim)
        {
            SimulationViewModel m = new SimulationViewModel(sim, _canCloseService);
            Models.Add(m);
            SelectedItem = m;
        }

        RelayCommand _fileSaveCommand;
        public RelayCommand FileSaveCommand
        {
            get
            {
                if (_fileSaveCommand == null)
                    _fileSaveCommand = new RelayCommand(() => {if (SelectedItem!=null) SelectedItem.Save();});
                return _fileSaveCommand;
            }
        }
 
        RelayCommand _fileSaveAllCommand;
        public RelayCommand FileSaveAllCommand
        {
            get
            {
                if (_fileSaveAllCommand == null)
                    _fileSaveAllCommand = new RelayCommand(() => { foreach (ClosableViewModel model in Models) model.Save(); });
                return _fileSaveAllCommand;
            }
        }

        RelayCommand _fileSearchCommand;
        public RelayCommand FileSearchCommand
        {
            get
            {
                if (_fileSearchCommand == null)
                    _fileSearchCommand = new RelayCommand(() => { if (SelectedItem != null) SelectedItem.Searching = true; });
                return _fileSearchCommand;
            }
        }

        private ClosableViewModel getModelHavingPath(string path)
        {
            IEnumerable<ClosableViewModel> o = Models.Where(x => x.PathName == path);
            return o.Any() ? o.ElementAt(0) : null;
        }
    }
}
