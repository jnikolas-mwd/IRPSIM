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
using GalaSoft.MvvmLight.Messaging;
using IrpsimEngineWrapper;

namespace IRPSIM.Services
{
    public interface IChooseFileNameService
    {
        string ChooseFileName();
    }

    public class ChooseFileNameService : IChooseFileNameService
    {
        public string ChooseFileName()
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
