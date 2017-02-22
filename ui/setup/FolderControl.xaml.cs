using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Diagnostics;
using System.IO;
using System.Windows.Markup;
using System.Xml;

namespace ssi
{    
    public delegate void FolderControlChanged (string folder);

    /// <summary>
    /// Interaction logic for FolderView.xaml
    /// </summary>
    public partial class FolderControl : Window
    {       
        public event FolderControlChanged folderControlChanged;

        public FolderControl()
        {
            InitializeComponent();

            treeView.SelectedItemChanged += new RoutedPropertyChangedEventHandler<object>(treeView_SelectedItemChanged);
        }

        void treeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (e.NewValue != null)
            {
                string folder = ((TreeViewItem)e.NewValue).Tag.ToString();
                if (folderControlChanged != null)
                {
                    folderControlChanged(folder);
                }
            }
        }

        #region TreeView Control
        void TreeView_Loaded(object sender, RoutedEventArgs e)
        {
            /// Create main expanded node of TreeView
            treeView.Items.Add(TreeView_CreateComputerItem());
            /// Update open directories every 5 second
            DispatcherTimer timer = new DispatcherTimer(TimeSpan.FromSeconds(15),
                DispatcherPriority.Background, TreeView_Update, Dispatcher);
        }
        void TreeView_Update(object sender, EventArgs e)
        {
            Stopwatch s = new Stopwatch();
            s.Start();
            /// Update drives and folders in Computer
            /// create copy for detect what item was expanded
            TreeView oldTreeView = CloneUsingXaml(treeView) as TreeView;
            /// populate items from scratch
            treeView.Items.Clear();
            /// add computer expanded node with all drives
            treeView.Items.Add(TreeView_CreateComputerItem());
            TreeViewItem newComputerItem = treeView.Items[0] as TreeViewItem;
            TreeViewItem oldComputerItem = oldTreeView.Items[0] as TreeViewItem;
            /// Save old state of item
            newComputerItem.IsExpanded = oldComputerItem.IsExpanded;
            newComputerItem.IsSelected = oldComputerItem.IsSelected;
            /// check all drives for creating it's root folders
            foreach (TreeViewItem newDrive in (treeView.Items[0] as TreeViewItem).Items)
                if (newDrive.Items.Contains(null))
                    /// Find relative old item for newDrive
                    foreach (TreeViewItem oldDrive in oldComputerItem.Items)
                        if (oldDrive.Tag as string == newDrive.Tag as string)
                        {
                            newDrive.IsSelected = oldDrive.IsSelected;
                            if (oldDrive.IsExpanded)
                            {
                                newDrive.Items.Clear();
                                TreeView_AddDirectoryItems(oldDrive, newDrive);
                            }
                            break;
                        }
            s.Stop();
            Debug.WriteLine(String.Format("TreeView_Update finished with {0} ms.", s.ElapsedMilliseconds));
        }
        void TreeView_AddDirectoryItems(TreeViewItem oldItem, TreeViewItem newItem)
        {
            newItem.IsExpanded = oldItem.IsExpanded;
            newItem.IsSelected = oldItem.IsSelected;
            /// add folders in this drive
            string[] directories = Directory.GetDirectories(newItem.Tag as string);
            /// for each folder create TreeViewItem
            foreach (string directory in directories)
            {
                TreeViewItem treeViewItem = new TreeViewItem();
                treeViewItem.Header = new DirectoryInfo(directory).Name;
                treeViewItem.Tag = directory;
                try
                {
                    if (Directory.GetDirectories(directory).Length > 0)
                        /// find respective old folder
                        foreach (TreeViewItem oldDir in oldItem.Items)
                            if (oldDir.Tag as string == directory)
                            {
                                if (oldDir.IsExpanded)
                                {
                                    TreeView_AddDirectoryItems(oldDir, treeViewItem);
                                }
                                else
                                {
                                    treeViewItem.Items.Add(null);
                                }
                                break;
                            }
                }
                catch { }
                treeViewItem.Expanded += TreeViewItem_Expanded;
                newItem.Items.Add(treeViewItem);
            }
        }
        TreeViewItem TreeView_CreateComputerItem()
        {
            TreeViewItem computer = new TreeViewItem { Header = "Computer", IsExpanded = true };
            foreach (var drive in DriveInfo.GetDrives())
            {
                TreeViewItem driveItem = new TreeViewItem();
                if (drive.IsReady)
                {
                    driveItem.Header = String.Format("{0} ({1}:)", drive.VolumeLabel, drive.Name[0]);
                    if (Directory.GetDirectories(drive.Name).Length > 0)
                        driveItem.Items.Add(null);
                }
                else
                {
                    driveItem.Header = String.Format("{0} ({1}:)", drive.DriveType, drive.Name[0]);
                }
                driveItem.Tag = drive.Name;
                driveItem.Expanded += TreeViewItem_Expanded;
                computer.Items.Add(driveItem);
            }
            return computer;
        }
        void TreeViewItem_Expanded(object sender, RoutedEventArgs e)
        {
            TreeViewItem rootItem = (TreeViewItem)sender;

            if (rootItem.Items.Count == 1 && rootItem.Items[0] == null)
            {
                rootItem.Items.Clear();

                string[] dirs;
                try
                {
                    dirs = Directory.GetDirectories((string)rootItem.Tag);
                }
                catch
                {
                    return;
                }

                foreach (var dir in dirs)
                {
                    TreeViewItem subItem = new TreeViewItem();
                    subItem.Header = new DirectoryInfo(dir).Name;
                    subItem.Tag = dir;
                    try
                    {
                        if (Directory.GetDirectories(dir).Length > 0)
                            subItem.Items.Add(null);
                    }
                    catch { }
                    subItem.Expanded += TreeViewItem_Expanded;
                    rootItem.Items.Add(subItem);
                }
            }
        }
        #endregion

        object CloneUsingXaml(object obj)
        {
            string xaml = XamlWriter.Save(obj);
            return XamlReader.Load(new XmlTextReader(new StringReader(xaml)));
        }

        private void ButtonSelect_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            Close();
        }

        private void ButtonCancel_Click (object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }
    }
}
