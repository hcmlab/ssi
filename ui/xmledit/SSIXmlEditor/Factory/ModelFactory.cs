using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Reflection.Emit;

namespace SSIXmlEditor.Factory
{
    class ModelFactory
    {
        private static Dictionary<string, TypeBuilder> m_TypeBuilders;

        static ModelFactory()
        {
            m_TypeBuilders = new Dictionary<string, TypeBuilder>();
        }

        public static object Create(Type source, MetaData data)
        {
            if (null == source)
                throw new ArgumentNullException("source");
            if (null == data)
                throw new ArgumentNullException("data");

            var name = data.Name + source.Name;

            TypeBuilder builder = null;
            if (m_TypeBuilders.ContainsKey(name))
                builder = m_TypeBuilders[name];
            else
            {
                builder = Builder.ModelBuilder.Build(source, data);
                m_TypeBuilders.Add(name, builder);
            }

            Type type = builder.CreateType();
            var obj = Activator.CreateInstance(type);
            type.GetProperty("Library").SetValue(obj, System.IO.Path.GetFileName(data.Lib), null);

            foreach (var item in data.Options)
            {
                var opt = item as Option;

                var prop = type.GetProperty(opt.Name);
                prop.SetValue(obj, opt.Value, null);
            }

            return obj;
        }
    }
}
