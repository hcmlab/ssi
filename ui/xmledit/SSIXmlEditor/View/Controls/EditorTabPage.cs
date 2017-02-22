using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using ICSharpCode.AvalonEdit;
using System.Windows.Forms.Integration;
using ICSharpCode.AvalonEdit.Snippets;
using SSIXmlEditor.Service;
using System.Windows.Input;
using SSIXmlEditor.Controller;
using SSIXmlEditor.Snippets;
using System.Text.RegularExpressions;

namespace SSIXmlEditor.Controls
{
	public class EditorTabPage : TabPage, View.IDocumentView, Model.IInputHandler
	{
        private Controller.DocumentController m_Controller;
        private TextEditor m_Editor;
        private SnippetService m_SnippetCreator;
        private bool m_bUpdating;
        
        #region IDocumentView Members

        public new event EventHandler TextChanged;
        public event EventHandler TextEntering;
        public event EventHandler TextEntered;
        public event EventHandler CaretChanged;
		public event EventHandler RightMouseClick;


        private void PerformInvoke(TextEditor ctrl, Action act)
        {
            if (ctrl.Dispatcher.Thread != System.Threading.Thread.CurrentThread)
                ctrl.Dispatcher.Invoke(act);
            else
                act();
        }

        private T PerformInvoke<T>(TextEditor ctrl, Func<T> func)
        {
            if (ctrl.Dispatcher.Thread != System.Threading.Thread.CurrentThread)
                return (T)ctrl.Dispatcher.Invoke(func);
            else
                return func();
        }

        public string DocumentName
        {
            get { return this.Text; }
            set 
            { 
				this.Text = value; 
			}
        }

        public int CaretPosition
        {
            get 
            {
                return PerformInvoke<int>(m_Editor, () => m_Editor.CaretOffset);
            }
            set 
            {
                PerformInvoke(m_Editor, () => m_Editor.CaretOffset = value);
            }
        }

        public int CurrentLine
        {
            get 
            {
                return PerformInvoke<int>(m_Editor, () => m_Editor.Document.GetLineByOffset(m_Editor.CaretOffset).LineNumber);
            }
            set
            {
				PerformInvoke(m_Editor, () => m_Editor.CaretOffset = m_Editor.Document.GetOffset(value, 0));
            }
        }

        public int Lines
        {
            get
            {
                return PerformInvoke<int>(m_Editor, () => m_Editor.LineCount);
            }
        }

        public int GetOffsetOfLine(int line)
        {
            return PerformInvoke<int>(m_Editor, () => m_Editor.Document.GetLineByNumber(line).Offset);
        }

        public int GetEndOffsetOfLine(int line)
        {
            return PerformInvoke<int>(m_Editor, () => m_Editor.Document.GetLineByNumber(line).EndOffset);
        }

        public void InsertSnippet(string snippet, string[] args)
        {
            if (m_Snippets.ContainsSnippet(snippet))
            {
                var snip = m_SnippetCreator.CreateSnippet(m_Snippets.GetSnippet(snippet));

                PerformInvoke(m_Editor, () => 
                {
                    m_Editor.Focus();
                    if(!string.IsNullOrEmpty(this.GetCurrentLineText()))
					{
						m_Editor.TextArea.Caret.Offset = m_Editor.Document.GetLineByOffset(CaretPosition).Offset;
						this.Insert(m_Editor.TextArea.Caret.Offset, Environment.NewLine);
						m_Editor.TextArea.Caret.Line -= 1;
					}

					snip.Insert(m_Editor.TextArea);

					if (null != args)
					{
						foreach (string arg in args)
						{
							m_Editor.Document.Insert(m_Editor.CaretOffset, arg);
							SendKeys.SendWait("{TAB}");
						}

						SendKeys.SendWait("{ENTER}");
						m_Editor.SelectionLength = 0;
					}

					m_Editor.TextArea.IndentationStrategy.IndentLines(m_Editor.Document, 1, Lines);
                });	                 
            }
        }
        
        public void BeginUpdate()
        {
			PerformInvoke(m_Editor, () => m_Editor.Document.BeginUpdate());
        }
        
        public void EndUpdate()
        {
			PerformInvoke(m_Editor, () => m_Editor.Document.EndUpdate());
        }

        public void Insert(int offset, string text)
        {
            PerformInvoke(m_Editor, () =>
            {				
				m_Editor.Document.Insert(offset, text);
                m_Editor.TextArea.IndentationStrategy.IndentLines(m_Editor.Document, 1, Lines);
            });
        }

        public void Remove(int line)
        {
            PerformInvoke(m_Editor, () =>
            {
                var docLine = m_Editor.Document.GetLineByNumber(line);
                m_Editor.Document.Remove(docLine.Offset, docLine.Length);
            });
        }

        public void Replace(int offset, int len, string value)
        {
            PerformInvoke(m_Editor, () =>
            {
				int iCarret = m_Editor.CaretOffset;
				
                m_Editor.Document.Replace(offset, len, value);
				m_Editor.TextArea.IndentationStrategy.IndentLines(m_Editor.Document, 1, Lines);
				
				m_Editor.CaretOffset = iCarret;
            });
        }

        public string GetCurrentLineText()
        {
            return PerformInvoke<string>(m_Editor, () => {
                var line = m_Editor.Document.GetLineByOffset(m_Editor.CaretOffset);
                return m_Editor.Document.GetText(line.Offset, line.Length);    
            });            
        }

        public string GetTextByLine(int lineNumber)
        {
            return PerformInvoke<string>(m_Editor, () =>
            {
                var line = m_Editor.Document.GetLineByNumber(lineNumber);
                return m_Editor.Document.GetText(line.Offset, line.Length);
            });        
        }

        public string GetText()
        {
            return PerformInvoke<string>(m_Editor, () => m_Editor.Text);
        }

        public void CorrectLineEndings()
        {
            m_Editor.Text = Regex.Replace(m_Editor.Text, "(?<!\r)\n", "\r\n");              
        }

		public void CreateAndShowContextMenu(IEnumerable<KeyValuePair<string, Action>> entries)
		{
			var cm = new System.Windows.Controls.ContextMenu();
			
			foreach(KeyValuePair<string, Action> entry in entries)
			{
				var mi = new System.Windows.Controls.MenuItem();
				mi.Header = entry.Key;
				mi.Click += delegate { entry.Value(); };
				
				cm.Items.Add(mi);		  				
			}
			
			cm.IsOpen = true;
		}

        #endregion

        public EditorTabPage(Document.ADocument document)
        {
            if (null == document)
                throw new ArgumentNullException("document");

            m_bUpdating = false;
            m_Editor = new SSIXmlEditorControl();
            m_Editor.AllowDrop = true;
            m_Controller = new DocumentController(document, this);
            document.InputHandler = this;

			this.Dock = DockStyle.Fill;
            ElementHost host = new ElementHost();
            host.Dock = DockStyle.Fill;
            host.Child = m_Editor;

            Controls.Add(host);
			
            m_SnippetCreator = new SnippetService();
            m_Editor.TextChanged += delegate { if (null != TextChanged) TextChanged(this, EventArgs.Empty); };
            m_Editor.TextArea.TextEntered += delegate { if (null != TextEntered) TextEntered(this, EventArgs.Empty); };
            m_Editor.TextArea.Caret.PositionChanged += delegate { if (null != CaretChanged && !m_bUpdating) CaretChanged(this, EventArgs.Empty); };
			m_Editor.TextArea.PreviewKeyDown += m_Editor_PreviewKeyDown;

            m_Editor.DragEnter += new System.Windows.DragEventHandler(m_Editor_DragEnter);
            m_Editor.DragOver += new System.Windows.DragEventHandler(m_Editor_DragOver);
            m_Editor.MouseRightButtonDown += delegate(object sender, MouseButtonEventArgs e) 
            { 
				var pos = m_Editor.GetPositionFromPoint(e.GetPosition(m_Editor));
				
				if(pos.HasValue)
					m_Editor.TextArea.Caret.Position = pos.Value;
				
				if(null != RightMouseClick) 
					RightMouseClick(this, EventArgs.Empty); 
			};
        }
        
        void m_Editor_DragOver(object sender, System.Windows.DragEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine("Over");
            
            
        }

        void m_Editor_DragEnter(object sender, System.Windows.DragEventArgs e)
        {
            
            if (e.Data.GetDataPresent(typeof(MetaData)))
                e.Effects = System.Windows.DragDropEffects.Move;
            else
                e.Effects = System.Windows.DragDropEffects.None;
        }

        private SnippetManager m_Snippets =SnippetManager.Instance;
        
        void m_Editor_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            string[] line = GetTextByLine(CurrentLine).TrimStart().Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries);
            if (line.Count() == 0)
                return;
            
            string value = line[0];

            if (e.Key == System.Windows.Input.Key.Tab && m_Snippets.ContainsSnippet(value))
            {
                e.Handled = true;
                this.Remove(CurrentLine);

                InsertSnippet(value, null);
            }
        }

        void TextArea_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            
        }
    }
}
