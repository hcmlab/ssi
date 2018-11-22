using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;

namespace WPFExtensions.Helpers
{
	public static class ExtLogicalTreeHelper
	{
		/// <summary>
		/// Finds the first ancestor of type <typeparamref name="T"/> in the LogicalTree
		/// starting from the <paramref name="obj"/> all way up.
		/// It checks the Parent and the TemplatedParent (if the <code>Parent == null</code>).
		/// </summary>
		/// <typeparam name="T">The type of the ancestor.</typeparam>
		/// <param name="obj">The object where we want to start.</param>
		/// <returns>The ancestor which have been found or <code>null</code> if there's 
		/// no ancestor with the type <typeparamref name="T"/>.</returns>
		public static T GetAncestorOfTypeExt<T>( this DependencyObject obj )
			where T : DependencyObject
		{
			T result = null;
			DependencyObject parent = obj;
			DependencyObject newParent = obj;
			while ( result == null && ( ( newParent = LogicalTreeHelper.GetParent( parent ) ) != null ||
				( parent is FrameworkElement && ( newParent = ( parent as FrameworkElement ).TemplatedParent ) != null ) ||
				( parent is FrameworkContentElement && ( newParent = ( parent as FrameworkContentElement ).TemplatedParent ) != null ) ) )
			{
				result = newParent as T;
				parent = newParent;
			}

			return result;
		}

		/// <summary>
		/// Finds the first ancestor of type <typeparamref name="T"/> in the LogicalTree
		/// starting from the <paramref name="obj"/> all way up, following the Parent property.
		/// </summary>
		/// <typeparam name="T">The type of the ancestor.</typeparam>
		/// <param name="obj">The object where we want to start.</param>
		/// <returns>The ancestor which have been found or <code>null</code> if there's 
		/// no ancestor with the type <typeparamref name="T"/>.</returns>
		public static T GetAncestorOfType<T>( this DependencyObject obj )
			where T : DependencyObject
		{
			//extended GetParent(), recursively goes up in the LogicalTree
			T result = null;
			DependencyObject parent = obj;
			while ( result == null && ( ( parent = LogicalTreeHelper.GetParent( parent ) ) != null ) )
				result = parent as T;

			return result;
		}

		/// <summary>
		/// Finds the first ancestor of type <typeparamref name="T"/> in the LogicalTree
		/// starting from the <paramref name="obj"/> all way up, following the 
		/// Parent/TemplatedParent property.
		/// </summary>
		/// <typeparam name="T">The type of the ancestor.</typeparam>
		/// <param name="obj">The object where we want to start.</param>
		/// <param name="followTemplatedParent">If <code>true</code> the search follows the 
		/// TemplatedParent property, if the Parent is null.</param>
		/// <returns>The ancestor which have been found or <code>null</code> if there's 
		/// no ancestor with the type <typeparamref name="T"/>.</returns>
		public static T GetAncestorOfType<T>( this DependencyObject obj, bool followTemplatedParent )
			where T : DependencyObject
		{
			return followTemplatedParent ? obj.GetAncestorOfTypeExt<T>() : obj.GetAncestorOfType<T>();
		}
	}
}
