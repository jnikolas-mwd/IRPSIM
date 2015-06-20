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
    }
}
