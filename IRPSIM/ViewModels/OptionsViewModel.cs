using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GalaSoft.MvvmLight;
using IRPSIM.Services;
using IrpsimEngineWrapper;

namespace IRPSIM.ViewModels
{
    public class SimulationOption : ViewModelBase
    {
        public SimulationOption(string title, string optionName)
        {
            Name = optionName;
            Title = title;
        }

        public static ICoreApplicationService CoreService { get; set; }

        public string Name { get; set; }

        public string Title {get; set;}

        public string Value
        {
            get { return CoreService.GetOption(Name); }
            set { CoreService.SetOption(Name, value); }
        }
    }

    public class SimulationTextOption : SimulationOption
    {
        public SimulationTextOption(string title, string optionName) 
            : base(title, optionName)
        {
        }
    }

    public class SimulationSelectOption : SimulationOption
    {
        public SimulationSelectOption(string title, string optionName, string [] items)
            : base(title, optionName)
        {
            Items = items;
        }

        public string[] Items { get; set; }
    }

    public class OptionsViewModel : ViewModelBase
    {
        private ICoreApplicationService _coreService;

        public OptionsViewModel(ICoreApplicationService coreService)
            : base()
        {
            _coreService = coreService;
            SimulationOption.CoreService = coreService;
            Options = new ObservableCollection<SimulationOption>();
            _coreService.IrpProjectLoaded += onProjectLoaded;
        }

        void onProjectLoaded(object sender, BoolEventArgs e)
        {
            Options.Clear();
            Options.Add(new SimulationTextOption("Simulation Name", "simulationname"));
            Options.Add(new SimulationTextOption("Output Folder", "outputfolder"));
            Options.Add(new SimulationTextOption("Number of Trials", "numtrials"));
            Options.Add(new SimulationSelectOption("Precision", "precision", new string [] {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"}));
            Options.Add(new SimulationSelectOption("Cost Precision", "costprecision", new string[] { "0", "2" }));
            Options.Add(new SimulationTextOption("Simulation Begin", "simbegin"));
            Options.Add(new SimulationTextOption("Simulation End", "simend"));
            Options.Add(new SimulationSelectOption("Simulation Interval", "siminterval", new string [] {"annual", "monthly", "weekly", "daily"}));
            Options.Add(new SimulationTextOption("Trace Begin", "tracebegin"));
            Options.Add(new SimulationTextOption("Trace End", "traceend"));
            Options.Add(new SimulationTextOption("Trace Start", "tracestart"));
            Options.Add(new SimulationSelectOption("Trace Mode", "tracemode", new string [] {"static" , "dynamic"}));
            Options.Add(new SimulationSelectOption("Year End", "yearend", new string [] {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}));
            Options.Add(new SimulationTextOption("Random Seed", "randomseed"));
        }

        /*
        public IrpObjectCollection Options
        {
            get { return _coreService.Options; }
        }
        */

        public ObservableCollection<SimulationOption> Options { get; set; }


        private string _outputfolder;
        public string OutputFolder
        {
            get { return getOption("outputfolder"); }
            set { setOption("OutputFolder", "outputfolder", ref _outputfolder, value); }
        }

        private string _debugon;
        public string DebugOn
        {
            get { return getOption("debugon"); }
            set { setOption("DebugOn", "debugon", ref _debugon, value); }
        }
 
        private string _numtrials;
        public string NumTrials
        {
            get { return getOption("numtrials"); }
            set { setOption("NumTrials", "numtrials", ref _numtrials, value); }
        }

        private string _precision;
        public string Precision
        {
            get { return getOption("precision"); }
            set { setOption("Precision", "precision", ref _precision, value); }
        }

        private string _costprecision;
        public string CostPrecision
        {
            get { return getOption("costprecision"); }
            set { setOption("CostPrecision", "costprecision", ref _costprecision, value); }
        }

        private string _simbegin;
        public string SimulationBegin
        {
            get { return getOption("simbegin"); }
            set { setOption("SimulationBegin", "simbegin", ref _simbegin, value); }
        }

        private string _simend;
        public string SimulationEnd
        {
            get { return getOption("simend"); }
            set { setOption("SimulationEnd", "simend", ref _simend, value); }
        }

        private string _siminterval;
        public string SimulationInterval
        {
            get { return getOption("siminterval"); }
            set { setOption("SimulationInterval", "siminterval", ref _siminterval, value); }
        }

        private string _simulationname;
        public string SimulationName
        {
            get { return getOption("simulationname"); }
            set { setOption("SimulationName", "simulationname", ref _simulationname, value); }
        }

        private string _tracebegin;
        public string TraceBegin
        {
            get { return getOption("tracebegin"); }
            set { setOption("TraceBegin", "tracebegin", ref _tracebegin, value); }
        }

        private string _traceend;
        public string TraceEnd
        {
            get { return getOption("traceend"); }
            set { setOption("TraceEnd", "traceend", ref _traceend, value); }
        }

        private string _tracestart;
        public string TraceStart
        {
            get { return getOption("tracestart"); }
            set { setOption("TraceStart", "tracestart", ref _tracestart, value); }
        }

        private string _tracemode;
        public string TraceMode
        {
            get { return getOption("tracemode"); }
            set { setOption("TraceMode", "tracemode", ref _tracemode, value); }
        }
  
        private string _yearend;
        public string YearEnd
        {
            get { return getOption("yearend"); }
            set { setOption("YearEnd", "yearend", ref _yearend, value); }
        }

        private string _randomseed;
        public string RandomSeed
        {
            get { return getOption("randomseed"); }
            set { setOption("RandomSeed", "randomseed", ref _randomseed, value); }
        }
        
        private string getOption(string name) { return _coreService.GetOption(name); }
        
        private void setOption(string propertyName, string optionName, ref string field, string value)
        {
            _coreService.SetOption(optionName, value);
            Set(propertyName, ref field, value);
        }
    }
}

/*
autoarchive			 yes
autooutcomes         yes
autosummary          yes
outputfolder		 ..\Output
debugon              no
numtrials            83
precision            5
costprecision		 0
simbegin             2013
simend               2050
siminterval          annual
simulationname	     50-Year IRP
tracebegin           1922
traceend             2004
tracemode            dynamic
tracestart           1922
yearend              Dec
randomseed 99
*/