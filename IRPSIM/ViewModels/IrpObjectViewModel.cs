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

    class IrpObjectContainer
    {
        public IrpObjectContainer(String name, IrpObjectCollection objects)
        {
            Name = name;
            Objects = objects;
        }

        public String Name { get; set; }

        public IrpObjectCollection Objects { get ; set; }
    }

    class IrpVariableContainer : IrpObjectContainer
    {
        public IrpVariableContainer(String name, IrpObjectCollection objects) : base(name, objects)
        {
        }
    }
    
    class IrpObjectViewModel : ViewModelBase
    {
        private CMWrappedIrpApplication _irpApp;
        private IrpObjectContainer _scenarios;
        private IrpObjectContainer _scripts;

        public IrpObjectViewModel(CMWrappedIrpApplication app)
            : base()
        {
            _irpApp = app;

            _containers.Add(new IrpVariableContainer("Supply", _irpApp.SupplyVariables));
            _containers.Add(new IrpVariableContainer("Demand", _irpApp.DemandVariables));
            _containers.Add(new IrpVariableContainer("Storage", _irpApp.StorageVariables));
            _containers.Add(new IrpVariableContainer("Cost", _irpApp.CostVariables));
            _containers.Add(new IrpVariableContainer("Variables", _irpApp.Variables));
            _containers.Add(new IrpObjectContainer("Definitions", _irpApp.Definitions));
            _containers.Add(_scenarios = new IrpObjectContainer("Scenarios", _irpApp.Scenarios));
            _containers.Add(_scripts = new IrpObjectContainer("Scripts", _irpApp.Scripts));
            _containers.Add(new IrpObjectContainer("Categories", _irpApp.Categories));
        }

        private object _selectedObject;
        public object SelectedObject
        {
            get { return _selectedObject; }
            set
            {
                if (Set("SelectedObject", ref _selectedObject, value))
                {
                    CMWrappedIrpObject o = _selectedObject as CMWrappedIrpObject;
                    if (o != null)
                        Debug.WriteLine(o.FileIndex);
                }
            }
        }

        private ObservableCollection<IrpObjectContainer> _containers = new ObservableCollection<IrpObjectContainer>();
        public ObservableCollection<IrpObjectContainer> Containers { get { return _containers; } }

         RelayCommand _selectIrpObjectCommand;
        public RelayCommand SelectIrpObjectCommand
        {
            get
            {
                if (_selectIrpObjectCommand == null)
                    _selectIrpObjectCommand = new RelayCommand(selectIrpObjectDelegate);
                return _selectIrpObjectCommand;
            }
        }

        RelayCommand _chooseIrpObjectCommand;
        public RelayCommand ChooseIrpObjectCommand
        {
            get
            {
                if (_chooseIrpObjectCommand == null)
                    _chooseIrpObjectCommand = new RelayCommand(chooseIrpObjectDelegate);
                return _chooseIrpObjectCommand;
            }
        }

        private void selectIrpObjectDelegate()
        {
            CMWrappedIrpObject irpObject = SelectedObject as CMWrappedIrpObject;
            if (irpObject != null && irpObject.FileId >= 0)
            {
                Debug.WriteLine("Opening file with index {0}", irpObject.FileIndex);
                OpenFileService.OpenFile(_irpApp.GetFilePath(irpObject.FileId), irpObject.FileIndex);
            }
                //irpObject.Selected = !irpObject.Selected;
        }

        private void chooseIrpObjectDelegate()
        {
            CMWrappedIrpObject irpObject = SelectedObject as CMWrappedIrpObject;
            if (irpObject == null)
                return;
            
            if (irpObject.Type == "CMScenario" || irpObject.Type == "CMScript")
            {
                Debug.WriteLine(irpObject.Name);
                bool current = irpObject.Chosen;
                IrpObjectContainer container = irpObject.Type == "CMScenario" ? _scenarios : _scripts;
                foreach (CMWrappedIrpObject o in container.Objects)
                    o.Chosen = false;
                string name = (current ? "" : irpObject.Name);

                if (irpObject.Type == "CMScenario")
                    _irpApp.UseScenario(name);
                else
                    _irpApp.UseScript(name);

                irpObject.Chosen = !current;
            }
        }
    }
}
