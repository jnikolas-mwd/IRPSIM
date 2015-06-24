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

namespace IRPSIM.ViewModels
{
    public class FoundResultsViewModel : NotifyViewModel
    {
        private IFindInFilesService _findInFilesService;
        private int _count = 0;

        public override string Name
        {
            get
            {
                return "Find Results";
            }
        }

        public FoundResultsViewModel(IFindInFilesService findInFilesService)
        {
            _findInFilesService = findInFilesService;
            FoundItems = new ObservableCollection<FoundInFileInfo>();

            _findInFilesService.StartSearch += (s, e) => { FoundItems.Clear(); _count = 0; Status = ""; };
            _findInFilesService.FoundInFile += (s, e) => { FoundItems.Add(e.Value); _count++; Status = String.Format("({0})", _count); };
        }

        public ObservableCollection<FoundInFileInfo> FoundItems { get; set; }
    }
}
