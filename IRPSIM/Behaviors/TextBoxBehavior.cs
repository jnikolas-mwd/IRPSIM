using System;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Diagnostics;
using System.IO;

namespace IRPSIM.Behaviors
{
    class TextBoxBehavior
    {
        #region SelectedLine

        public static int GetSelectedLine(TextBox textBox)
        {
            return (int)textBox.GetValue(SelectedLineProperty);
        }

        public static void SetSelectedLine(TextBox textBox, int value)
        {
            textBox.SetValue(SelectedLineProperty, value);
        }

        public static readonly DependencyProperty SelectedLineProperty =
            DependencyProperty.RegisterAttached(
            "SelectedLine",
            typeof(int),
            typeof(TextBoxBehavior),
            new UIPropertyMetadata(0, OnSelectedLineChanged));

        static void OnSelectedLineChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            Debug.WriteLine("Selected Line Changed to {0}", (int)e.NewValue);
            TextBox textbox = depObj as TextBox;
            if (textbox == null)
                return;

            if (e.NewValue is int == false)
                return;
            
            if (textbox.Text.Length == 0)
                return;

            StringReader sr = new StringReader(textbox.Text);
            int index = (int)e.NewValue;
            String line="";
            int pos=0;

            while (index > 0 && (line=sr.ReadLine())!=null) {
                pos += line.Length + 1;
                index--;
            }
            line = sr.ReadLine();
            Debug.WriteLine("Index is {0}. Pos is {1}", index, pos);
            textbox.Focus();
            textbox.Select(pos, line.Length);
        }





        #endregion // SelectedLine

        #region SelectedCharacterIndex

        public static int GetSelectedCharacterIndex(TextBox textBox)
        {
            return (int)textBox.GetValue(SelectedCharacterIndexProperty);
        }

        public static void SetSelectedCharacterIndex(TextBox textBox, int value)
        {
            textBox.SetValue(SelectedCharacterIndexProperty, value);
        }

        public static readonly DependencyProperty SelectedCharacterIndexProperty =
            DependencyProperty.RegisterAttached(
            "SelectedCharacterIndex",
            typeof(int),
            typeof(TextBoxBehavior),
            new UIPropertyMetadata(0, OnSelectedCharacterIndexChanged));

        static void OnSelectedCharacterIndexChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            Debug.WriteLine("Selected Character Index Changed to {0}", (int)e.NewValue);
            TextBox textbox = depObj as TextBox;
            if (textbox == null)
                return;

            if (e.NewValue is int == false)
                return;

            if (textbox.Text.Length == 0)
                return;
            
            int index = (int)e.NewValue;

            textbox.Focus();

            if (index >= 0)
                textbox.Select(index, GetSearchText(textbox).Length);
            else
                textbox.Select(0, 0);
        }

        #endregion // SelectedCharacterIndex

        #region SearchText

        public static string GetSearchText(TextBox textBox)
        {
            return (string)textBox.GetValue(SearchTextProperty);
        }

        public static void SetSearchText(TextBox textBox, string value)
        {
            textBox.SetValue(SearchTextProperty, value);
        }

        public static readonly DependencyProperty SearchTextProperty =
            DependencyProperty.RegisterAttached(
            "SearchText",
            typeof(string),
            typeof(TextBoxBehavior),
            new UIPropertyMetadata("", OnSearchTextChanged));

        static void OnSearchTextChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            Debug.WriteLine(e.NewValue);
        }

        #endregion // SearchText
    }
}
