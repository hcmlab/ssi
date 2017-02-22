using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.Highlighting;
using ICSharpCode.AvalonEdit.Folding;
using System.Windows.Forms;
using System.Xml;
using ICSharpCode.AvalonEdit.Document;
using System.Windows.Media;

namespace SSIXmlEditor
{
    public class SSIXmlEditorControl : TextEditor
    {
        private FoldingManager m_FoldingManager;
        private AbstractFoldingStrategy m_XmlFoldingStrategy;
        private Timer m_FoldingTimer;
        
        public SSIXmlEditorControl() 
        {
            Background = Brushes.White;
            ShowLineNumbers = true;
            SyntaxHighlighting = HighlightingManager.Instance.GetDefinition("XML");
            m_FoldingManager = FoldingManager.Install(this.TextArea);
            m_XmlFoldingStrategy = new XmlFoldingStrategy();
            m_XmlFoldingStrategy.UpdateFoldings(m_FoldingManager, Document);
            
            this.TextArea.IndentationStrategy = new Indentation.XmlIndentionStrategy();
			this.TextArea.MouseWheel += new System.Windows.Input.MouseWheelEventHandler(TextArea_MouseWheel);
            this.TextArea.FontFamily = new System.Windows.Media.FontFamily("Courier New");

            m_FoldingTimer = new Timer();
            m_FoldingTimer.Tick += new EventHandler(m_FoldingTimer_Tick);
            m_FoldingTimer.Interval = 1000;
            m_FoldingTimer.Enabled = true;
            m_FoldingTimer.Start();
        }

		void TextArea_MouseWheel(object sender, System.Windows.Input.MouseWheelEventArgs e)
		{
			if (Control.ModifierKeys == Keys.Control)
			{
				double value = e.Delta > 0 ? 1 : -1;
				
				this.FontSize = FontSize + value <= 1 ? 1 : FontSize + value;
			}
		}

        void m_FoldingTimer_Tick(object sender, EventArgs e)
        {
            m_XmlFoldingStrategy.UpdateFoldings(m_FoldingManager, Document);
        }
    }
}
