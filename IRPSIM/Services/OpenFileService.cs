using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IRPSIM.Services
{
    public class OpenFileEventArgs : EventArgs
    {
        public OpenFileEventArgs(String path, int index, bool indexIsObjectIndex) : base() { Path = path; Index = index; IndexIsObjectIndex = indexIsObjectIndex; }
        public String Path {get; set;}   
        public int Index {get; set;}
        public bool IndexIsObjectIndex { get; set; }
    }

    public delegate void OpenFileEventHandler(object sender, OpenFileEventArgs e);

    public interface IOpenFileService
    {
        event OpenFileEventHandler OpenFile;

        void RequestOpenFile(String path, int index, bool indexIsObjectIndex); 
    }
    
    public class OpenFileService : IOpenFileService
    {
        public event OpenFileEventHandler OpenFile;

        public void RequestOpenFile(String path, int index, bool indexIsObjectIndex)
        {
            if (OpenFile != null) OpenFile(this, new OpenFileEventArgs(path, index, indexIsObjectIndex));            
        }
    }
}
