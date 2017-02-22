using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Collections;
using System.Runtime.InteropServices;

namespace SSIXmlEditor
{
	[TypeConverter(typeof(OptionConverter))]
	public class Option : ICloneable
	{
		private string m_Value;
		
		public string Name { get; private set; }
		public string Help { get; private set; }
		public string Value 
		{ 
			get { return m_Value; }
			set { m_Value = value; }
		}
				
		private Option() {}		
			
		public Option(XMLReader.Option opt)
		{
			Name = opt.Name;
			Help = opt.Help;
			
			Value = opt.Value;
		}

		#region ICloneable Member

		public object Clone()
		{
			return new Option { Value = this.Value, Name = this.Name, Help = this.Help };
		}

		#endregion
	}
	
	public class OptionConverter : StringConverter 
	{
		public override object ConvertTo(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, Type destinationType)
		{
			if(destinationType == typeof(string) && value is Option)
			{
				Option opt = value as Option;
				return opt.Value;
			}
		
			return base.ConvertTo(context, culture, value, destinationType);
		}

		public override object ConvertFrom(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
		{
			return value;
			//return base.ConvertFrom(context, culture, value);			
		}

		public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
		{
			if(sourceType == typeof(string))
				return true;
			
			return base.CanConvertFrom(context, sourceType);
		}

		public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
		{
			return base.CanConvertTo(context, destinationType);
		}
	}
	
	public class GCollection<T, TTypeDescriptor> : System.Collections.CollectionBase, ICustomTypeDescriptor
		where TTypeDescriptor : PropertyDescriptor  
	{
		private Func<GCollection<T, TTypeDescriptor>, int, TTypeDescriptor> m_TypeDescriptorDel;
		public T this[int index] { get { return (T)this.List[index]; } }

		public void Add(T item) { this.List.Add(item); }

		public GCollection(Func<GCollection<T, TTypeDescriptor>, int, TTypeDescriptor> function)
		{
			m_TypeDescriptorDel = function;
		}

        public IList Collection { get { return this.List; } }

		#region ICustomTypeDescriptor Member

		public AttributeCollection GetAttributes()
		{
			return TypeDescriptor.GetAttributes(this, true);
		}

		public string GetClassName()
		{
			return TypeDescriptor.GetClassName(this, true);
		}

		public string GetComponentName()
		{
			return TypeDescriptor.GetComponentName(this, true);
		}

		public TypeConverter GetConverter()
		{
			return TypeDescriptor.GetConverter(this, true);
		}

		public EventDescriptor GetDefaultEvent()
		{
			return TypeDescriptor.GetDefaultEvent(this, true);
		}

		public PropertyDescriptor GetDefaultProperty()
		{
			return TypeDescriptor.GetDefaultProperty(this, true);
		}

		public object GetEditor(Type editorBaseType)
		{
			return TypeDescriptor.GetEditor(this, editorBaseType, true);
		}

		public EventDescriptorCollection GetEvents(System.Attribute[] attributes)
		{
			return TypeDescriptor.GetEvents(this, attributes, true);
		}

		public EventDescriptorCollection GetEvents()
		{
			return TypeDescriptor.GetEvents(this, true);
		}

		public PropertyDescriptorCollection GetProperties(System.Attribute[] attributes)
		{
			return GetProperties();
		}

		public PropertyDescriptorCollection GetProperties()
		{
			var pds = new PropertyDescriptorCollection(null);

			for (int i = 0; i < this.List.Count; ++i)
			{
				var pd = m_TypeDescriptorDel(this, i);
				pds.Add(pd);
			}

			return pds;
		}

		public object GetPropertyOwner(PropertyDescriptor pd)
		{
			return this;
		}

        public override string ToString()
        {
            return "";
        }

		#endregion
	}

	public class OptionCollectionPropertyDescriptor : PropertyDescriptor
	{
		private GCollection<Option, OptionCollectionPropertyDescriptor> m_List;
		private int m_iIndex;

		public OptionCollectionPropertyDescriptor(GCollection<Option, OptionCollectionPropertyDescriptor> list, int id)
			: base("#" + id.ToString(), null)
		{
			m_List = list;

			m_iIndex = id;
		}

		public override bool CanResetValue(object component)
		{
			return true;
		}

		public override Type ComponentType
		{
			get { return this.m_List.GetType(); }
		}

		public override object GetValue(object component)
		{
			return this.m_List[m_iIndex].Value.ToLower();
		}

		public override bool IsReadOnly
		{
			get { return false; }
		}

		public override Type PropertyType
		{
			get { return this.m_List[m_iIndex].GetType(); }
		}

		public override string DisplayName
		{
			get
			{
				Option opt = m_List[m_iIndex];
				return opt.Name;
			}
		}

		public override string Description
		{
			get
			{
				Option opt = m_List[m_iIndex];
				return opt.Help;
			}
		}

		public override void ResetValue(object component)
		{

		}

		public override void SetValue(object component, object value)
		{
			Option opt = this.m_List[m_iIndex];
			opt.Value = value.ToString().ToLower();
		}

		public override bool ShouldSerializeValue(object component)
		{
			return true;
		}
	}

	public class ChannelCollectionPropertyDescriptor : PropertyDescriptor
	{
		private GCollection<Channel, ChannelCollectionPropertyDescriptor> m_List;
		private int m_iIndex;

		public ChannelCollectionPropertyDescriptor(GCollection<Channel, ChannelCollectionPropertyDescriptor> list, int id)
			: base("#" + id.ToString(), null)
		{
			m_List = list;

			m_iIndex = id;
		}

		public override bool CanResetValue(object component)
		{
			return true;
		}

		public override Type ComponentType
		{
			get { return this.m_List.GetType(); }
		}

		public override object GetValue(object component)
		{
			//return this.m_List[m_iIndex];
			return this.m_List[m_iIndex].Info;
		}

		public override bool IsReadOnly
		{
			get { return false; }
		}

		public override Type PropertyType
		{
			get { return this.m_List[m_iIndex].GetType(); }
		}

		public override string DisplayName
		{
			get
			{
				Channel channel = m_List[m_iIndex];
				return channel.Name;
			}
		}

		public override string Description
		{
			get
			{
				Channel channel = m_List[m_iIndex];
				return channel.Info;
			}
		}

		public override void ResetValue(object component)
		{

		}

		public override void SetValue(object component, object value)
		{
			
		}

		public override bool ShouldSerializeValue(object component)
		{
			return true;
		}
	}
	
	public enum ObjectType : int 
	{
        SSI_OBJECT = 0,
        SSI_PROVIDER = 1,
        SSI_CONSUMER = 2,
        SSI_TRANSFORMER = 3,
        SSI_FEATURE = 4,
        SSI_FILTER = 5,
        SSI_TRIGGER = 6,
        SSI_MODEL = 7,
        SSI_MODEL_CONTINUOUS = 8,
        SSI_FUSION = 9,
        SSI_SENSOR = 10,
        SSI_SELECTION = 11
	}
	
	public class MetaData
	{
        [CategoryAttribute("Options")]
        [Browsable(false)]
        public string OptionName { get; set; }
		public string Lib { get; private set; }
		public string Name { get; private set; }
		public string Info { get; private set; }
		public ObjectType Type { get; private set; }
		[Browsable(false)]
		public virtual bool HasTrigger { get; protected set; }
				
		[TypeConverter(typeof(ExpandableObjectConverter))]
		[CategoryAttribute("Options")]
		public GCollection<Option, OptionCollectionPropertyDescriptor> Options { get; private set; }

        public XMLReader.MetaData Reference { get; private set; }

		public MetaData(XMLReader.MetaData m)
		{
            Reference = m;

			Lib = m.Lib;
			Name = m.Name;
			Info = m.Info;
			Type = (ObjectType) m.Type;

			Options = new GCollection<Option, OptionCollectionPropertyDescriptor>((p1, p2) => new OptionCollectionPropertyDescriptor(p1, p2));
			
			foreach(var item in m.Options)
				Options.Add(new Option(item));			
		}
	}
	
	public class Channel
	{
		public String Name { get; set; }
		public String Info { get; set; }
	}
	
	public class ConsumerMetaData : MetaData
	{
		public ConsumerMetaData(XMLReader.MetaData m) : base(m)
		{
		}
	}
		
	public class SensorMetaData : MetaData
	{
		[TypeConverter(typeof(ExpandableObjectConverter))]
		[CategoryAttribute("Channels")]
		public GCollection<Channel, ChannelCollectionPropertyDescriptor> Channels { get; private set; }
		
		//public GCollection<Option, OptionCollectionPropertyDescriptor> Options { get; private set; }
		
		public SensorMetaData(XMLReader.SensorMetaData m) : base(m)
		{
			Channels = new GCollection<Channel, ChannelCollectionPropertyDescriptor>((p1, p2) => new ChannelCollectionPropertyDescriptor(p1, p2));
			
			m.Channels.ForEach(c => Channels.Add(new Channel() { Name = c.Name, Info = c.Info }));
		}
	}
}
