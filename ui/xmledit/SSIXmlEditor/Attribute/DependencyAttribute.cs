using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Attribute
{
	[global::System.AttributeUsage(AttributeTargets.Property, Inherited = true, AllowMultiple = false)]
	public sealed class MetaDataAttribute : System.Attribute
	{
		public MetaDataAttribute() {}
	}

	[global::System.AttributeUsage(AttributeTargets.Property, Inherited = true, AllowMultiple = false)]
	public sealed class SelectableAttribute : System.Attribute
	{
		public SelectableAttribute() { }
	}

	[global::System.AttributeUsage(AttributeTargets.Property, Inherited = true, AllowMultiple = false)]
	public sealed class DependencyAttribute : System.Attribute
	{
		private readonly string m_Dependency;
		private readonly string m_DependencyAttribute;

		public string Dependency { get { return m_Dependency; } }
		public string Attribute { get { return m_DependencyAttribute; } }

		public DependencyAttribute(string dependency, string dependencyAttribute)
		{
			if(string.IsNullOrEmpty(dependency))
				throw new ArgumentNullException("dependency");
			if(string.IsNullOrEmpty(dependencyAttribute))
				throw new ArgumentNullException("dependencyAttribute");
				
			m_Dependency = dependency;
			m_DependencyAttribute = dependencyAttribute;
		}
	}

	[global::System.AttributeUsage(AttributeTargets.All, Inherited = true, AllowMultiple = true)]
	public sealed class DependencyUpdateAttribute : System.Attribute
	{
		private readonly string m_InsertInNode;
		private readonly string m_InsertInAttribute;
		private readonly string m_InsertValue;
		private readonly string m_InsertType;

		public string InsertNodeType { get { return m_InsertType; } }
		public string InsertValue { get { return m_InsertValue; } }
		public string InsertInNode { get { return m_InsertInNode; } }
		public string InsertInAttribute { get { return m_InsertInAttribute; } }

		public DependencyUpdateAttribute(string insertType, string insertValue, string insertInNode, string insertInAttribute)
		{
			m_InsertType = insertType;
			m_InsertInNode = insertInNode;
			m_InsertValue = insertValue;
			m_InsertInAttribute = insertInAttribute;
		}
	}

    [global::System.AttributeUsage(AttributeTargets.Property, Inherited = true, AllowMultiple = false)]
    public sealed class DependencyAttributeValueAttribute : System.Attribute
    {   
        private readonly string m_Attribute;

        public string Attribute { get { return m_Attribute; } }

        public DependencyAttributeValueAttribute(string attribute)
        {
            m_Attribute = attribute;
        }
    }
}
