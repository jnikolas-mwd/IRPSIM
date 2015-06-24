using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IRPSIM.Models
{
    public class ErrorModel
    {
       public ErrorModel(int severity, string msg)
        {
            _severity = severity;
            _message = msg;
        }

        private int _severity;

        public int Severity
        {
            get { return _severity; }
        }
       
        public string SeverityString
        {
            get { return _severity == 1 ? "ERROR" : "WARNING"; }
        }

        private string _message;
        public string Message
        {
            get { return _message; }
        }

        public override string ToString()
        {
            return SeverityString + ": " + Message;
        }    
    }
}
