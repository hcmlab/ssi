using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.Extension;

namespace SSIXmlEditor.Repository
{
	public interface IModelRepository
	{
		Model.IInputHandler InputHandler { set; }
		Object getModel(string name);
	}
	
	class ModelRepository : IModelRepository
	{
		private Dictionary<string, Type> m_Models;
		private Model.SSIModel m_SSIModel;

		public Model.IInputHandler InputHandler { set; private get; }
		
		public ModelRepository(string file, Model.SSIModel ssiModel)
		{
			m_Models = new Dictionary<string,Type>();
			m_SSIModel = ssiModel;
			
			foreach(var entry in Builder.ClassBuilder.CreateClass(file))
				m_Models.Add(entry.Key, entry.Value);
		}

		#region IModelRepository Member

		public object getModel(string name)
		{	
			if(!m_Models.ContainsKey(name))
				return null;
				
			Object obj = Activator.CreateInstance(m_Models[name]);
			if(obj is Model.BaseObject)
			{
				Model.BaseObject tmpObj = obj as Model.BaseObject;
				if(tmpObj.HasMetaData)
				{
					var mdValue = tmpObj.GetAttributeValue(InputHandler, tmpObj.GetAttribute2(tmpObj.MetaDataAttribute));
					if(!string.IsNullOrEmpty(mdValue))
					{
						var metaData = m_SSIModel.GetMetaData(mdValue);
						obj = Factory.ModelFactory.Create(obj.GetType(), metaData);
					}
				}
			}
			
			return obj;
		}

		#endregion
	}
}
