using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace IRPSIM.Services
{
    public interface IDialogService
    {
        bool Prompt(string message, string caption);
    }

    public class DialogService : IDialogService
    {
        public bool Prompt(string message, string caption)
        {
            if (MessageBox.Show(message, caption, MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                return true;
            return false;
        }
    }
}
