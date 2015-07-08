/*
  In App.xaml:
  <Application.Resources>
      <vm:ViewModelLocator xmlns:vm="clr-namespace:IRPSIM"
                           x:Key="Locator" />
  </Application.Resources>
  
  In the View:
  DataContext="{Binding Source={StaticResource Locator}, Path=ViewModelName}"

  You can also use Blend to do all this with the tool's support.
  See http://www.galasoft.ch/mvvm
*/

using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Ioc;
using Microsoft.Practices.ServiceLocation;
using IRPSIM.Services;

namespace IRPSIM.ViewModels
{
    /// <summary>
    /// This class contains static references to all the view models in the
    /// application and provides an entry point for the bindings.
    /// </summary>
    public class ViewModelLocator
    {
        /// <summary>
        /// Initializes a new instance of the ViewModelLocator class.
        /// </summary>
        static ViewModelLocator()
        {
            ServiceLocator.SetLocatorProvider(() => SimpleIoc.Default);

            SimpleIoc.Default.Register<IChooseFileNameService, ChooseFileNameService>();
            SimpleIoc.Default.Register<IOpenFileService, OpenFileService>();
            SimpleIoc.Default.Register<IOpenSimulationService, OpenSimulationService>();
            SimpleIoc.Default.Register<ICoreApplicationService, CoreApplicationService>();
            SimpleIoc.Default.Register<IFindInFilesService, FindInFilesService>();
            SimpleIoc.Default.Register<ICanCloseService, CanCloseService>();
            SimpleIoc.Default.Register<IDialogService, DialogService>();
            SimpleIoc.Default.Register<ISaveFileService, SaveFileService>();
            SimpleIoc.Default.Register<IPersist, Persist>();
 
            ////if (ViewModelBase.IsInDesignModeStatic)
            ////{
            ////    // Create design time view services and models
            ////    SimpleIoc.Default.Register<IDataService, DesignDataService>();
            ////}
            ////else
            ////{
            ////    // Create run time view services and models
            ////    SimpleIoc.Default.Register<IDataService, DataService>();
            ////}

            SimpleIoc.Default.Register<MainViewModel>();
            SimpleIoc.Default.Register<DisplayCollectionViewModel>();
            SimpleIoc.Default.Register<LoadedFilesViewModel>();
            SimpleIoc.Default.Register<IrpObjectViewModel>();
            SimpleIoc.Default.Register<OptionsViewModel>();
            SimpleIoc.Default.Register<LogViewModel>();
            SimpleIoc.Default.Register<ErrorViewModel>();
            SimpleIoc.Default.Register<FoundResultsViewModel>();
            SimpleIoc.Default.Register<NotifyCollectionViewModel>();
            SimpleIoc.Default.Register<MenuViewModel>();
        }

        public static MainViewModel Main
        {
            get
            {
                return ServiceLocator.Current.GetInstance<MainViewModel>();
            }
        }

        public static DisplayCollectionViewModel DisplayCollection
        {
            get
            {
                return ServiceLocator.Current.GetInstance<DisplayCollectionViewModel>();
            }
        }

        public static IrpObjectViewModel IrpObjectCollection
        {
            get
            {
                return ServiceLocator.Current.GetInstance<IrpObjectViewModel>();
            }
        }

        public static OptionsViewModel Options
        {
            get
            {
                return ServiceLocator.Current.GetInstance<OptionsViewModel>();
            }
        }

        public static LoadedFilesViewModel IrpLoadedFileCollection
        {
            get
            {
                return ServiceLocator.Current.GetInstance<LoadedFilesViewModel>();
            }
        }

        public static LogViewModel IrpLog
        {
            get
            {
                return ServiceLocator.Current.GetInstance<LogViewModel>();
            }
        }

        public static ErrorViewModel IrpErrors
        {
            get
            {
                return ServiceLocator.Current.GetInstance<ErrorViewModel>();
            }
        }

        public static FoundResultsViewModel FoundResults
        {
            get
            {
                return ServiceLocator.Current.GetInstance<FoundResultsViewModel>();
            }
        }

        public static NotifyCollectionViewModel Notify
        {
            get
            {
                return ServiceLocator.Current.GetInstance<NotifyCollectionViewModel>();
            }
        }

        public static MenuViewModel Menu
        {
            get
            {
                return ServiceLocator.Current.GetInstance<MenuViewModel>();
            }
        }

        public static void Cleanup()
        {
            // TODO Clear the ViewModels
        }
    }
}