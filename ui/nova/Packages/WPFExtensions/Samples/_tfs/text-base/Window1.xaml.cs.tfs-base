using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using System.ComponentModel;
using System.Collections.ObjectModel;
using WPFExtensions.Collections.ObjectModel;
using WPFExtensions.ViewModel.Commanding;
using System.Threading;
using System;

namespace Samples
{
	public class SampleDataContext : CommandSink
	{
		public static readonly RoutedCommand AddItems = new RoutedUICommand( "Add items", "AddItems", typeof( SampleDataContext ) );
		public static readonly RoutedCommand ExpandTreeView = new RoutedUICommand( "ExpandTreeView", "ExpandTreeView", typeof( SampleDataContext ) );

		public TreeViewDataSource TreeViewDS { get; private set; }
		private DispatchedObservableCollection<int> privateCollection = new DispatchedObservableCollection<int>();
		public ReadOnlyDispatchedObservableCollection<int> ObservableInts { get; private set; }

		public SampleDataContext()
		{
			TreeViewDS = new TreeViewDataSource();
			ObservableInts = new ReadOnlyDispatchedObservableCollection<int>( privateCollection );

			RegisterCommand(
				AddItems,
				param => true,
				param => AddIntItems() );

			RegisterCommand(
				ExpandTreeView,
				param => true,
				param => TreeViewDS.ExpandAll() );
		}

		private void AddIntItems()
		{
			var worker = new BackgroundWorker();
			worker.DoWork += ( s, e ) =>
			{
				privateCollection.Clear();
				for ( int i = 0; i < 10; i++ )
				{
					var list = new List<int>();
					var rnd = new Random( DateTime.Now.Millisecond );
					for ( int j = 0; j < 5; j++ )
					{
						list.Add( rnd.Next( 2000 ) );
					}
					privateCollection.AddRange( list );
					Thread.Sleep( 500 );
				}
			};
			worker.RunWorkerAsync();
		}
	}

	/// <summary>
	/// Interaction logic for Window1.xaml
	/// </summary>
	public partial class Window1
	{
		public Window1()
		{
			InitializeComponent();

			DataContext = new SampleDataContext();
		}

		public void ExecuteFirst( object sender, ExecutedRoutedEventArgs e )
		{
			MessageBox.Show( "Executing 1st command!" );
		}

		public void ExecuteSecond( object sender, ExecutedRoutedEventArgs e )
		{
			MessageBox.Show( "Executing 2nd command!" );
		}

		public void ExecuteThird( object sender, ExecutedRoutedEventArgs e )
		{
			MessageBox.Show( "Executing 3rd command!" );
		}

		private void TextBoxCommand_Execute( object sender, ExecutedRoutedEventArgs e )
		{
			MessageBox.Show( "TextBox Command Executed with parameter: " + e.Parameter );
		}

		private void btnSelectOne_Click( object sender, RoutedEventArgs e )
		{
			var ds = DataContext as TreeViewDataSource;
			ds.Items[2].IsExpanded = true;
			ds.Items[2].Children[2].IsExpanded = true;
			ds.Items[2].Children[2].Children[2].IsExpanded = true;
			ds.Items[2].Children[2].Children[2].Children[2].IsSelected = true;
		}
	}
}
