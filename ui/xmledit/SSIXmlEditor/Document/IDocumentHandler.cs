using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SSIXmlEditor.Document
{
	public class ItemSelectedEventArgs : EventArgs
	{
		public MetaData Selected { get; private set; }
	
		public ItemSelectedEventArgs(MetaData data)
		{
			Selected = data;		
		}
	}
	
	public class ChangedOption
	{
		public string Name { get; set; }
		public string Value { get; set; }
		public string OldValue { get; set; }
	}

	public interface IDocumentHandler
	{
		Snippets.SnippetManager SnippetManager { get; set; }
		Dictionary<string, XMLReader.MetaData> LookUpMetaData { set; }
	
		void Initialize(object sender);
		void InsertDefaultTemplate(object sender);

        void InsertText(object sender, string text);
        void InsertItem(object sender, MetaData obj);
        void OptionChanged(object sender, MetaData data, ChangedOption value);
		
		event EventHandler<ItemSelectedEventArgs> SSIItemSelected;
	}
}
