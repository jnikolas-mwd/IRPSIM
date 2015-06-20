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

    public class OpenFileInfo
    {
        public OpenFileInfo(String path, int index)
        {
            Path = path;
            Index = index;
        }

        public String Path { get; set; }

        public int Index { get; set; }
    }

    public class OpenFileService
    {
        public static void Subscribe(object recipient, Action<OpenFileInfo> action)
        {
            Messenger.Default.Register(recipient, action);
        }

        public static void OpenFile(String path, int index=0)
        {
            OpenFileInfo msg = new OpenFileInfo(path, index);
            Messenger.Default.Send<OpenFileInfo>(msg);
        }
    }
}
