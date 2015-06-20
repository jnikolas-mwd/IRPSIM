using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using IrpsimEngineWrapper;
using IRPSIM.ViewModels.Services;
using System.Diagnostics;

namespace IRPSIM.ViewModels
{
    class IrpFileViewModel : ViewModelBase
    {        
        private CMWrappedIrpApplication _irpApp;

        public IrpFileViewModel(CMWrappedIrpApplication app)
            : base()
        {
            _irpApp = app;
        }

        public ObservableCollection<CMLoadedFile> LoadedFiles { get { return _irpApp.LoadedFiles; } }

        private CMLoadedFile _selectedItem;
        public CMLoadedFile SelectedItem
        {
            get { return _selectedItem; }
            set { Set("SelectedItem", ref _selectedItem, value); }
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
        
        private void selectFileDelegate()
        {
            Debug.WriteLine(SelectedItem);
            CMLoadedFile file = SelectedItem as CMLoadedFile;
            if (file != null)
                OpenFileService.OpenFile(file.Path, 0);
        }
    }
}
