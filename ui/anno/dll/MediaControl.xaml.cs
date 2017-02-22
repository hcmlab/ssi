using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ssi
{
    /// <summary>
    /// Interaction logic for MediaControl.xaml
    /// </summary>
    public partial class MediaControl : UserControl
    {
        public MediaControl()
        {
            InitializeComponent();
        }

        public void clear()
        {
            this.audioGrid.ColumnDefinitions.Clear();
            this.audioGrid.Children.Clear();
            this.videoGrid.ColumnDefinitions.Clear();
            this.videoGrid.Children.Clear();
        }

        public void addMedia(IMedia media, bool is_video)
        {
            Grid grid = is_video ? videoGrid : audioGrid;

            if (is_video == true && grid.RowDefinitions.Count > 0)
            {
                // splitter
                ColumnDefinition split_column = new ColumnDefinition();
                split_column.Width = new GridLength(1, GridUnitType.Auto);
                grid.ColumnDefinitions.Add(split_column);
                GridSplitter splitter = new GridSplitter();
                splitter.ResizeDirection = GridResizeDirection.Columns;
                splitter.Width = 3;
                splitter.HorizontalAlignment = HorizontalAlignment.Stretch;
                splitter.VerticalAlignment = VerticalAlignment.Stretch;
                Grid.SetRowSpan(splitter, 1);
                Grid.SetRow(splitter, 0);
                Grid.SetColumn(splitter, grid.ColumnDefinitions.Count - 1);
                grid.Children.Add(splitter);
            }
            
            // video
            ColumnDefinition col = new ColumnDefinition();
            col.Width = new GridLength(1, GridUnitType.Star);
            grid.ColumnDefinitions.Add(col);

            MediaBox media_box = new MediaBox(media, is_video);
            Grid.SetRow(media_box, 0);
            Grid.SetColumn(media_box, grid.ColumnDefinitions.Count - 1);
            grid.Children.Add(media_box);            
        }
    }
}
