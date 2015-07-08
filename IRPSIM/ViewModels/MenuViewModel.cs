using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GalaSoft.MvvmLight;
using IRPSIM.Services;

namespace IRPSIM.ViewModels
{
    public class MenuViewModel : ViewModelBase
    {
        private IPersist _persist;

        public MenuViewModel(IPersist persist)
        {
            _persist = persist;
        }

        public bool SaveArchive
        {
            get { return _persist.SaveArchive; }
            set { _persist.SaveArchive = value; }
        }

        public bool SaveOutcomes
        {
            get { return _persist.SaveOutcomes; }
            set { _persist.SaveOutcomes = value; }
        }

        public bool SaveSummary
        {
            get { return _persist.SaveSummary; }
            set { _persist.SaveSummary = value; }
        }

        public bool DebugOn
        {
            get { return _persist.DebugOn; }
            set { _persist.DebugOn = value; }
        }
    }
}
