using System.Collections.Generic;
using System.ComponentModel;

namespace Samples
{
	public class TreeViewDataSource : INotifyPropertyChanged
	{
		public IList<TreeItemViewModel> Items { get; private set; }
		private IEnumerable<object> selectedRoute;
		public IEnumerable<object> SelectedRoute
		{
			get { return selectedRoute; }
			set
			{
				if ( value == selectedRoute )
					return;

				selectedRoute = value;
				if ( PropertyChanged != null )
					PropertyChanged( this, new PropertyChangedEventArgs( "SelectedRoute" ) );
			}
		}

		public TreeViewDataSource()
		{
			Items = new List<TreeItemViewModel>();
			for ( int i = 0; i < 3; i++ )
			{
				Items.Add( new TreeItemViewModel( 0 )
				{
					Title = "Root" + i
				} );
			}
		}

		public void ExpandAll()
		{
			foreach (var item in Items)
			{
				item.ExpandAll();
			}
		}

		#region INotifyPropertyChanged Members

		public event PropertyChangedEventHandler PropertyChanged;

		#endregion
	}
}
