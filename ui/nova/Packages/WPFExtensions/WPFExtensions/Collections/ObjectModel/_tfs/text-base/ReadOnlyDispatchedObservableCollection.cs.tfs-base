using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;

namespace WPFExtensions.Collections.ObjectModel
{
	public class ReadOnlyDispatchedObservableCollection<T> : ReadOnlyObservableCollection<T>, INotifyCollectionChanged, INotifyPropertyChanged
	{
		DispatchEvent collectionChanged = new DispatchEvent();
		DispatchEvent propertyChanged = new DispatchEvent();

		protected override event NotifyCollectionChangedEventHandler CollectionChanged
		{
			add { collectionChanged.Add( value ); }
			remove { collectionChanged.Remove( value ); }
		}
		protected override event PropertyChangedEventHandler PropertyChanged
		{
			add { propertyChanged.Add( value ); }
			remove { propertyChanged.Remove( value ); }
		}

		public ReadOnlyDispatchedObservableCollection( ObservableCollection<T> collection )
			: base( collection )
		{ }

		protected override void OnCollectionChanged( NotifyCollectionChangedEventArgs args )
		{
			Debug.WriteLine( "ReadOnlyDispatchedObservableCollection.OnCollectionChanged" );
			collectionChanged.Fire( this, args );
		}

		protected override void OnPropertyChanged( PropertyChangedEventArgs args )
		{
			Debug.WriteLine( "ReadOnlyDispatchedObservableCollection.OnPropertyChanged" );
			propertyChanged.Fire( this, args );
		}

		#region INotifyCollectionChanged Members

		event NotifyCollectionChangedEventHandler INotifyCollectionChanged.CollectionChanged
		{
			add { CollectionChanged += value; }
			remove { CollectionChanged -= value; }
		}

		#endregion

		#region INotifyPropertyChanged Members

		event PropertyChangedEventHandler INotifyPropertyChanged.PropertyChanged
		{
			add { PropertyChanged += value; }
			remove { PropertyChanged -= value; }
		}

		#endregion
	}
}
