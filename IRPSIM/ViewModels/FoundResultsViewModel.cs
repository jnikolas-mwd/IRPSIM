using System;
using System.Collections.Generic;
using GalaSoft.MvvmLight;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IRPSIM.Services;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Windows;
using GalaSoft.MvvmLight.Command;

namespace IRPSIM.ViewModels
{
    public class FoundResultsViewModel : NotifyViewModel
    {
        private IFindInFilesService _findInFilesService;
        private IOpenFileService _openFileService;
        private int _count = 0;

        public override string Name
        {
            get
            {
                return "Find Results";
            }
        }

        FoundInFileInfo _selectedItem;
        public FoundInFileInfo SelectedItem
        {
            get { return _selectedItem; }
            set { Set("SelectedItem", ref _selectedItem, value); }
        }

        public FoundResultsViewModel(IFindInFilesService findInFilesService, IOpenFileService openFileService)
        {
            _findInFilesService = findInFilesService;
            _openFileService = openFileService;
            FoundItems = new ObservableCollection<FoundInFileInfo>();

            _findInFilesService.StartSearch += (s, e) => { FoundItems.Clear(); _count = 0; Status = ""; };
            _findInFilesService.FoundInFile += (s, e) => { FoundItems.Add(e.Value); _count++; Status = String.Format("({0})", _count); };
        }

        public ObservableCollection<FoundInFileInfo> FoundItems { get; set; }

        RelayCommand _selectFoundItemCommand;
        public RelayCommand SelectFoundItemCommand
        {
            get
            {
                if (_selectFoundItemCommand == null)
                    _selectFoundItemCommand = new RelayCommand(() => {
                        Debug.WriteLine("Item Selected");
                        if (SelectedItem != null) 
                            _openFileService.RequestOpenFile(SelectedItem.Path,SelectedItem.Line,false); 
                    });
                return _selectFoundItemCommand;
            }
        }
    }
}
