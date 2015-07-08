using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace IRPSIM.Services
{
    public enum SaveFileResponse { Save, Close, Cancel };

    public interface ISaveFileService
    {
        SaveFileResponse SaveFilePrompt(string message, string caption);
    }

    public class SaveFileService : ISaveFileService
    {
        public SaveFileResponse SaveFilePrompt(string message, string caption)
        {
            MessageBoxResult result = MessageBox.Show(message, caption, MessageBoxButton.YesNoCancel);
            if (result == MessageBoxResult.Yes)
                return SaveFileResponse.Save;
            else if (result == MessageBoxResult.No)
                return SaveFileResponse.Close;
            return SaveFileResponse.Cancel;
        }
    }
}
