using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Attribute
{
    [global::System.AttributeUsage(AttributeTargets.All, Inherited = true, AllowMultiple = true)]
    sealed class CreateDependencyAttribute : System.Attribute
    {
        private readonly Type m_Type;
        private readonly string m_ParentElement;
        private readonly string m_Element;
        private readonly string m_Attribute;

        public Type DependencyType { get { return m_Type; } }
        public string ParentElement { get { return m_ParentElement; } }
        public string Element { get { return m_Element; } }
        public string Attribute { get { return m_Attribute; } }

        public CreateDependencyAttribute(Type dependencyType, string parentElement, string element, string attribute)
        {
            m_Type = dependencyType;
            m_ParentElement = parentElement;
            m_Element = element;
            m_Attribute = attribute;
        }
    }
}
