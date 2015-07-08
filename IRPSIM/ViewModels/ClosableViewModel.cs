using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using IRPSIM.Services;
using System.Diagnostics;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using System.IO;

namespace IRPSIM.ViewModels
{
    public class ClosableViewModel : ViewModelBase
    {
        private ICanCloseService _canCloseService;

        public ClosableViewModel(ICanCloseService canCloseService)
        {
            _canCloseService = canCloseService;
        }

        ~ClosableViewModel()
        {
            Debug.WriteLine("Disposing Closable View Model");
        }

        RelayCommand _closeMeCommand;
        public RelayCommand CloseMeCommand
        {
            get
            {
                if (_closeMeCommand == null)
                    _closeMeCommand = new RelayCommand(() => { if (!Modified || closeIfModified()) _canCloseService.Close(this); });
                return _closeMeCommand;
            }
        }

        private bool _searching=false;
        public bool Searching
        {
            get { return _searching; }
            set { Set("Searching", ref _searching, value); }
        }

        bool _saving = false;
        public bool Saving
        {
            get { return _saving; }
            set
            {
                Set("Saving", ref _saving, value);
                Modified = !value;
            }
        }

        bool _modified = false;
        public bool Modified
        {
            get { return _modified; }
            set { Set("Modified", ref _modified, value); }
        }
 
        protected virtual bool closeIfModified()
        {
            return true;
        }

        public virtual string PathName { get; set; }

        public virtual string Name { get; set; }

        public virtual void Save() { Saving = true; ; }
    }
}
 
