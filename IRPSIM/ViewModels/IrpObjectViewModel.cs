using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using IrpsimEngineWrapper;
using IRPSIM.Services;
using System.Diagnostics;
using System.Windows.Data;
using System.ComponentModel;

namespace IRPSIM.ViewModels
{

    public class IrpObjectContainer
    {
        private CollectionViewSource _collectionViewSource;

        public IrpObjectContainer(String name, ObservableCollection<CMWrappedIrpObject> objects)
        {
            Name = name;
            Objects = objects;
            _collectionViewSource = new CollectionViewSource();
            _collectionViewSource.Source = objects;
            _collectionViewSource.SortDescriptions.Add(new SortDescription("Name", ListSortDirection.Ascending));
        }

        public String Name { get; set; }

        public ObservableCollection<CMWrappedIrpObject> Objects { get; set; }

        //public CollectionViewSource Collection { get; set; }
        public ICollectionView Collection { get { return _collectionViewSource.View; } }
    }

    public class IrpVariableContainer : IrpObjectContainer
    {
        public IrpVariableContainer(String name, ObservableCollection<CMWrappedIrpObject> objects)
            : base(name, objects)
        {
        }
    }
    
    public class IrpObjectViewModel : ViewModelBase
    {
        private ICoreApplicationService _coreService;
        private IOpenFileService _openFileService;
        private IrpObjectContainer _scenarios;
        private IrpObjectContainer _scripts;

        public IrpObjectViewModel(ICoreApplicationService coreService, IOpenFileService openFileService)
            : base()
        {
            _coreService = coreService;
            _openFileService = openFileService;

            _containers.Add(new IrpVariableContainer("Aggregates", _coreService.AggregateVariables));
            _containers.Add(new IrpVariableContainer("Supply", _coreService.SupplyVariables));
            _containers.Add(new IrpVariableContainer("Demand", _coreService.DemandVariables));
            _containers.Add(new IrpVariableContainer("Storage", _coreService.StorageVariables));
            _containers.Add(new IrpVariableContainer("Cost", _coreService.CostVariables));
            _containers.Add(new IrpVariableContainer("Variables", _coreService.Variables));
            _containers.Add(new IrpObjectContainer("Definitions", _coreService.Definitions));
            _containers.Add(_scenarios = new IrpObjectContainer("Scenarios", _coreService.Scenarios));
            _containers.Add(_scripts = new IrpObjectContainer("Scripts", _coreService.Scripts));
            _containers.Add(new IrpObjectContainer("Categories", _coreService.Categories));
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
                _openFileService.RequestOpenFile(_coreService.GetFilePath(irpObject.FileId), irpObject.FileIndex);
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
                    _coreService.UseScenario(name);
                else
                    _coreService.UseScript(name);

                irpObject.Chosen = !current;
            }
        }
    }
}
