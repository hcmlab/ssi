using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Model
{
    public interface IInputHandler
    {
        event EventHandler TextChanged;
        event EventHandler TextEntering;
        event EventHandler TextEntered;
        event EventHandler CaretChanged;
        event EventHandler RightMouseClick;

        int Lines { get; }
        int CurrentLine { get; set; }
		int CaretPosition { get; set; }

        string GetText();
        string GetTextByLine(int lineNumber);
        string GetCurrentLineText();
        int GetOffsetOfLine(int line);
        int GetEndOffsetOfLine(int line);

        void InsertSnippet(string snippet, string[] args);
        void Insert(int offset, string text);
        void Remove(int line);
        void Replace(int offset, int len, string value);
        void BeginUpdate();
        void EndUpdate();
        void CreateAndShowContextMenu(IEnumerable<KeyValuePair<string, Action>> entries);

        void CorrectLineEndings();
    }
}
