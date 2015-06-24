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
 
    public class OpenFileEventArgs : EventArgs
    {
        public OpenFileEventArgs(String path, int index) : base() { Path = path; Index = index; }
        public String Path {get; set;}   
        public int Index {get; set;}   
    }

    public delegate void OpenFileEventHandler(object sender, OpenFileEventArgs e);

    public interface IOpenFileService
    {
        event OpenFileEventHandler OpenFile;

        void RequestOpenFile(String path, int index);
    }
    
    public class OpenFileService : IOpenFileService
    {
        public event OpenFileEventHandler OpenFile;

        public void RequestOpenFile(String path, int index)
        {
            if (OpenFile!=null) OpenFile(this, new OpenFileEventArgs(path,index));            
        }
    }
}
