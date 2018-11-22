using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace Samples
{
	public class TreeItemViewModel : INotifyPropertyChanged
	{
		public string Title { get; set; }
		public IList<TreeItemViewModel> Children { get; private set; }

		private bool isExpanded;
		private bool isSelected;

		public bool IsExpanded
		{
			get
			{
				return isExpanded;
			}
			set
			{
				if ( value == isExpanded )
					return;

				isExpanded = value;
				NotifyChanged( "IsExpanded" );
			}
		}

		public bool IsSelected
		{
			get
			{
				return isSelected;
			}
			set
			{
				if ( value == isSelected )
					return;

				isSelected = value;
				NotifyChanged( "IsSelected" );
			}
		}

		public TreeItemViewModel( int depth )
		{
			Children = new List<TreeItemViewModel>();
			if ( depth > 3 )
				return;

			for (int i = 0; i < 4; i++)
			{
				Children.Add(new TreeItemViewModel(depth + 1)
				             	{
				             		Title = "Children" + i
				             	});
			}
		}

		public void ExpandAll()
		{
			foreach (var child in Children)
			{
				IsExpanded = true;
				child.ExpandAll();
			}
		}

		#region INotifyPropertyChanged Members

		public event PropertyChangedEventHandler PropertyChanged;

		private void NotifyChanged( string propertyName )
		{
			if ( PropertyChanged != null )
				PropertyChanged( this, new PropertyChangedEventArgs( propertyName ) );
		}

		#endregion
	}
}
