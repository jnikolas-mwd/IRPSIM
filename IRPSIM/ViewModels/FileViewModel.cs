using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows;

namespace IRPSIM.ViewModels
{
    class FileViewModel : ObservableObject
    {
        private string filePath, fileContents;

        public FileViewModel(string path)
        {
            this.filePath = path;

            try
            {
                StreamReader streamReader = new StreamReader(this.filePath);
                FileContents = streamReader.ReadToEnd();
                streamReader.Close();
            }
            catch (Exception ex)
            {
                FileContents = ex.Message;
            }
        }

        /*
        public FileViewModel(string path, string contents)
        {
            this.filePath = path;
            this.fileContents = contents;
        }
        */

        public string FilePath { get { return filePath; } }

        public string FileName { get { return Path.GetFileName(filePath); } }

        public string FileContents
        {
            get { return fileContents; }
            set
            {
                fileContents = value;
                RaisePropertyChangedEvent("FileContents");
            }
        }
    }
}
