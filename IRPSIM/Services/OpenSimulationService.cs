using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IrpsimEngineWrapper;

namespace IRPSIM.Services
{
    public class OpenSimulationEventArgs : EventArgs
    {
        public OpenSimulationEventArgs(CMWrappedSimulation sim) : base() { Simulation = sim; }

        public CMWrappedSimulation Simulation { get; set; }
    }

    public delegate void OpenSimulationEventHandler(object sender, OpenSimulationEventArgs e);

    public interface IOpenSimulationService
    {
        event OpenSimulationEventHandler OpenSimulation;

        void RequestOpenSimulation(CMWrappedSimulation sim);
    }

    public class OpenSimulationService : IOpenSimulationService
    {
        public event OpenSimulationEventHandler OpenSimulation;

        public void RequestOpenSimulation(CMWrappedSimulation sim)
        {
            if (OpenSimulation != null) OpenSimulation(this, new OpenSimulationEventArgs(sim));
        }
    }
}
