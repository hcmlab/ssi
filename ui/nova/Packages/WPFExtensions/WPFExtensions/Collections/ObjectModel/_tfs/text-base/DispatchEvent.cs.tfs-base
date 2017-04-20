using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Threading;
using System.Diagnostics;

namespace WPFExtensions.Collections.ObjectModel
{
	public class DispatchEvent
	{
		private List<DispatchHandler> handlers = new List<DispatchHandler>();

		public void Add( Delegate handler )
		{
			Add( handler, Dispatcher.CurrentDispatcher );
		}

		public void Add( Delegate handler, Dispatcher dispatcher )
		{
			Debug.WriteLine( "Adding handler" );
			handlers.Add( new DispatchHandler( handler, dispatcher ) );
		}

		public void Remove( Delegate handler )
		{
			Debug.WriteLine( "Removing handler" );
			var rmvHandler = ( from h in handlers
							   where h.DelegateEquals( handler )
							   select h ).FirstOrDefault();

			if ( rmvHandler != null )
			{
				this.handlers.Remove( rmvHandler );
				rmvHandler.Dispose();
			}
		}

		public void Fire( object sender, EventArgs args )
		{
			Debug.WriteLine( "Firing event" );
			//delete the disposable handlers
			var disposableHandlers = (from h in handlers
									 where h.IsDisposable
									 select h).ToArray();
			foreach ( DispatchHandler handler in disposableHandlers )
			{
				Debug.WriteLine( "Removing a handler..." );
				this.handlers.Remove( handler );
				handler.Dispose();
			}

			//call the remaining handlers
			foreach ( DispatchHandler handler in handlers )
			{
				Debug.WriteLine( "Invoking handler..." );
				handler.Invoke( sender, args );
			}
		}
	}
}
