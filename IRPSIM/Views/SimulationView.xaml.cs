using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using IRPSIM.ViewModels;
using IrpsimEngineWrapper;

namespace IRPSIM.Views
{
    /// <summary>
    /// Description for SimulationView.
    /// </summary>
    public partial class SimulationView : UserControl
    {
        /// <summary>
        /// Initializes a new instance of the SimulationView class.
        /// </summary>
        public SimulationView()
        {
            InitializeComponent();
        }

        private void ListBox_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == Key.Space)
            {
                SimulationViewModel vm = DataContext as SimulationViewModel;
                if (vm != null)
                {
                    vm.ToggleSelectedVariable.Execute(null);
                }
            }
        }
    }
}