using System;
using System.Windows;

namespace WPFExtensions.AttachedBehaviours
{
	public abstract class FrameworkElementAttachedBehaviourBase<T>
		where T : FrameworkElement
	{
		public T Element { get; private set; }

		protected static object GetResourceKey<TAttachment>()
			where TAttachment : FrameworkElementAttachedBehaviourBase<T>
		{
			return typeof( TAttachment ).GUID;
		}

		public static TAttachment Get<TAttachment>( T element )
			where TAttachment : FrameworkElementAttachedBehaviourBase<T>
		{
			return Get<TAttachment>( element, false );
		}

		public static TAttachment Get<TAttachment>( T element, bool allowRegistration )
			where TAttachment : FrameworkElementAttachedBehaviourBase<T>
		{
			if ( element == null )
				throw new ArgumentNullException( "element" );

			var ext = element.Resources[GetResourceKey<TAttachment>()] as TAttachment;

			if ( allowRegistration && ext == null )
				ext = Attach<TAttachment>( element );

			return ext;
		}

		/// <summary>
		/// If attaches the behaviour to the FrameworkElement.
		/// If it were attached already, it returs with the attached behaviour instance.
		/// Otherwise it creates a new instance.
		/// </summary>
		/// <typeparam name="TAttachment"></typeparam>
		/// <param name="element"></param>
		/// <returns></returns>
		public static TAttachment Attach<TAttachment>( T element )
			where TAttachment : FrameworkElementAttachedBehaviourBase<T>
		{
			TAttachment attachment;
			if ( ( attachment = Get<TAttachment>( element ) ) != null )
				return attachment;

			attachment = (TAttachment)Activator.CreateInstance( typeof( TAttachment ), element );
			attachment.OnAttach();
			element.Resources.Add( GetResourceKey<TAttachment>(), attachment );
			return attachment;
		}

		public static void Detach<TAttachment>( T element )
			where TAttachment : FrameworkElementAttachedBehaviourBase<T>
		{
			var ext = Get<TAttachment>( element, false );
			if ( ext != null )
			{
				ext.OnDetach();
				element.Resources.Remove( GetResourceKey<TAttachment>() );
			}
		}

		protected FrameworkElementAttachedBehaviourBase( T element )
		{
			Element = element;
		}

		protected virtual void OnAttach()
		{ }

		protected virtual void OnDetach()
		{ }
	}
}
