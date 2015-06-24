using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IRPSIM.Services
{
    public interface ICanCloseService
    {
        event EventHandler CloseMe;

        void Close(object obj);
    }

    public class CanCloseService : ICanCloseService
    {
        public event EventHandler CloseMe;

        public void Close(object obj) 
        {
            if (CloseMe != null) CloseMe(obj, EventArgs.Empty);
        }
    }

}
