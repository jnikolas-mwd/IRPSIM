using System.Windows;
using System.Windows.Controls;
using System.Diagnostics;
using IRPSIM.ViewModels;
using System.Windows.Input;

namespace IRPSIM.Views
{
    /// <summary>
    /// Description for FileView.
    /// </summary>
    public partial class FileView : UserControl
    {
        /// <summary>
        /// Initializes a new instance of the FileView class.
        /// </summary>
        public FileView()
        {
            InitializeComponent();
        }

        private void textbox_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            FileViewModel vm = DataContext as FileViewModel;
            if (vm == null)
                return;

            if (e.Key == Key.Back || e.Key == Key.Space || e.Key == Key.Return)
                vm.Modified = true;
        }

        private void textbox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            FileViewModel vm = DataContext as FileViewModel;
            if (vm == null)
                return;
             vm.Modified = true;
        }
    }
}