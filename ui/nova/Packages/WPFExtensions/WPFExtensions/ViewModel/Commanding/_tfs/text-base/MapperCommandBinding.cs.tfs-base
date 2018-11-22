using System;
using System.Windows.Input;
using System.Windows;

namespace WPFExtensions.ViewModel.Commanding
{
	public class MapperCommandBinding : CommandBinding
	{

		private ICommand _mappedToCommand;

		/// <summary>
		/// The command which will executed instead of the 'Command'.
		/// </summary>
		public ICommand MappedToCommand
		{
			get { return _mappedToCommand; }
			set
			{
				//mapped command cannot be null
				if ( value == null )
					throw new ArgumentException( "value" );

				if ( _mappedToCommand != null )
				{
					CanExecute -= OnCanExecute;
					Executed -= OnExecuted;
				}

				_mappedToCommand = value;

				CanExecute += OnCanExecute;
				Executed += OnExecuted;
			}
		}


		//
		//Mapper event handlers
		//
		protected void OnCanExecute( object sender, CanExecuteRoutedEventArgs e )
		{
			if ( MappedToCommand is RoutedCommand && e.Source is IInputElement )
				e.CanExecute = ( MappedToCommand as RoutedCommand ).CanExecute( e.Parameter, e.Source as IInputElement );
			else
				e.CanExecute = MappedToCommand.CanExecute( e.Parameter );
			e.Handled = true;
			e.ContinueRouting = false;
		}

		protected void OnExecuted( object sender, ExecutedRoutedEventArgs e )
		{
			if ( MappedToCommand is RoutedCommand && e.Source is IInputElement )
				( MappedToCommand as RoutedCommand ).Execute( e.Parameter, e.Source as IInputElement );
			else
				MappedToCommand.Execute( e.Parameter );
			e.Handled = true;
		}
	}
}
