using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;

namespace SSIXmlEditor.Controls
{
	public class EditorTabPage : TabPage, Document.IDocument
	{
		private ScintillaNet.Scintilla		m_Editor;
		private bool						m_bSaved;
		private string						m_Text;
		private Document.IDocumentHandler	m_DocHandler;
		private bool						m_bLoadTemplate;
		private System.IO.FileInfo			m_File;
        private Document.DocumentValidator  m_Validator;

        public Document.DocumentValidator Validator 
        { 
            private get { return m_Validator; }
            set { m_Validator = value; value.Editor = Editor; }
        }

        public System.IO.FileInfo File { get { return m_File; } private set { m_File = value; } }

		public Document.IDocumentHandler DocumentHandler 
		{
			set 
			{
				value.Initialize(m_Editor);
				m_DocHandler = value;
				
				if(m_bLoadTemplate)
					value.InsertDefaultTemplate(m_Editor);
			}
            get { return m_DocHandler; }
		}
		
		public ScintillaNet.Scintilla Editor { get; private set; }

        public String DocumentText { get { return Editor.Text; } }

		public EditorTabPage(string name)
		{
			Initialize();
			setEventHandlers();
			
			m_bLoadTemplate = true;
		
			m_Text = name;
			Text = name + "*";
			
			m_bSaved = false;  	
			m_File = null;		
		}
		
		public EditorTabPage(System.IO.FileInfo file)
		{
			Initialize();
			m_File = file;
			m_bLoadTemplate = false;
			
			var stream = System.IO.File.OpenText(file.FullName);
			
			var content = stream.ReadToEnd();
			stream.Close();
			
			m_Text = file.Name;
			Text = m_Text;
						
			m_bSaved = true;

			m_Editor.InsertText(content);
			setEventHandlers();
		}
		
		private void Initialize()
		{
			m_Editor = new ScintillaNet.Scintilla();
			m_Editor.Dock = DockStyle.Fill;
			m_Editor.Margins[0].Width = 20;
			m_Editor.Indentation.SmartIndentType = ScintillaNet.SmartIndent.Simple;
			Editor = m_Editor;													   			

			Controls.Add(m_Editor);
		}
		
		private void setEventHandlers()
		{
			m_Editor.PreviewKeyDown += new PreviewKeyDownEventHandler(m_Editor_PreviewKeyDown);
			m_Editor.TextChanged += m_Editor_TextChanged;
		}
		
		public void InsertText(string text)
		{
			m_DocHandler.InsertText(m_Editor, text);
		}

		void m_Editor_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
		{
			if(e.Control && e.KeyCode == Keys.S)
			{
				Save();
			}
			else if(e.Control && e.KeyCode == Keys.E)
			{
				var selection = m_Editor.Selection.Range;
				
				m_Editor.InsertText(selection.Start, "<!--");
				m_Editor.InsertText(selection.EndingLine.EndPosition, "-->");
			}
			else if(e.Control && e.KeyCode == Keys.R)
			{
				var selection = m_Editor.Selection.Range;
				if(selection.Length == 0)
				{
					selection.End = selection.EndingLine.EndPosition;
				}
				
				var text = selection.Text.Replace("<!--", "");
				text = text.Replace("-->", "");
				
				selection.Text = text;
			}
		}

		public void Execute()
		{
			Save();
			if(!m_bSaved)
				return;
				
			var process = new System.Diagnostics.Process();
			process.StartInfo.FileName = Properties.Settings.Default.PipelineApp;
            process.StartInfo.Arguments = string.Format("\"{0}\"", m_File.FullName);
            
            process.Start();
		}
		
		void m_Editor_TextChanged(object sender, EventArgs e)
		{
			if(m_bSaved)
			{
				m_bSaved = false;
				Text = Text + "*";
			}
		}
		
		public void Save()
		{
            if(m_File == null)
			{
				SaveAs();	
				return;
			}
		
			m_bSaved = true;
			Text = m_Text;	
			
			var stream = System.IO.File.CreateText(m_File.FullName);
			stream.Write(Editor.Text);
			stream.Close();
		}

		public void SaveAs()
		{
            string path;
			var dialog = new SaveFileDialog();
			dialog.Filter = "Pipeline (*.pipeline)|*.pipeline";
            dialog.FileName = Text.Replace("*", "");
            dialog.RestoreDirectory = true;
			if (dialog.ShowDialog() == DialogResult.OK)
			{
				path = dialog.FileName;
			}
			else
				return;
		
			
			
			m_File = new System.IO.FileInfo(path);
			m_Text = m_File.Name;
			m_bSaved = true;
			
			Text = m_Text;
			
			var stream = System.IO.File.CreateText(m_File.FullName);
			stream.Write(Editor.Text);
			stream.Close();
		}

        protected override void OnEnter(EventArgs e)
        {
            if (null != Validator)
                Validator.Start();

            base.OnEnter(e);
        }

        protected override void OnLeave(EventArgs e)
        {
            if (null != Validator)
                Validator.Stop();

            base.OnLeave(e);
        }
	}
}
