using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using IrpsimEngineWrapper;
using IRPSIM.Services;
using System.Diagnostics;

namespace IRPSIM.ViewModels
{
    public class LoadedFilesViewModel : ViewModelBase
    {        
        private ICoreApplicationService _coreService;
        private IOpenFileService _openFileService;
        private IFindInFilesService _findInFilesService;

        public LoadedFilesViewModel(ICoreApplicationService coreService, IOpenFileService openFileService, IFindInFilesService findInFilesService)
            : base()
        {
            _coreService = coreService;
            _openFileService = openFileService;
            _findInFilesService = findInFilesService;
        }

        public ObservableCollection<CMLoadedFile> LoadedFiles { get { return _coreService.LoadedFiles; } }

        private CMLoadedFile _selectedItem;
        public CMLoadedFile SelectedItem
        {
            get { return _selectedItem; }
            set { Set("SelectedItem", ref _selectedItem, value); }
        }

        private string _searchText="";
        public string SearchText
        {
            get { return _searchText; }
            set { Set("SearchText", ref _searchText, value); }
        }


        private RelayCommand _selectFileCommand;
        public RelayCommand SelectFileCommand
        {
            get
            {
                if (_selectFileCommand == null)
                    _selectFileCommand = new RelayCommand(selectFileDelegate);
                return _selectFileCommand;
            }
        }

        RelayCommand _findInFilesCommand;
        public RelayCommand FindInFilesCommand
        {
            get
            {
                if (_findInFilesCommand == null)
                    _findInFilesCommand = new RelayCommand(findInFileDelegate);
                return _findInFilesCommand;
            }
        }
        
        private void selectFileDelegate()
        {
            Debug.WriteLine(SelectedItem);
            CMLoadedFile file = SelectedItem as CMLoadedFile;
            if (file != null)
                _openFileService.RequestOpenFile(file.Path, 0, false);
        }

        private void findInFileDelegate()
        {
            if (SearchText.Length < 2)
                return;

            List<string> files = new List<string>();
            foreach (CMLoadedFile file in LoadedFiles)
                files.Add(file.Path);

            Debug.WriteLine("Searching");

            _findInFilesService.MultiFile = true;
            _findInFilesService.FindInFiles(SearchText, files);
        }
    }
}
