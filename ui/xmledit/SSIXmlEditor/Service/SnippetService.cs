using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ICSharpCode.AvalonEdit.Snippets;
using System.Text.RegularExpressions;

namespace SSIXmlEditor.Service
{
    class SnippetService
    {
        private Regex m_SnippetRegex;
        //@"</*(?<tag>\w+)\s*"
        public SnippetService()
        {
            m_SnippetRegex = new Regex(@"((?<text>.*?)\$(?<id>\w+)\$)|(?<text>.*)", RegexOptions.Compiled | RegexOptions.Singleline);
        }

        public Snippet CreateSnippet(Snippets.Snippet snippet)
        {
            Snippet snip = new Snippet();

            var list = new Dictionary<string, string>();
            foreach (var id in snippet.Declaration)
                list.Add(id.Name, id.Value);

            var matches = m_SnippetRegex.Matches(snippet.Code);
            var listRepl = new Dictionary<string, SnippetReplaceableTextElement>();
            
            foreach (Match match in m_SnippetRegex.Matches(snippet.Code))
            {
                string id = match.Groups["id"].Value;
                string value = String.Empty;
                if(!String.IsNullOrEmpty(id))
                    value = list[match.Groups["id"].Value];                

                SnippetReplaceableTextElement replacable = null;
                if (listRepl.ContainsKey(id))
                    replacable = listRepl[id];
                else if (!string.IsNullOrEmpty(id))
                {
                    replacable = new SnippetReplaceableTextElement { Text = value };
                    listRepl.Add(id, replacable);
                }
                
                snip.Elements.Add(new SnippetTextElement { Text = match.Groups["text"].Value });
                if (!String.IsNullOrEmpty(id))
                {
                    if(!snip.Elements.Contains(replacable))
                        snip.Elements.Add(replacable);
                    else
                        snip.Elements.Add(new SnippetBoundElement { TargetElement = replacable });
                }
            }

            
            return snip;
        }
    }
}
