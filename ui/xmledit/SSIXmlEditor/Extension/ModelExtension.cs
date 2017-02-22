using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ICSharpCode.AvalonEdit.Document;
using System.Xml.Linq;
using System.Xml;
using System.ComponentModel;
using System.Text.RegularExpressions;
using System.Collections;
using System.Xml.XPath;
using System.IO;
using SSIXmlEditor.Attribute;

namespace SSIXmlEditor.Extension
{
    public static class ModelExtension
    {
		private static XDocument CreateDocument(string text)
		{
			XDocument data = null;

			try
			{
				data = XDocument.Parse(text, LoadOptions.SetBaseUri | LoadOptions.SetLineInfo | LoadOptions.PreserveWhitespace);
			}
			catch (Exception)
			{
			}
			
			return data;
		}
		
		public static string GetAttributeValue(this Model.BaseObject obj, Model.IInputHandler document, string attribute)
		{
			var line = document.GetTextByLine(document.CurrentLine);
			if(!line.Contains(attribute))
				return string.Empty;

			var regex = new Regex(attribute + "=\"(?<value>.+?)\"");
			
			if (regex.IsMatch(line))
				return regex.Match(line).Groups["value"].Value;
			
			return string.Empty;
		}
    
		public static void AddAttribute(this Model.BaseObject model, Model.IInputHandler document, int line, string name, string value)
		{
			System.Diagnostics.Debug.WriteLine(String.Format("AddAttribute {0} := {1}", name, value));
			
			var element = XElement.Parse(document.GetTextByLine(line));	
			element.Add(new XAttribute(name, value));
			
			document.Remove(line);
			document.Insert(document.GetOffsetOfLine(line), element.ToString());
		}

        private static void SyncModel(this Model.BaseObject model, Repository.IModelRepository repository, Model.IInputHandler document, XElement element)
        {
			foreach (var child in element.Descendants())
			{
				var pChild = model[model.GetAttribute(child.Name.LocalName)];
				if (pChild is System.Collections.IList)
				{
					IList list = pChild as IList;
					
					ReadList(element, list, model, repository, document);
					break;
				}
				else
				{
					if (null != pChild)
					{
						SyncAttributes(pChild as Model.BaseObject, child);
						model[model.GetAttribute(child.Name.LocalName)] = pChild;

						if (child.HasElements)
						{
							(pChild as Model.BaseObject).SyncModel(repository, document, child);
						}
					}
				}
			}        
        }
        
        public static void SyncModel(this Model.BaseObject model, Repository.IModelRepository repository, Model.IInputHandler document, int line)
        {	
			var reader = XmlTextReader.Create(new StringReader(document.GetText()));

			//model.Line = document.CurrentLine;		
			model.Line = line;
			MoveToLine(line, reader);//MoveToLine(document.CurrentLine, reader);
        	
        	var element = XElement.Load(reader.ReadSubtree(), LoadOptions.SetLineInfo) as XElement;
        		 
			SyncAttributes(model, element);
						
			foreach(var child in element.Descendants())
			{
				var pChild = model[model.GetAttribute(child.Name.LocalName)];
				if(pChild is System.Collections.IList)
				{
					IList list = pChild as IList;
					ReadList(element, list, model, repository, document);
					
					break;
				}
				else
				{
					if(null != pChild)
					{
						SyncAttributes(pChild as Model.BaseObject, child);
						model[model.GetAttribute(child.Name.LocalName)] = pChild;
						
						if(child.HasElements)
						{
							(pChild as Model.BaseObject).SyncModel(repository, document, child);
						}
					}
				}
			}

			reader.Close();
				
			//check dependencies
			object[] depUpdateAttr;
			
			if ((depUpdateAttr = model.GetType().GetCustomAttributes(typeof(DependencyUpdateAttribute), true)).Count() != 0)
			{
				var doc  = CreateDocument(document.GetText());
		
				System.Diagnostics.Debug.Assert(depUpdateAttr.Count() == 1);
				DependencyUpdateAttribute attr = depUpdateAttr[0] as DependencyUpdateAttribute;

				var type = attr.InsertNodeType;
				var insertInNode = attr.InsertInNode;
				var value = attr.InsertValue;

				var insertNode = doc.Root.Element(insertInNode) as XElement;
				var typeNode = insertNode.Element(type);
				var toModel = repository.getModel(type) as Model.BaseObject;
				
				if(null == typeNode)
				{
					insertNode.Add(new XElement(type, new XAttribute(attr.InsertInAttribute, model[value])));
					
					document.Remove(((IXmlLineInfo)insertNode).LineNumber);
					document.Insert(document.GetOffsetOfLine(((IXmlLineInfo)insertNode).LineNumber), insertNode.ToString());
				}
				else
				{
					var test = insertNode.Elements(type).Where(n => n.Attributes() != null);
					bool bInsert = true;
					foreach(var item in test)
					{
						if(item.Attribute(attr.InsertInAttribute).Value.Equals(model[value]))
							bInsert = false;							
					}

					if(bInsert)
					{
						var newEntry = new XElement(type, new XAttribute(attr.InsertInAttribute, model[value]));
						document.Insert(document.GetEndOffsetOfLine(((IXmlLineInfo)insertNode).LineNumber), Environment.NewLine + newEntry.ToString());
					}
				}
			}
        }
        
        public static void ChangeModel(this Model.BaseObject model, Model.IInputHandler document, string changedAttribute, string oldValue)
        {            
            //document.CorrectLineEndings();

            /*string time = DateTime.Now.ToString("hh-mm-ss");
            StreamWriter logfile = new StreamWriter("ChangeModel_" + time + ".log");
            logfile.WriteLine("-----------\r\nBEFORE\r\n-----------\r\n\r\n");
            logfile.Write(document.GetText());*/

			var reader = XmlTextReader.Create(new StringReader(document.GetText()));
			string newValue = model[changedAttribute].ToString();
						
			MoveToLine(model.Line, reader);
			
			var element = XElement.Load(reader.ReadSubtree(), LoadOptions.PreserveWhitespace) as XElement;
			var removeText = element.ToString();
			//delete line
			var changedAttributeNode = element.Attribute(model.GetAttribute2(changedAttribute));
			if(null != changedAttributeNode)
			{
				if(!string.IsNullOrEmpty(newValue))
					changedAttributeNode.Value = (string)model[changedAttribute];
				else
					changedAttributeNode.Remove();
			}
			else
			{
				if(!string.IsNullOrEmpty(newValue))
					element.Add(new XAttribute(model.GetAttribute2(changedAttribute), model[changedAttribute]));
			}
			 
			var text = document.GetTextByLine(model.Line).TrimEnd();
			int ws = text.Length - text.Trim().Length;            

            int offset = document.GetOffsetOfLine(model.Line) + ws;
            int len = removeText.Length;
            string value = element.ToString();

            /*logfile.WriteLine("\r\n\r\n-----------\r\nREPLACE\r\n-----------\r\n\r\n");
            logfile.WriteLine("offset\t" + offset);
            logfile.WriteLine("length\t" + len);
            logfile.WriteLine("replace\t" + removeText);
            logfile.WriteLine("with\t" + value);*/

			document.Replace(offset, len, value);

            /*logfile.WriteLine("\r\n\r\n-----------\r\nAFTER\r\n-----------\r\n\r\n");
            logfile.Write(document.GetText());
            logfile.Close();*/

			if(model.HasDependencyAttribute(model.GetAttribute(changedAttribute)))
			{
				var dependency = model.GetDependency(model.GetAttribute(changedAttribute));
				
				while(reader.ReadToFollowing(dependency.Key))
				{
					if(!reader.HasAttributes)
						continue;
						
					var currentLine = (reader as IXmlLineInfo).LineNumber;
					
					XElement dependencyNode = XElement.Parse(reader.ReadOuterXml(), LoadOptions.PreserveWhitespace);
					var cRemove = dependencyNode.ToString().Length;
					
					var attribute = dependencyNode.Attribute(dependency.Value);
					if(null == attribute || !attribute.Value.Equals(oldValue))
						continue;
					
					attribute.Value = (string)model[model.GetAttribute(changedAttribute)];
					if(string.IsNullOrEmpty(attribute.Value))
						attribute.Remove();

					text = document.GetTextByLine(currentLine).TrimEnd();
					ws = text.Length - text.Trim().Length;
					
					document.Replace(document.GetOffsetOfLine(currentLine) + ws, cRemove, dependencyNode.ToString());
				}
			}
        }
        
        private static void MoveToLine(int line, XmlReader reader)
        {
			while(!(((IXmlLineInfo)reader).LineNumber == line) && reader.Read()) {}
        }
        
        private static void SyncAttributes(Model.BaseObject model, XElement element)
        {
			model.Line = (element as IXmlLineInfo).LineNumber;
            foreach (var att in element.Attributes())
            {
                try
                {
                    model[model.GetAttribute(att.Name.LocalName)] = att.Value;
                }
                catch (Exception e)
                {
                }
            }
        }

		private static void ReadList(XElement parent, IList model, Model.BaseObject parentObj, Repository.IModelRepository repository, Model.IInputHandler document)
        {
			var currentLine = document.CaretPosition;
        
			foreach(XElement child in parent.Descendants())
			{
				document.CaretPosition = document.GetOffsetOfLine((child as IXmlLineInfo).LineNumber);
				var objChild = repository.getModel(child.Name.LocalName) as Model.BaseObject;
				
				objChild.Parent = parentObj;
								
				objChild.Line = (child as IXmlLineInfo).LineNumber;
				SyncAttributes(objChild, child);
				
				model.Add(objChild);
			}
			
			document.CaretPosition = currentLine;
        }
        
        private static string GetXmlElement(Model.IInputHandler document)
        {
			var currentLine = document.CurrentLine;
			var line = document.GetCurrentLineText();

			Matcher xmlTag = new Matcher();
			var tag = xmlTag.getTagName(document.GetCurrentLineText());
			var sb = new StringBuilder();

			while (currentLine < document.Lines && !line.Contains("</" + tag + ">"))
			{
				sb.Append(line);
				line = document.GetTextByLine(++currentLine);
			}
			sb.Append(line);
			return sb.ToString();
        }
        
    }
}
