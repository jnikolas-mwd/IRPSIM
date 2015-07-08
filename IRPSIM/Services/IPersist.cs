using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IRPSIM.Services
{
    public interface IPersist
    {
        bool SaveArchive { get; set; }
        bool SaveOutcomes { get; set; }
        bool SaveSummary { get; set; }
        bool DebugOn { get; set; }
    }

    public class Persist : IPersist
    {
        public bool SaveArchive
        {
            get { return Properties.Settings.Default.SaveArchive; }
            set { 
                Properties.Settings.Default.SaveArchive = value; 
                Properties.Settings.Default.Save(); 
            }
        }

        public bool SaveOutcomes
        {
            get { return Properties.Settings.Default.SaveOutcomes; }
            set { Properties.Settings.Default.SaveOutcomes = value; Properties.Settings.Default.Save(); }
        }

        public bool SaveSummary
        {
            get { return Properties.Settings.Default.SaveSummary; }
            set { Properties.Settings.Default.SaveSummary = value; Properties.Settings.Default.Save(); }
        }

        public bool DebugOn
        {
            get { return Properties.Settings.Default.DebugOn; }
            set { Properties.Settings.Default.DebugOn = value; Properties.Settings.Default.Save(); }
        }
    
    }
}
