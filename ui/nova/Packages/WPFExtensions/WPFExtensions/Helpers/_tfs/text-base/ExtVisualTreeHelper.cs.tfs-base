using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Diagnostics;

namespace WPFExtensions.Helpers
{
	public static class ExtVisualTreeHelper
	{
		/// <summary>
		/// Mini-Mole. Prints the VisualTree of the DependencyObject.
		/// </summary>
		public static void PrintVisualTree( this DependencyObject obj )
		{
			PrintVisualTree( obj, 0 );
		}

		private static void PrintVisualTree( DependencyObject obj, int depth )
		{
			// Print the object with preceding spaces that represent its depth
			Debug.WriteLine( new string( ' ', depth ) + obj );

			// Recursive call for each visual child
			for ( int i = 0, n = VisualTreeHelper.GetChildrenCount( obj ); i < n; i++ )
				PrintVisualTree( VisualTreeHelper.GetChild( obj, i ), depth + 1 );
		}
	}
}
