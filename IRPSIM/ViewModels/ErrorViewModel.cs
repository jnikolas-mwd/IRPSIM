using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows;
using GalaSoft.MvvmLight;
using IRPSIM.Services;
using IRPSIM.Models;
using System.Collections.ObjectModel;

namespace IRPSIM.ViewModels
{
    public class ErrorViewModel : NotifyViewModel
    {
        private ICoreApplicationService _coreService;
    
        private List<ErrorModel> _threadsaferrors = new List<ErrorModel>();
                               
        public ErrorViewModel(ICoreApplicationService coreService)
        {
            _coreService = coreService;

            Errors = new ObservableCollection<ErrorModel>();
            _coreService.IrpError += (s,e) => _threadsaferrors.Add(e.Value);
            _coreService.IrpProjectClosed += (s, e) => clearErrors();
            _coreService.IrpSimulationStarting += (s, e) => clearErrors();
            _coreService.IrpProjectLoaded += (s, e) => setErrors();
            _coreService.IrpSimulationCompleted += (s, e) => setErrors();
        }

        public override string Name
        {
            get
            {
                return "Errors";
            }
        }

        public ObservableCollection<ErrorModel> Errors { get; set; }

        public Boolean HasErrors
        {
            get { return Errors.Count>0; }
        }

        private void setErrors()
        {
            foreach (ErrorModel m in _threadsaferrors)
                Errors.Add(m);
            Status = Errors.Count == 0 ? "" : String.Format("({0})", Errors.Count); 
            RaisePropertyChanged("HasErrors");
        }

        private void clearErrors()
        {
            _threadsaferrors.Clear(); 
            Errors.Clear();
            Status = "";
            RaisePropertyChanged("HasErrors");
        }
    }
}
