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

namespace IRPSIM.Views
{
    /// <summary>
    /// Interaction logic for ObjectView.xaml
    /// </summary>
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

        private void Supply_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Supply";
        }
        private void Demand_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Demand";
        }
        private void Storage_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Storage";
        }
        private void Cost_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Cost";
        }
        private void Variable_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "UserVariable";
        }
        private void Scenario_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Scenario";
        }
        private void Script_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Script";
        }
        private void Category_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Category";
        }
        private void Definition_Filter(object sender, FilterEventArgs e)
        {
            e.Accepted = ((MainViewModel)DataContext).IrpObjectType(e.Item) == "Definition";
        }
    }
}
