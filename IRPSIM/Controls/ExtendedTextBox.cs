using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Diagnostics;

namespace IRPSIM.Controls
{
    public class ExtendedTextBox : TextBox
    {
        public ExtendedTextBox() : base()
        {
            Debug.WriteLine("Instantiating Extended TextBox");
        }

        ~ExtendedTextBox()
        {
            Debug.WriteLine("Destroying Extended TextBox");

        }
    }
}
