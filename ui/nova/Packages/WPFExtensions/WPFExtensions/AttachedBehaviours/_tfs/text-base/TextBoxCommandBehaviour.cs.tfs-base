using System;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows;

namespace WPFExtensions.AttachedBehaviours
{
	public enum TextBoxCommandTriggers
	{
		None,
		Enter,
		Focus,
		Unfocus,
		TextChange
	}

	/// <summary>
	/// Adds Command behaviour to the TextBox control.
	/// </summary>
	public class TextBoxCommandBehaviour : FrameworkElementAttachedBehaviourBase<TextBox>
	{
		#region Attached Properties

		public static ICommand GetCommand( DependencyObject obj )
		{
			return (ICommand)obj.GetValue( CommandProperty );
		}

		public static void SetCommand( DependencyObject obj, ICommand value )
		{
			obj.SetValue( CommandProperty, value );
		}

		// Using a DependencyProperty as the backing store for Command.  This enables animation, styling, binding, etc...
		public static readonly DependencyProperty CommandProperty =
			DependencyProperty.RegisterAttached( "Command", typeof( ICommand ), typeof( TextBoxCommandBehaviour ), new UIPropertyMetadata( null ) );



		public static object GetCommandParameter( DependencyObject obj )
		{
			return obj.GetValue( CommandParameterProperty );
		}

		public static void SetCommandParameter( DependencyObject obj, object value )
		{
			obj.SetValue( CommandParameterProperty, value );
		}

		// Using a DependencyProperty as the backing store for CommandParameter.  This enables animation, styling, binding, etc...
		public static readonly DependencyProperty CommandParameterProperty =
			DependencyProperty.RegisterAttached( "CommandParameter", typeof( object ), typeof( TextBoxCommandBehaviour ), new UIPropertyMetadata( null ) );



		public static IInputElement GetCommandTarget( DependencyObject obj )
		{
			return (IInputElement)obj.GetValue( CommandTargetProperty );
		}

		public static void SetCommandTarget( DependencyObject obj, IInputElement value )
		{
			obj.SetValue( CommandTargetProperty, value );
		}

		// Using a DependencyProperty as the backing store for CommandTarget.  This enables animation, styling, binding, etc...
		public static readonly DependencyProperty CommandTargetProperty =
			DependencyProperty.RegisterAttached( "CommandTarget", typeof( IInputElement ), typeof( TextBoxCommandBehaviour ), new UIPropertyMetadata( null ) );



		public static TextBoxCommandTriggers GetTrigger( DependencyObject obj )
		{
			return (TextBoxCommandTriggers)obj.GetValue( TriggerProperty );
		}

		public static void SetTrigger( DependencyObject obj, TextBoxCommandTriggers value )
		{
			obj.SetValue( TriggerProperty, value );
		}

		// Using a DependencyProperty as the backing store for Trigger.  This enables animation, styling, binding, etc...
		public static readonly DependencyProperty TriggerProperty =
			DependencyProperty.RegisterAttached( "Trigger", typeof( TextBoxCommandTriggers ), typeof( TextBoxCommandBehaviour ), new UIPropertyMetadata( TextBoxCommandTriggers.None, Trigger_PropertyChanged ) );

		private static void Trigger_PropertyChanged( DependencyObject d, DependencyPropertyChangedEventArgs e )
		{
			var textBox = d as TextBox;
			if ( textBox == null )
				return;

			var trigger = (TextBoxCommandTriggers)e.NewValue;


			var attachment = Get<TextBoxCommandBehaviour>( textBox, false );

			if ( attachment != null )
			{
				if ( trigger == TextBoxCommandTriggers.None )
					Detach<TextBoxCommandBehaviour>( textBox );
				else
				{
					attachment.Trigger = trigger;
				}
			}
			else if ( trigger != TextBoxCommandTriggers.None )
			{
				Attach<TextBoxCommandBehaviour>( textBox );
			}
		}

		#endregion

		private TextBoxCommandTriggers trigger = TextBoxCommandTriggers.None;
		protected TextBoxCommandTriggers Trigger
		{
			get
			{
				return trigger;
			}
			set
			{
				if ( value == trigger )
					return;

				OnDetach();
				trigger = value;
				OnAttach();
			}
		}

		public TextBoxCommandBehaviour( TextBox element )
			: base( element )
		{
			trigger = GetTrigger( element );
		}

		protected override void OnAttach()
		{
			switch ( trigger )
			{
				case TextBoxCommandTriggers.None:
					break;
				case TextBoxCommandTriggers.Enter:
					Element.KeyDown += TextBox_KeyDown;
					break;
				case TextBoxCommandTriggers.Focus:
					Element.GotFocus += TextBox_Focused;
					break;
				case TextBoxCommandTriggers.Unfocus:
					Element.LostFocus += TextBox_Unfocused;
					break;
				case TextBoxCommandTriggers.TextChange:
					Element.TextChanged += TextBox_TextChanged;
					break;
				default:
					throw new ArgumentOutOfRangeException();
			}
		}

		private void TextBox_TextChanged( object sender, TextChangedEventArgs e )
		{
			RunCommand();
		}

		private void TextBox_Unfocused( object sender, RoutedEventArgs e )
		{
			RunCommand();
		}

		private void TextBox_Focused( object sender, RoutedEventArgs e )
		{
			RunCommand();
		}

		private void TextBox_KeyDown( object sender, KeyEventArgs e )
		{
			if ( e.Key == Key.Enter )
				RunCommand();
		}

		private void RunCommand()
		{
			var command = GetCommand( Element );
			if ( command == null )
				return;

			var parameter = GetCommandParameter( Element );
			var target = GetCommandTarget( Element ) ?? Element;

			if ( command is RoutedCommand )
				( (RoutedCommand)command ).Execute( parameter, target );
			else
				command.Execute( target );
		}

		protected override void OnDetach()
		{
			switch ( trigger )
			{
				case TextBoxCommandTriggers.None:
					break;
				case TextBoxCommandTriggers.Enter:
					Element.KeyDown -= TextBox_KeyDown;
					break;
				case TextBoxCommandTriggers.Focus:
					Element.GotFocus -= TextBox_Focused;
					break;
				case TextBoxCommandTriggers.Unfocus:
					Element.LostFocus -= TextBox_Unfocused;
					break;
				case TextBoxCommandTriggers.TextChange:
					Element.TextChanged -= TextBox_TextChanged;
					break;
				default:
					throw new ArgumentOutOfRangeException();
			}
		}
	}
}
