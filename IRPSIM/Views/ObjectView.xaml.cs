using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using IRPSIM.ViewModels;
using System.Diagnostics;
using IrpsimEngineWrapper;

namespace IRPSIM.Views
{
    /// <summary>
    /// Interaction logic for ObjectView.xaml
    /// </summary>
    /// 
    public partial class ObjectView : UserControl
    {
        public ObjectView()
        {
            InitializeComponent();
        }

        private void TreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
             SelectedObjectHelper.Content = e.NewValue;
        }

        private void TreeView_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Space)
            {
                IrpObjectViewModel vm = DataContext as IrpObjectViewModel;
                if (vm != null)
                {
                    vm.ChooseIrpObjectCommand.Execute(null);
                }
            }
        }
    }
}
