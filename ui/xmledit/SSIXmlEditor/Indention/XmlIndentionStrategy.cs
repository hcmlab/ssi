using System;
using ICSharpCode.AvalonEdit.Document;
using ICSharpCode.AvalonEdit.Indentation;
using ICSharpCode.AvalonEdit.Utils;
using System.Collections.Generic;
using System.Text;

namespace SSIXmlEditor.Indentation
{
    public class XmlIndentionStrategy : IIndentationStrategy
    {
        public virtual void IndentLine(TextDocument document, DocumentLine line)
        {
            DocumentLine previousLine = line.PreviousLine;
            if (previousLine != null)
            {
                ISegment indentationSegment = TextUtilities.GetWhitespaceAfter(document, previousLine.Offset);
                string indentation = document.GetText(indentationSegment);
                
                string text = document.GetText(previousLine.Offset, previousLine.Length).Trim();
                string curtext = document.GetText(line.Offset, line.Length).Trim();

				System.Diagnostics.Debug.WriteLine(curtext);

                if (text.StartsWith("<") && text.EndsWith("/>"))
                {
                    indentationSegment = TextUtilities.GetWhitespaceAfter(document, line.Offset);
                    document.Replace(indentationSegment, indentation);
                }
                else if (text.StartsWith("</") || curtext.StartsWith("</"))
                {                  
                    indentationSegment = TextUtilities.GetWhitespaceAfter(document, line.Offset);
                    document.Replace(indentationSegment, indentation);
                }
                else if (text.StartsWith("<") && text.EndsWith(">"))
                {
                    indentationSegment = TextUtilities.GetWhitespaceAfter(document, line.Offset);
                    document.Replace(indentationSegment, indentation + "\t");
                }               
            }
        }

        //TODO: gscheit implementieren...
        public virtual void IndentLines(TextDocument document, int beginLine, int endLine)
        {
            var stack = new Stack<string>();
            DocumentLine line = document.GetLineByNumber(beginLine);
            var text = document.GetText(line.Offset, line.Length).Trim();
            if (text.StartsWith("<?"))
                ++beginLine;

            for (int i = beginLine; i < endLine; ++i)
            {
                line = document.GetLineByNumber(i);
                text = document.GetText(line.Offset, line.Length).Trim();
                var indentationSegment = TextUtilities.GetWhitespaceAfter(document, line.Offset);

                if (1 == i)
                {
                    document.Replace(indentationSegment, "");
                    stack.Push("\t");
                    continue;
                }
                                
                if (text.StartsWith("</") && stack.Count > 0)
                    stack.Pop();

                var sb = new StringBuilder();
                foreach (var tab in stack.ToArray())
                    sb.Append(tab);

                document.Replace(indentationSegment, sb.ToString());

                if (text.StartsWith("<!--") || text.EndsWith("-->"))
                    ;
                else if (text.StartsWith("<") && text.EndsWith("/>"))
                    ;
                else if (text.StartsWith("</"))
                    ;
                else if (text.StartsWith("<") && text.EndsWith(">"))
                    stack.Push("\t");                             
            }
        }
    }
}
