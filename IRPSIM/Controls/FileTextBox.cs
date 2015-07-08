using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Diagnostics;
using System.Windows;
using System.Windows.Data;
using System.IO;
using System.Reflection;
using IRPSIM.ViewModels;

namespace IRPSIM.Controls
{
    public class FileTextBox : TextBox
    {
        private bool _newDataContext = false;

        public FileTextBox()
            : base()
        {
            this.DataContextChanged += onDataContextChanged;
            this.TextChanged += onTextChanged;
        }

        void onDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            _newDataContext = true;
        }

        /// <summary>
        /// handles the first TextChanged event (after the file is loaded) by selecting the assigned line
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void onTextChanged(object sender, TextChangedEventArgs e)
        {
            Debug.WriteLine("Text Changed");
            if (_newDataContext && e.Changes.Count>0 && e.Changes.ElementAt(0).AddedLength>0)
            {
                selectLine(SelectedLine);
            }
            _newDataContext = false;
        }

        ~FileTextBox()
        {
            Debug.WriteLine("Destroying FileTextBox");
        }

        #region Property SelectedLine

        public int SelectedLine
        {
            get { return (int)GetValue(SelectedLineProperty); }
            set { SetValue(SelectedLineProperty, value); }
        }

        public static readonly DependencyProperty SelectedLineProperty =
            DependencyProperty.RegisterAttached(
            "SelectedLine",
            typeof(int),
            typeof(FileTextBox),
            new UIPropertyMetadata(0, OnSelectedLineChanged));

        static void OnSelectedLineChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            Debug.WriteLine("Selected Line Changed to {0}", (int)e.NewValue);
            FileTextBox textbox = depObj as FileTextBox;
            
            if (textbox == null)
                return;

            if (e.NewValue is int == false)
                return;

            if (textbox.Text.Length == 0)
                return;
 
            textbox.selectLine((int)e.NewValue);
        }
      
        private void selectLine(int line)
        {
            try
            {
                Debug.WriteLine("Selecting line {0}", line);
                StringReader sr = new StringReader(this.Text);
                  
                String text = "";
                int pos = 0;

                while (line > 0 && (text = sr.ReadLine()) != null)
                {
                    pos += text.Length + 1;
                    line--;
                }
                text = sr.ReadLine();
                this.Focus();
                this.Select(pos, text.Length);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }
        
        #endregion
        
        #region Property SelectedCharacterIndex

        public int SelectedCharacterIndex
        {
            get { return (int)GetValue(SelectedCharacterIndexProperty); }
            set { SetValue(SelectedCharacterIndexProperty, value); }
        }

        public static readonly DependencyProperty SelectedCharacterIndexProperty =
            DependencyProperty.RegisterAttached(
            "SelectedCharacterIndex",
            typeof(int),
            typeof(FileTextBox),
            new UIPropertyMetadata(0, OnSelectedCharacterIndexChanged));

        static void OnSelectedCharacterIndexChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            Debug.WriteLine("Selected Character Index Changed to {0}", (int)e.NewValue);
            FileTextBox textbox = depObj as FileTextBox;
            if (textbox == null)
                return;

            if (e.NewValue is int == false)
                return;

            if (textbox.Text.Length == 0)
                return;

            int index = (int)e.NewValue;

            textbox.Focus();

            if (index >= 0)
                textbox.Select(index, textbox.SearchText.Length);
            else
                textbox.Select(0, 0);
        }

        #endregion

        #region Property SavingFile

        public bool SavingFile
        {
            get { return (bool)GetValue(SavingFileProperty); }
            set { SetValue(SavingFileProperty, value); }
        }

        public static readonly DependencyProperty SavingFileProperty =
            DependencyProperty.RegisterAttached(
            "SavingFile",
            typeof(bool),
            typeof(FileTextBox),
            new UIPropertyMetadata(false, onSavingFileChanged));

        static void onSavingFileChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            Debug.WriteLine("Saving File Value changed");
            FileTextBox textbox = depObj as FileTextBox;
            if (textbox == null)
                return;

            if (e.NewValue is bool == false)
                return;

            if ((bool)e.NewValue == true) {
                BindingExpression bindingExpression = textbox.GetBindingExpression(TextProperty);

                if (bindingExpression != null)
                {
                    bindingExpression.UpdateSource();
                }
            }
        }

        #endregion

        #region Property SearchText

        public string SearchText
        {
            get { return (string)GetValue(SearchTextProperty); }
            set { SetValue(SearchTextProperty, value); }
        }

        public static readonly DependencyProperty SearchTextProperty =
            DependencyProperty.RegisterAttached(
            "SearchText",
            typeof(string),
            typeof(FileTextBox),
            new UIPropertyMetadata(""));

        #endregion
    }
}
