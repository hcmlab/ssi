using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.ComponentModel.Design;
using System.Drawing.Design;

namespace SSIXmlEditor.Model
{
	[TypeConverter(typeof(SequenzExpandableConverter))]
	[Editor(typeof(Editor), typeof(UITypeEditor))]
	public class Sequenz<T> : List<T>, ICustomTypeDescriptor, IList<T> where T : BaseObject 
	{
		[Browsable(false)]
		public Model.BaseObject Parent { get; set; }	
	
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
			var propertyDescriptors = new PropertyDescriptorCollection(null);
			
			for(int i = 0; i < Count; ++i)
				propertyDescriptors.Add(new GenericPropertyDescriptor<T>(this, i));
				
			return propertyDescriptors;
		}

		public object GetPropertyOwner(PropertyDescriptor pd)
		{
			return null;
		}

		#endregion

		public override string ToString()
		{
			return typeof(T).ToString() + "s";
		}

		#region IList<T> Member

		int IList<T>.IndexOf(T item)
		{
			return base.IndexOf(item);
		}

		void IList<T>.Insert(int index, T item)
		{
			base.Insert(index, item);
		}

		void IList<T>.RemoveAt(int index)
		{
			base.RemoveAt(index);
		}

		T IList<T>.this[int index]
		{
			get
			{
				return base[index];
			}
			set
			{
				base[index] = value;
			}
		}

		#endregion

		#region ICollection<T> Member

		void ICollection<T>.Add(T item)
		{
			base.Add(item);	
		}

		void ICollection<T>.Clear()
		{
			base.Clear();
		}

		bool ICollection<T>.Contains(T item)
		{
			return base.Contains(item);
		}

		void ICollection<T>.CopyTo(T[] array, int arrayIndex)
		{
			base.CopyTo(array, arrayIndex);
		}

		int ICollection<T>.Count
		{
			get { return base.Count; }
		}

		bool ICollection<T>.IsReadOnly
		{
			get { return false; }
		}

		bool ICollection<T>.Remove(T item)
		{
			return base.Remove(item);
		}

		#endregion

		#region IEnumerable<T> Member

		IEnumerator<T> IEnumerable<T>.GetEnumerator()
		{
			return base.GetEnumerator();
		}

		#endregion

		#region IEnumerable Member

		System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
		{
			return base.GetEnumerator();
		}

		#endregion
	}
	
	public class SequenzExpandableConverter : ExpandableObjectConverter
	{
		public override object ConvertTo(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, Type destinationType)
		{
            if (value == null)
            {
                return null;
            }

			if(destinationType == typeof(string))
				return value.ToString();
		
			return base.ConvertTo(context, culture, value, destinationType);
		}
	}

	class GenericPropertyDescriptor<T> : PropertyDescriptor where T : BaseObject 
	{
		private Sequenz<T> m_List;
		private int m_Index;

		public GenericPropertyDescriptor(Sequenz<T> list, int index) : base((index + 1).ToString(), null)
		{
			m_Index = index;
			m_List = list;
		}
		
		public override bool CanResetValue(object component)
		{
			return true;
		}

		public override Type ComponentType
		{
			get { return m_List.GetType(); }
		}

		public override object GetValue(object component)
		{
			return m_List[m_Index];
		}

		public override bool IsReadOnly
		{
			get { return true; }
		}

		public override Type PropertyType
		{
			get { return m_List[m_Index].GetType(); }
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
	
	class Editor : CollectionEditor
	{
		private bool Canceled { get; set; }
		private List<object> m_Items;
		
		public Editor(Type type) : base(type)
		{
			Canceled = false;
			m_Items = new List<object>();
		}

		protected override CollectionEditor.CollectionForm CreateCollectionForm()
		{
			var form = base.CreateCollectionForm();
			form.FormClosed += new System.Windows.Forms.FormClosedEventHandler(form_FormClosed);
			
			m_Items.Clear();
			return form;
		}

		void form_FormClosed(object sender, System.Windows.Forms.FormClosedEventArgs e)
		{
			if(Canceled)
				return;
			
            //var document = App.SingleInstanceApp.Instance.ActiveDocument.InputHandler;
            //var line = document.CurrentLine + 1;
            //foreach(var item in m_Items)
            //{
            //    document.Insert(document.GetOffsetOfLine(line++), "<" + item.GetType().Name + "/>" + Environment.NewLine);
            //}		
		}

		protected override void CancelChanges()
		{
			Canceled = true;
			base.CancelChanges();
		}

		protected override object CreateInstance(Type itemType)
		{
			var item = base.CreateInstance(itemType);
			m_Items.Add(item);
			
			return item;
		}
		
		protected override object SetItems(object editValue, object[] value)
		{
			var obj = base.SetItems(editValue, value);
			return obj;
		} 	
	}

}
