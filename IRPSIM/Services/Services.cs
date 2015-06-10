using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Diagnostics;
using System.Windows.Input;
using Microsoft.Win32;

namespace IRPSIM.ViewModels.Services
{
    public interface IGetFileName
    {
        string GetFileName();
    }

    public class OpenFileDialogService : IGetFileName
    {
        public string GetFileName()
        {
            OpenFileDialog dlg = new OpenFileDialog();

            if (dlg.ShowDialog() == true)
            {
                return dlg.FileName;
            }
            return null;
        }
    }
}
