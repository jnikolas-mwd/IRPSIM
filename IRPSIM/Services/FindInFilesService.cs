using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;
using System.ComponentModel;

namespace IRPSIM.Services
{
    public class FoundInFileInfo
    {
        public FoundInFileInfo(String searchText, String foundText, String path, int line, int lineIndex)
            : base()
        {
            SearchText = searchText;
            FoundText = foundText;
            Path = path;
            Line = line;
            LineIndex = lineIndex;
        }

        public String SearchText { get; set; }
        public String FoundText { get; set; }
        public String Path { get; set; }
        public int Line { get; set; }
        public int LineIndex { get; set; }

        public override string ToString()
        {
            return string.Format("{0}, {1}, {2}, {3}, {4}", SearchText, FoundText, Path, Line, LineIndex);
        }
    }


    public class FoundInFileEventArgs : EventArgs
    {
        public FoundInFileEventArgs(FoundInFileInfo f)
            : base()
        {
            Value = f;
        }

        public FoundInFileInfo Value { get; set; }
    }

    public delegate void FoundInFileEventHandler(object sender, FoundInFileEventArgs e);

    public interface IFindInFilesService
    {
        event EventHandler StartSearch;
        event FoundInFileEventHandler FoundInFile;

        bool CaseSensitive { get; set; }

        bool MultiFile { get; set; }

        void FindInFiles(string searchText, List<string> files);
    }

    class FindInFilesService : IFindInFilesService
    {
        private StringComparison _comparisonType = StringComparison.CurrentCultureIgnoreCase;
        public event EventHandler StartSearch;
        public event FoundInFileEventHandler FoundInFile;
        private BackgroundWorker _backgroundWorker;

        private int _foundCount = 0;
        private int _lastReported = 0;
        private List<FoundInFileInfo> _foundItems = new List<FoundInFileInfo>();

        private bool _caseSensitive;
        public bool CaseSensitive
        {
            get { return _caseSensitive; }
            set
            {
                _caseSensitive = value;
                _comparisonType = (value == true ? StringComparison.CurrentCulture : StringComparison.CurrentCultureIgnoreCase);
            }
        }

        public bool MultiFile { get; set; }

        public FindInFilesService()
        {
            CaseSensitive = false;
            MultiFile = false;
            _backgroundWorker = new BackgroundWorker();
            _backgroundWorker.WorkerReportsProgress = true;
            _backgroundWorker.DoWork += doWork;
            _backgroundWorker.ProgressChanged += reportFoundItems;
            _backgroundWorker.RunWorkerCompleted += workCompleted;
        }

        void workCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            reportFoundItems(null, null);
            _foundItems.Clear();
        }
        
        public void FindInFiles(string searchText, List<string> files)
        {
            Debug.WriteLine(String.Format("Searching for {0}", searchText));
            if (StartSearch != null) StartSearch(this, EventArgs.Empty);
            _backgroundWorker.RunWorkerAsync(new object[] { searchText, files });
        }

        private void doWork(object sender, DoWorkEventArgs e)
        {
            object [] args = e.Argument as object[];
            string searchText = args[0] as string;
            List<string> files = args[1] as List<string>;
            _lastReported = 0;

            foreach (String path in files)
            {
                try
                {
                    StreamReader streamReader = new StreamReader(path);
                    String contents = streamReader.ReadToEnd().Replace("\r", "");
                    streamReader.Close();
                    searchFile(searchText, contents, path);
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                }
            }
        }

         void reportFoundItems(object sender, ProgressChangedEventArgs e)
         {
             if (FoundInFile == null)
                 return;

             int start = _lastReported;
             _lastReported = _foundItems.Count;

             try
             {
                 for (int i = start; i < _lastReported; i++)
                     FoundInFile(this, new FoundInFileEventArgs(_foundItems[i]));
             }
             catch (Exception ex)
             {
                 Debug.WriteLine(ex.Message);
             }
         }

        private void searchFile(string searchText, string contents, string path)
        {
            int line = 0;
            int index1 = 0;
            int index2 = 0;

            try
            {
                while (index2 >= 0)
                {
                    index2 = contents.IndexOf(searchText, index1, _comparisonType);
                    if (index2 >= 0)
                    {
                        line += getLinesFromIndexToIndex(contents, index1, index2);
                        if (FoundInFile != null)
                        {
                            int linestart = contents.LastIndexOf('\n', index2) + 1;
                            int lineend = contents.IndexOf('\n', index2);
                            if (lineend < 0) lineend = contents.Length - 1;
                            string foundText = contents.Substring(linestart, lineend - linestart).Trim();
                            FoundInFileInfo f = new FoundInFileInfo(searchText, foundText, path, line, index2 - linestart);
                            _foundItems.Add(f);
                            if (++_foundCount % 20 == 0)
                                _backgroundWorker.ReportProgress(_foundCount);
                        }
                        index1 = index2 + 1;
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }

        private int getLinesFromIndexToIndex(string text, int index1, int index2)
        {
            int lines = 0;
            int index = index1;

            try
            {
                while (index >= 0)
                {
                    index = text.IndexOf('\n', index);
                    if (index >= index2 || index < 0)
                        return lines;
                    lines++;
                    index++;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
            return lines;
        }
    }
}
