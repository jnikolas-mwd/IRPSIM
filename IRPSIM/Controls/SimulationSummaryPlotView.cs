using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using OxyPlot;
using OxyPlot.Wpf;
using System.Diagnostics;
using IRPSIM.ViewModels;
using System.Windows.Media;

namespace IRPSIM.Controls
{
    class DisplaySeries
    {
        public LineSeries Mean { get; set; }
        public LineSeries UpperStandardError { get; set; }
        public LineSeries LowerStandardError { get; set; }
    };


    public class SimulationSummaryPlotView : PlotView
    {
        //ObservableCollection<LineSeries> _series = null;
        ObservableCollection<SimulationSummary> _series = null;
        Dictionary<string, DisplaySeries> _lineSeries = new Dictionary<string,DisplaySeries>();
        List<string> _displayedVariables = new List<string>();
        static OxyColor[] _colors = new OxyColor[11] { OxyColor.FromRgb(0x4E, 0x9A, 0x06),
                                               OxyColor.FromRgb(0xC8, 0x8D, 0x00),
                                               OxyColor.FromRgb(0xCC, 0x00, 0x00),
                                               OxyColor.FromRgb(0x20, 0x4A, 0x87),
                                               OxyColors.Red,
                                               OxyColors.Orange,
                                               OxyColors.Yellow,
                                               OxyColors.Green,
                                               OxyColors.Blue,
                                               OxyColors.Indigo,
                                               OxyColors.Violet};
        public SimulationSummaryPlotView()
            : base()
        {
        }

        ~SimulationSummaryPlotView()
        {
            if (_series!=null)
                _series.CollectionChanged -= series_CollectionChanged;
        }

        #region SimulationSummaries Property

        public ObservableCollection<SimulationSummary> SimulationSummaries
        {
            get { return (ObservableCollection<SimulationSummary>)GetValue(SimulationSummariesProperty); }
            set { SetValue(SimulationSummariesProperty, value); }
        }

        public static readonly DependencyProperty SimulationSummariesProperty =
            DependencyProperty.RegisterAttached(
            "SimulationSummaries",
             typeof(ObservableCollection<SimulationSummary>),
             typeof(SimulationSummaryPlotView),
             new UIPropertyMetadata(null, OnSimulationSummariesChanged));

        static void OnSimulationSummariesChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            SimulationSummaryPlotView plotView = depObj as SimulationSummaryPlotView;

            if (plotView == null)
                return;

            if (plotView._series != null)
                plotView._series.CollectionChanged -= plotView.series_CollectionChanged;

            if (e.NewValue == null)
                return;

            plotView._series = e.NewValue as ObservableCollection<SimulationSummary>;

            plotView._lineSeries.Clear();
            plotView._displayedVariables.Clear();

            foreach (SimulationSummary summary in plotView._series)
            {
                plotView._displayedVariables.Add(summary.Name);
                plotView._lineSeries[summary.Name] = plotView.createDisplaySeriesFromSummary(summary);
            }

            plotView._series.CollectionChanged += plotView.series_CollectionChanged;

            plotView.showSeries();
        }

        void series_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == System.Collections.Specialized.NotifyCollectionChangedAction.Add)
            {
                SimulationSummary summary = (SimulationSummary)e.NewItems[0];
                if (!_lineSeries.ContainsKey(summary.Name))
                {
                    _lineSeries[summary.Name] = createDisplaySeriesFromSummary(summary);
                }
                this._displayedVariables.Add(summary.Name);

                showSeries();
            }
            else
            {
                if (e.OldItems == null)
                {
                    this._displayedVariables.Clear();
                }
                else
                {
                    SimulationSummary summary = (SimulationSummary)e.OldItems[0];
                    this._displayedVariables.Remove(summary.Name);
                }
                showSeries();
            }
           
        }

        #endregion

        #region ShowStandardError Property

        public bool ShowStandardError
        {
            get { return (bool)GetValue(ShowStandardErrorProperty); }
            set { SetValue(ShowStandardErrorProperty, value); }
        }

        public static readonly DependencyProperty ShowStandardErrorProperty =
            DependencyProperty.RegisterAttached(
            "ShowStandardError",
             typeof(bool),
             typeof(SimulationSummaryPlotView),
             new UIPropertyMetadata(false, OnShowStandardErrorChanged));

        static void OnShowStandardErrorChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            SimulationSummaryPlotView plotView = depObj as SimulationSummaryPlotView;
            if (plotView == null || e.NewValue == null)
                return;

            plotView.showSeries();
        }

        #endregion

        DisplaySeries createDisplaySeriesFromSummary(SimulationSummary summary)
        {
            DisplaySeries d = new DisplaySeries();

            d.Mean = new LineSeries();
            d.Mean.Title = summary.Name;
            d.Mean.ItemsSource = summary.Mean;

            d.UpperStandardError = new LineSeries();
            d.UpperStandardError.ItemsSource = summary.UpperStandardError;
            d.UpperStandardError.LineStyle = LineStyle.Dash;

            d.LowerStandardError = new LineSeries();
            d.LowerStandardError.ItemsSource = summary.LowerStandardError;
            d.LowerStandardError.LineStyle = LineStyle.Dash;

            return d;
        }
   
        void showSeries()
        {
            this.Series.Clear();

            for (int i = 0; i < _displayedVariables.Count;i++ )
            {
                OxyColor c = _colors[i % 10];
                DisplaySeries s = _lineSeries[_displayedVariables[i]];
                s.Mean.Color = s.UpperStandardError.Color = s.LowerStandardError.Color = c.ToColor();
                this.Series.Add(s.Mean);
                if (this.ShowStandardError)
                {
                    this.Series.Add(s.UpperStandardError);
                    this.Series.Add(s.LowerStandardError);
                }
            }
            this.InvalidatePlot();
        }
    
    }
}
