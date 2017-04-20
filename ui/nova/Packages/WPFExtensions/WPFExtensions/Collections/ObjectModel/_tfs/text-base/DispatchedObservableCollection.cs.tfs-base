using System;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Collections.Generic;

namespace WPFExtensions.Collections.ObjectModel
{
	public class DispatchedObservableCollection<T> : ObservableCollection<T>
	{
		DispatchEvent collectionChanged = new DispatchEvent();
		DispatchEvent propertyChanged = new DispatchEvent();
		HashSet<string> changedPropertyList = new HashSet<string>();

		protected bool delayNotification;
		protected bool DelayNotification
		{
			get { return delayNotification; }
			set
			{
				if ( value == delayNotification )
					return;

				delayNotification = value;
				if ( !delayNotification )
				{
					OnCollectionChanged( new NotifyCollectionChangedEventArgs( NotifyCollectionChangedAction.Reset ) );
					BurstPropertyChangeNotification();
				}
			}
		}

		public override event NotifyCollectionChangedEventHandler CollectionChanged
		{
			add { collectionChanged.Add( value ); }
			remove { collectionChanged.Remove( value ); }
		}

		protected override event PropertyChangedEventHandler PropertyChanged
		{
			add { propertyChanged.Add( value ); }
			remove { propertyChanged.Remove( value ); }
		}

		protected override void OnCollectionChanged( NotifyCollectionChangedEventArgs e )
		{
			Debug.WriteLine( "DispatchedObservableCollection.OnCollectionChanged" );
			if ( DelayNotification )
				return;

			collectionChanged.Fire( this, e );
		}

		protected override void OnPropertyChanged( PropertyChangedEventArgs e )
		{
			Debug.WriteLine( "DispatchedObservableCollection.OnPropertyChanged" );
			changedPropertyList.Add( e.PropertyName );
			if ( DelayNotification )
				return;

			BurstPropertyChangeNotification();
		}

		private void BurstPropertyChangeNotification()
		{
			foreach ( var propertyName in changedPropertyList )
				propertyChanged.Fire( this, new PropertyChangedEventArgs( propertyName ) );

			changedPropertyList.Clear();
		}

		public void AddRange( IEnumerable<T> collection )
		{
			InsertRange( this.Count, collection );
		}

		public void InsertRange( int index, IEnumerable<T> collection )
		{
			if ( collection == null )
				throw new ArgumentNullException( "collection" );
			if ( index > this.Count )
				throw new ArgumentOutOfRangeException( "index" );

			DelayNotification = true;
			int i = 0;
			foreach ( var item in collection )
			{
				this.InsertItem( index + i, item );
				i++;
			}
			DelayNotification = false;
		}

		public void RemoveRange( int index, int count )
		{
			if ( ( index < 0 ) || ( count < 0 ) )
				throw new ArgumentOutOfRangeException( index < 0 ? "index" : "count" );
			if ( this.Count - index < count )
				throw new ArgumentOutOfRangeException( "count", "Not enough element in the collection" );

			DelayNotification = true;
			var removedItems = new List<T>();
			for ( int i = 0; i < count; i++ )
			{
				removedItems.Add( this.Items[index] );
				this.RemoveAt( index );
			}
			DelayNotification = false;
		}

		public void Remove( IEnumerable<T> collection )
		{
			if ( collection == null )
				throw new ArgumentNullException( "collection" );
			DelayNotification = true;
			foreach ( var item in collection )
				Remove( item );
			DelayNotification = false;
		}
	}
}
