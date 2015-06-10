using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows;
using System.Windows.Input;
using IRPSIM.ViewModels.Services;
using GalaSoft.MvvmLight;

namespace IRPSIM.ViewModels
{
    class FileViewModel : ViewModelBase
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
                RaisePropertyChanged("FileContents");
            }
        }
    }

    class FilesViewModel : ViewModelBase
    {
        private ObservableCollection<FileViewModel> _files = new ObservableCollection<FileViewModel>();
        private int openFileIndex;
        private IGetFileName _getFileNameService;

        public FilesViewModel(IGetFileName fnservice)
        {
            _getFileNameService = fnservice;
        }
        
        
        public ObservableCollection<FileViewModel> Files
        {
            get { return _files; }
        }

        public int OpenFileIndex
        {
            get { return openFileIndex; }
            set
            {
                openFileIndex = value;
                RaisePropertyChanged("OpenFileIndex");
            }
        }

        public ICommand FileOpenCommand
        {
            get { return new DelegateCommand(param => openFileDelegate()); }
        }

        private void openFileDelegate()
        {
            string path = _getFileNameService.GetFileName();

            if (path!=null)
            {
                _files.Add(new FileViewModel(path));
                OpenFileIndex = _files.Count - 1;
            }
        }
            
    }
}
