using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using System.Collections.ObjectModel;
using IRPSIM.Services;

namespace IRPSIM.ViewModels
{
    public class NotifyViewModel : ViewModelBase
    {
        public virtual string Name { get; set; }
 
        private string _status;
        public string Status
        {
            get { return _status; }
            set { Set("Status", ref _status, value); }
        }
    }

    public class NotifyCollectionViewModel : ViewModelBase
    {
        public NotifyCollectionViewModel()
        {
            Collection = new ObservableCollection<NotifyViewModel>();
            Collection.Add(ViewModelLocator.IrpLog);
            Collection.Add(ViewModelLocator.IrpErrors);
            Collection.Add(ViewModelLocator.FoundResults);
        }
        
        public ObservableCollection<NotifyViewModel> Collection { get; set; }
    }
}
