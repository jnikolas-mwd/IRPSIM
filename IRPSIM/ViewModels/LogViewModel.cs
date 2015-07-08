using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using IRPSIM.Services;
using System.Diagnostics;

namespace IRPSIM.ViewModels
{
    /// <summary>
    /// This class contains properties that a View can data bind to.
    /// <para>
    /// See http://www.galasoft.ch/mvvm
    /// </para>
    /// </summary>
    public class LogViewModel : NotifyViewModel
    {
         private ICoreApplicationService _coreService;
                               
        public LogViewModel(ICoreApplicationService coreService)
        {
            Debug.WriteLine(coreService);
            _coreService = coreService;
            _coreService.IrpLog += (s,e) => {_log += (e.Value + System.Environment.NewLine); RaisePropertyChanged("Log");};
            _coreService.IrpProjectClosed += (s,e) => Log="";
        }

        public override string Name
        {
            get
            {
                return "Log";
            }
        }

        private string _log;
        public string Log
        {
            get { return _log; }
            set { Set("Log", ref _log, value); }
        }

        RelayCommand _clearCommand;
        public RelayCommand ClearCommand
        {
            get
            {
                if (_clearCommand == null)
                    _clearCommand = new RelayCommand(() => Log="");
                return _clearCommand;
            }
        }
    }
}