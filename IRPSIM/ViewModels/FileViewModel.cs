using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using IRPSIM.Services;
using System.Diagnostics;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using System.IO;

namespace IRPSIM.ViewModels
{
    public class FileViewModel : ClosableViewModel
    {
        private ISaveFileService _saveFileService;

        public FileViewModel(ICanCloseService canCloseService, ISaveFileService saveFileService)
            : base(canCloseService)
        {
            _saveFileService = saveFileService;
        }
  
        ~FileViewModel()
        {
            Debug.WriteLine("Disposing FileViewModel");
        }

        private string _pathName;
        public override string PathName
        {
            get { return _pathName; }
            set
            {
                if (Set("PathName", ref _pathName, value))
                {
                    try
                    {
                        StreamReader streamReader = new StreamReader(value);
                        Contents = streamReader.ReadToEnd().Replace("\r","");
                        streamReader.Close();
                    }
                    catch (Exception ex)
                    {
                        Contents = ex.Message;
                    }
                }
            }
        }
        
        public override string Name 
        {
            get { return Path.GetFileName(PathName); }
            set { }
        }

        private string _contents;
        public string Contents
        {
            get { return _contents; }
            set { Set("Contents", ref _contents, value); }
        }      
        
        private int _selectedLine;
        public int SelectedLine
        {
            get { return _selectedLine; }
            set { Set("SelectedLine", ref _selectedLine, value); }
        }

        private int _selectedCharacterIndex;
        public int SelectedCharacterIndex
        {
            get { return _selectedCharacterIndex; }
            set { Set("SelectedCharacterIndex", ref _selectedCharacterIndex, value); }
        }

        private string _searchText;
        public string SearchText
        {
            get { return _searchText; }
            set { Set("SearchText", ref _searchText, value); }
        }

        public int SelectedObjectIndex
        {
            set { SelectedLine = getLineFromObjectIndex(value); }
        }

        public override void Save()
        {
            base.Save();
            StreamWriter streamWriter = new StreamWriter(PathName,false);
            streamWriter.Write(Contents);
            streamWriter.Close();
        }

        private string _oldSearchText;
        private void search()
        {
            if (_oldSearchText != SearchText)
            {
                _oldSearchText = SearchText;
                SelectedCharacterIndex = -1;
            }
            SelectedCharacterIndex = Contents.IndexOf(SearchText, SelectedCharacterIndex + 1, StringComparison.CurrentCultureIgnoreCase);
        }

        protected override bool closeIfModified()
        {
            SaveFileResponse response = _saveFileService.SaveFilePrompt(
                String.Format("Contents have been modified. Do you wish to save {0}?", this.Name),
                "Save File");

            if (response==SaveFileResponse.Save)
            {
                Save();
                return true;
            }

            if (response == SaveFileResponse.Close)
                return true;

            return false;
        }

        private int getLineFromObjectIndex(int index)
        {
            if (index == 0) return 0;

            String s = Contents;
            StringReader sr = new StringReader(Contents);

            String line;
            int lineno = 0;
            int startline = -1;
            while ((line = sr.ReadLine()) != null)
            {
                line = line.Trim();
                if (line.Length>0 && line[0] == '#')
                {
                    if (line.Substring(0, 4).ToUpper() == "#END")
                    {
                        if (--index == 0)
                        {
                            sr.Close();
                            return startline;
                        }
                        startline = -1;
                    }
                    else if (startline < 0)
                        startline = lineno;
                }
                lineno++;
            }
            sr.Close();
            return 0;
        }

        RelayCommand _modifiedCommand;
        public RelayCommand ModifiedCommand
        {
            get
            {
                if (_modifiedCommand == null)
                    _modifiedCommand = new RelayCommand(() => Modified = true);
                return _modifiedCommand;
            }
        }

        RelayCommand _searchCommand;
        public RelayCommand SearchCommand
        {
            get
            {
                if (_searchCommand == null)
                    _searchCommand = new RelayCommand(search);
                return _searchCommand;
            }
        }

        RelayCommand _startSearchCommand;
        public RelayCommand StartSearchCommand
        {
            get
            {
                if (_startSearchCommand == null)
                    _startSearchCommand = new RelayCommand(() => Searching = true);
                return _startSearchCommand;
            }
        }

        RelayCommand _stopSearchCommand;
        public RelayCommand StopSearchCommand
        {
            get
            {
                if (_stopSearchCommand == null)
                    _stopSearchCommand = new RelayCommand(()=>Searching=false);
                return _stopSearchCommand;
            }
        }
    }
}
