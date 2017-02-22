using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using SSIXmlEditor.Attribute;
using System.ComponentModel;

namespace SSIXmlEditor.Model
{
	public abstract class BaseObject
	{
		private Dictionary<string, MethodInfo> m_Properties;
		private Dictionary<string, string> m_Attributes;
		private Dictionary<string, KeyValuePair<string, string>> m_DependencyAttribute;
		
		[Browsable(false)]
		public int Line { get; set; }
		[Browsable(false)]
		public bool HasMetaData { get; set; }
		[Browsable(false)]
		public string MetaDataAttribute { get; set; }
		[Browsable(false)]
		public BaseObject Parent { get; set; }
		[Browsable(false)]
		public bool Selectable { get; set; }
		
		public BaseObject ()
		{
			m_Properties = new Dictionary<string, MethodInfo>();
			m_Attributes = new Dictionary<string, string>();
			m_DependencyAttribute = new Dictionary<string,KeyValuePair<string,string>>();			
		}	
		
		protected void Init(Object obj)
		{
			Type type = obj.GetType();
			var properties = type.GetProperties();

			if (type.GetCustomAttributes(typeof(SelectableAttribute), true).Count() > 0)
				Selectable = true;

			
			foreach(var pi in properties)
			{
				m_Properties.Add("get_" + pi.Name, pi.GetGetMethod());
				m_Properties.Add("set_" + pi.Name, pi.GetSetMethod());
				
				var attributes = pi.GetCustomAttributes(typeof(DependencyAttributeValueAttribute), true);
				foreach(DependencyAttributeValueAttribute attr in attributes)
					m_Attributes.Add(attr.Attribute, pi.Name);
					
				attributes = pi.GetCustomAttributes(typeof(DependencyAttribute), true);
				foreach(DependencyAttribute attr in attributes)
					m_DependencyAttribute.Add(pi.Name, new KeyValuePair<string,string>(attr.Dependency, attr.Attribute));
					
				if(pi.GetCustomAttributes(typeof(MetaDataAttribute), true).Count() > 0)
				{
					HasMetaData = true;
					MetaDataAttribute = pi.Name;
				}
			} 			
		}
		
		public bool HasDependencyAttribute(string name)
		{
			return m_DependencyAttribute.ContainsKey(name);
		}
		
		public KeyValuePair<string, string> GetDependency(string name)
		{
			return m_DependencyAttribute[name];
		}
		
		public string GetAttribute(string prop)
		{
			if(m_Attributes.ContainsKey(prop))
				return m_Attributes[prop];
				
			return prop;
		}
		
		public string GetAttribute2(string prop)
		{
			if(m_Attributes.ContainsValue(prop))
				return m_Attributes.Where(e => e.Value.Equals(prop)).First().Key;
			
			return prop;
		}
		
		public bool HasGetProperty(string prop)
		{
			return m_Properties.ContainsKey("get_" + prop);				
		}
		
		public object this[string prop]
		{
			get 
			{
				if(!m_Properties.ContainsKey("get_" + prop))
					return null;
					 
				var obj = m_Properties["get_" + prop].Invoke(this, null); 
				if(null == obj)
				{
					obj = Activator.CreateInstance(m_Properties["get_" + prop].ReturnParameter.ParameterType);
					this[prop] = obj;
					
					if(obj is BaseObject)
						(obj as BaseObject).Parent = this;
				}
				
				return obj;
			}
			set 
			{ 
				if(prop.ToLower() == "pin" && this.ToString().Equals("provider"))
				{
					var channel = this["channel"].ToString();
					if(!string.IsNullOrEmpty(channel))
						Parent[this["channel"].ToString()] = value;
				}
				
				m_Properties["set_" + prop].Invoke(this, new object[] { value }); 
			}
		}
	}
	
	public class DataSourceConverter : StringConverter
	{
		public override bool GetStandardValuesSupported(ITypeDescriptorContext context)
		{
			return true;
		}

		public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
		{
			BaseObject obj = context.Instance as Model.BaseObject;
			BaseObject parent = obj.Parent;
			var items = new List<string>();
			
			var properties = parent.GetType().GetProperties();
			foreach(PropertyInfo pi in properties)
			{
				var attrs = pi.GetCustomAttributes(typeof(CategoryAttribute), true);
				if(attrs.Count() == 0)
					continue;	
				
				if(attrs.Where(c => (c as CategoryAttribute).Category.Equals("Channels")).Count() == 0)
					continue;
					
				items.Add(pi.Name);
			}
			
			
			return new StandardValuesCollection(items.ToArray());
		}
	}
}
