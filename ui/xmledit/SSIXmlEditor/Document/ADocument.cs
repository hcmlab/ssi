using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using SSIXmlEditor.App;

namespace SSIXmlEditor.Document
{
    public class DocumentStateChangedEventArg : EventArgs
    {
        public bool Modified { get; private set; }

        public DocumentStateChangedEventArg(bool bModified)
        {
            Modified = bModified;
        }
    }

    public class ADocument
    {
        private static int s_Documents = 0;

        public event EventHandler<DocumentStateChangedEventArg> StateChanged;

        private string m_FileName;
        private Model.IInputHandler m_InputHandler = null;

        public int ID { get; private set; }
        public SingleInstanceApp Application { get; private set; }
        public FileInfo File { get; private set; }
        public String FileName 
        { 
            get { return null == File ? m_FileName : File.Name; } 
        }

        private bool m_Changed;
        public bool Changed
        {
            get { return m_Changed; }
            set
            {
                if (m_Changed != value)
                    FireStateChangedEvent(value);
                
                m_Changed = value;
            }
        }

        public Model.IInputHandler InputHandler 
        {
            get { return m_InputHandler; }
            set
            {
                if (null == value)
                    throw new ArgumentNullException("InputHandler");

                if (m_InputHandler != null)
					m_InputHandler.TextChanged -= m_InputHandler_TextChanged;
                

                m_InputHandler = value;
                m_InputHandler.TextChanged += m_InputHandler_TextChanged;
                Text = m_Text;
            }
        }

        void m_InputHandler_TextChanged(object sender, EventArgs e)
        {
            Changed = true;
        }

        private string m_Text = String.Empty;

        public String Text 
        { 
            get { return InputHandler.GetText(); } 
            set 
            {
                if(m_InputHandler != null)
                    InputHandler.Insert(0, value);
                else
                    m_Text = value;
            } 
        }
        
        
        public bool SaveAble
        {
            get { return null == File ? false : !File.IsReadOnly; }
        }

        public View.IDocumentView View { get; set; }

        public ADocument(string fileName, SingleInstanceApp app)
        {
            if (String.IsNullOrEmpty(fileName))
                throw new ArgumentNullException("fileName");
            if (null == app)
                throw new ArgumentNullException("app");

            Application = app;
            m_FileName = fileName;
            Changed = true;

            ID = ++ADocument.s_Documents;
        }

        public ADocument(FileInfo file, SingleInstanceApp app) : this(file.Name, app)
        {
            File = file;

            string text = null;
            using (var stream = file.OpenText())
            {
                text = stream.ReadToEnd();
                stream.Close();
            }

            this.Text = text;
            Changed = false;
        }

        public void Activate()
        {
            Application.ActiveDocument = this;
        }

        public void Close()
        {
            
        }
        
        /// <summary>
        /// 
        /// </summary>
        /// <exception cref="InvalidOperationException"></exception>
        /// <exception cref="UnauthorizedAccessException"></exception>
        public void Save()
        {
            if(null == File)
                throw new InvalidOperationException("No file specified. Call SaveAs");
            //if (!SaveAble)
            //    throw new UnauthorizedAccessException("file is read-only");

            using (var file = new System.IO.StreamWriter(File.Open(FileMode.Truncate, FileAccess.Write)))
            {
                //System.Diagnostics.Debug.WriteLine(Text);   
                file.Write(Text);
                file.Close();
            }

            Changed = false;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="path"></param>
        /// <exception cref="ArgumentNullException"/>
        public void SaveAs(string path)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentNullException("path");
                                    
            //create file
            System.IO.File.CreateText(path).Close();            
            File = new FileInfo(path);
            m_FileName = File.Name;
            
            Save();
        }

        private void FireStateChangedEvent(bool bModified)
        {
            if (null != StateChanged)
                StateChanged(this, new DocumentStateChangedEventArg(bModified));
        }
    }
}
