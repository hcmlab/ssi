using System.Collections;
using System.Collections.Generic;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows;

namespace WPFExtensions.Helpers
{
	public static class TreeViewHelper
	{
		/// <summary>
		/// 
		/// </summary>
		/// <param name="treeView"></param>
		/// <param name="item"></param>
		/// <returns></returns>
		public static IEnumerable<TreeViewItem> GetTreeViewItemRouteForItem( this TreeView treeView, object item )
		{
			if ( item == null )
				return new TreeViewItem[0];

			IList<TreeViewItem> route = new List<TreeViewItem>();

			//find the TreeViewItem with DFS
			var selectedTreeViewItem = treeView.FindContainerHierarchically<TreeViewItem>( item );

			if ( selectedTreeViewItem != null )
			{
				for ( FrameworkElement element = selectedTreeViewItem; element != treeView && item != null; item = element.Parent as FrameworkElement )
					route.Insert( 0, element as TreeViewItem );
			}

			return route;
		}

		public static IEnumerable<object> GetItemRouteForItem( this TreeView treeView, object item )
		{
			//find the TreeViewItem with DFS

			if ( item == null )
				return new object[0];

			var selectedTreeViewItem = treeView.FindContainerHierarchically<TreeViewItem>( item );
			return GetItemRouteFromTreeViewItem( selectedTreeViewItem );
		}

		public static IEnumerable<object> GetItemRouteFromTreeViewItem( this TreeViewItem treeViewItem )
		{
			if ( treeViewItem == null )
				return new object[0];

			IList<object> route = new List<object>();
			ItemsControl parent = ItemsControl.ItemsControlFromItemContainer( treeViewItem );
			for ( ItemsControl container = treeViewItem;
				 !( container is TreeView ) && parent != null;
				 container = parent, parent = ItemsControl.ItemsControlFromItemContainer( container ) )
			{
				route.Insert( 0, ( parent.ItemContainerGenerator.ItemFromContainer( container ) ) );
			}
			return route;
		}

		public static TContainer FindContainerHierarchically<TContainer>( this ItemsControl itemsControl, object selectedItem )
			where TContainer : class
		{
			var container = itemsControl.ItemContainerGenerator.ContainerFromItem( selectedItem ) as TContainer;
			if ( container != null )
				return container;

			if ( itemsControl.ItemContainerGenerator.Status != GeneratorStatus.ContainersGenerated )
				return null;

			foreach ( var item in itemsControl.Items )
			{
				var childItemsControl = itemsControl.ItemContainerGenerator.ContainerFromItem( item ) as ItemsControl;
				if ( childItemsControl != null && ( container = childItemsControl.FindContainerHierarchically<TContainer>( selectedItem ) ) != null )
					//the selected container has been found
					return container;
			}
			return null;
		}
	}
}
