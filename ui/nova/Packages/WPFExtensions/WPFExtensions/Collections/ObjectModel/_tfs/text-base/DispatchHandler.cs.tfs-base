using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Windows.Threading;
using System.Collections.ObjectModel;
using System.Threading;
using System.Collections;
using System.Diagnostics;
using ThreadState = System.Threading.ThreadState;

namespace WPFExtensions.Collections.ObjectModel
{
	internal class DispatchHandler : IDisposable
	{
		private MethodInfo handlerInfo;
		private WeakReference targetRef;
		private WeakReference dispatcherRef;

		public object Target
		{
			get { return targetRef.IsAlive ? targetRef.Target : null; }
		}

		public Dispatcher Dispatcher
		{
			get { return dispatcherRef.IsAlive ? dispatcherRef.Target as Dispatcher : null; }
		}

		public DispatchHandler( Delegate handler, Dispatcher dispatcher )
		{
			//make the WeakReferences to the target and the dispatcher
			this.handlerInfo = handler.Method;
			this.targetRef = new WeakReference( handler.Target );
			this.dispatcherRef = new WeakReference( dispatcher );
		}

		public bool IsDisposable
		{
			get
			{
				object target = this.Target;
				Dispatcher dispatcher = this.Dispatcher;

				return (
					target == null
					|| dispatcher == null
					|| ( target is DispatcherObject
							&& ( ( dispatcher.Thread.ThreadState &
									( ThreadState.Aborted
										| ThreadState.AbortRequested
										| ThreadState.StopRequested
										| ThreadState.Stopped ) ) != 0 ) )
				);
			}
		}

		public void Invoke( object sender, params object[] args )
		{
			object target = this.Target;
			Dispatcher dispatcher = this.Dispatcher;

			if ( this.IsDisposable )
				return;

			var paramList = new ArrayList();
			paramList.Add( sender );
			paramList.AddRange( args );

			if ( dispatcher != null && dispatcher.Thread.IsAlive )
			{
				Debug.WriteLine( "Calling handler via dispatcher sync mode" );
				dispatcher.Invoke(
					new EventHandler( ( s, e ) =>
					{
						this.handlerInfo.Invoke( target, new object[] { sender, e } );
					} ), DispatcherPriority.Send, paramList.ToArray() );
			}
			else if ( target is DispatcherObject )
			{
				Debug.WriteLine( "Calling handler via dispatcher async mode" );
				dispatcher.BeginInvoke(
					new EventHandler( ( s, e ) =>
					{
						this.handlerInfo.Invoke( target, new object[] { sender, e } );
					} ), DispatcherPriority.Send, paramList.ToArray() );
			}
			else
			{
				Debug.WriteLine("Calling handler directly");

				//call the handler directly
				this.handlerInfo.Invoke( target, paramList.ToArray() );
			}
		}

		public bool DelegateEquals( Delegate other )
		{
			object target = this.Target;
			return ( target != null
						&& object.ReferenceEquals( target, other.Target )
						&& this.handlerInfo.Name == other.Method.Name );
		}

		#region IDisposable Members

		public void Dispose()
		{
			//nothing to do, there's only WeakReferences
		}

		#endregion
	}
}
