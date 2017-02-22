using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Reflection.Emit;
using System.Xml;
using System.ComponentModel;
using SSIXmlEditor.Attribute;
using SSIXmlEditor.Model;

namespace SSIXmlEditor.Builder
{
	public static class ClassBuilder
	{
		public static Dictionary<string, Type> CreateClass(string file)
		{
			string xml = System.IO.File.ReadAllText(file);
		
			var doc = new XmlDocument();
			doc.LoadXml(xml);
		
			string name = "Test";
			AppDomain domain = System.Threading.Thread.GetDomain();
			AssemblyName asmName = new AssemblyName();
			asmName.Name = name;


			AssemblyBuilder asmBuilder = domain.DefineDynamicAssembly(asmName, System.Reflection.Emit.AssemblyBuilderAccess.Run);
			ModuleBuilder moduleBuilder = asmBuilder.DefineDynamicModule(name + "Module", true);

			var clsDict = new Dictionary<string, TypeBuilder>();
			
			foreach(XmlNode node in doc.SelectSingleNode("ClassDef").ChildNodes)
			{
				if(node is XmlComment)
					continue;
		
				string szClass = node.Name;
				TypeBuilder cls1 = null;
				
				if(node.Attributes["extends"] == null)
					cls1 = CreateClass(moduleBuilder, szClass);
				else
				{
					var baseClass = node.Attributes["extends"].Value;
					System.Diagnostics.Debug.Assert(!string.IsNullOrEmpty(baseClass));
					
					var baseClassType = clsDict[baseClass].CreateType();
					cls1 = CreateClass(moduleBuilder, szClass, baseClassType);
				}
				
				
				clsDict.Add(szClass, cls1);
				
				if(node.Attributes.Count > 0)
				{
					if(node.Attributes["insertInNode"] != null && 
					   node.Attributes["insertType"] != null &&
					   node.Attributes["insertValue"] != null &&
					   node.Attributes["insertInAttribute"] != null)
					{
						SetDependencyUpdateAttribute(cls1, node.Attributes["insertInNode"].Value,
														   node.Attributes["insertInAttribute"].Value,
														   node.Attributes["insertType"].Value,
														   node.Attributes["insertValue"].Value);
					}	
					else
						System.Diagnostics.Debug.WriteLine("Missing attributes in " + node.Name);
				}

				if (node.Attributes["selectable"] != null)
				{
					if (Boolean.Parse(node.Attributes["selectable"].Value))
						SetCustomAttribute(cls1, typeof(SelectableAttribute), Type.EmptyTypes, new object[] { });
				}
				
				foreach(XmlNode models in node.ChildNodes)
				{				
					var categoryNode = models.Attributes["category"];
					var categoryDescNode = models.Attributes["categoryDesc"];
					var attributeNameNode = models.Attributes["attribute"];
					var dependecyNode = models.Attributes["dependency"];
					var dependecyAttributeNode = models.Attributes["dependencyAttribute"];
					
					
					string propertyName = models.Attributes["name"].Value;
					string typeName = models.Attributes["type"].Value;
					
					Type type = null;
					
					
					if(typeName.Contains(":"))
					{
						var types = typeName.Split(':');
						Type objType = null;
						
						if(clsDict.ContainsKey(types[1]))
							objType = clsDict[types[1]].CreateType();
						else
							objType = Type.GetType(types[1]);
						
						if(types[0].Equals("Sequenz"))
						{
							var hack = typeof(Model.Sequenz<Model.BaseObject>);
							
							type = Type.GetType("SSIXmlEditor.Model.Sequenz`1");
							type = type.MakeGenericType(new Type[] { objType });
						}
						else
							type = Type.GetType(types[0]).MakeGenericType(new Type[] { objType });
					}
					else if(clsDict.ContainsKey(typeName))
					{
						type = clsDict[typeName].CreateType();
					}
					else
						type = Type.GetType(typeName);
					
					PropertyBuilder property = ModelBuilder.CreateProperty(cls1, propertyName, type, false);
					
					if(models.Attributes["readonly"] != null)
					{
						if(Boolean.Parse(models.Attributes["readonly"].Value))
							SetCustomAttribute(property, typeof(ReadOnlyAttribute), new Type[] { typeof(bool) }, new Object[] { true });
					}

					if (models.Attributes["visible"] != null)
					{
						if (!Boolean.Parse(models.Attributes["visible"].Value))
							SetCustomAttribute(property, typeof(BrowsableAttribute), new Type[] { typeof(bool) }, new object[] { false });
					}
					
					if(models.Attributes["dataSource"] != null)
						SetCustomAttribute(property, typeof(TypeConverterAttribute), new Type[] { typeof(Type) }, new Object[] { typeof(DataSourceConverter) });
					
					SetExpandableClassAttribute(cls1, typeof(ExpandableObjectConverter));
					
					if(null != models.Attributes["hasMetadata"])
						SetCustomAttribute(property, typeof(MetaDataAttribute), Type.EmptyTypes, new object[] {});
					if (null != attributeNameNode)
						SetAttributeNameAttribute(property, attributeNameNode.Value);
					if(null != dependecyNode && null != dependecyAttributeNode)
						SetDependencyAttribute(property, dependecyNode.Value, dependecyAttributeNode.Value);;
					if(null != categoryNode)
						SetCategoryAttribute(property, categoryNode.Value);
					if(null != categoryDescNode)
						SetCategoryDescriptionAttribute(property, categoryDescNode.Value);
				}
			}
										
			var typeDict = new Dictionary<string, Type>();
			foreach(var item in clsDict)
				typeDict.Add(item.Key, item.Value.CreateType());
			
			return typeDict;
		}

		private static void SetDependencyUpdateAttribute(TypeBuilder typeBuilder, string insertInNode, string insertInAttribute, string insertType, string insertValue)
		{
			ConstructorInfo ctorCategoryInfo = typeof(DependencyUpdateAttribute).GetConstructor(new Type[] { typeof(string), typeof(string), typeof(string), typeof(string) });
			CustomAttributeBuilder attAttributeNameBuilder = new CustomAttributeBuilder(ctorCategoryInfo, new object[] { insertType, insertValue, insertInNode, insertInAttribute });

			typeBuilder.SetCustomAttribute(attAttributeNameBuilder);
		}

		private static void SetDependencyAttribute(PropertyBuilder typeBuilder, string dependency, string attribute)
		{
			ConstructorInfo ctorCategoryInfo = typeof(DependencyAttribute).GetConstructor(new Type[] { typeof(string), typeof(string) });
			CustomAttributeBuilder attAttributeNameBuilder = new CustomAttributeBuilder(ctorCategoryInfo, new object[] { dependency, attribute });

			typeBuilder.SetCustomAttribute(attAttributeNameBuilder);
		}

		private static void SetAttributeNameAttribute(PropertyBuilder propBuilder, string attributeName)
		{
			ConstructorInfo ctorCategoryInfo = typeof(DependencyAttributeValueAttribute).GetConstructor(new Type[] { typeof(string) });
			CustomAttributeBuilder attAttributeNameBuilder = new CustomAttributeBuilder(ctorCategoryInfo, new object[] { attributeName });

			propBuilder.SetCustomAttribute(attAttributeNameBuilder);
		} 	
		
		private static void SetCategoryAttribute(PropertyBuilder propBuilder, string category)
		{
			ConstructorInfo ctorCategoryInfo = typeof(CategoryAttribute).GetConstructor(new Type[] { typeof(string) });
			CustomAttributeBuilder attCategoryBuilder = new CustomAttributeBuilder(ctorCategoryInfo, new object[] { category });

			propBuilder.SetCustomAttribute(attCategoryBuilder);
		}
		
		private static void SetCategoryDescriptionAttribute(PropertyBuilder propBuilder, string desc)
		{
			ConstructorInfo ctorDescriptionInfo = typeof(DescriptionAttribute).GetConstructor(new Type[] { typeof(string) });
			var attDescriptionBuilder = new CustomAttributeBuilder(ctorDescriptionInfo, new object[] { desc });
			
			propBuilder.SetCustomAttribute(attDescriptionBuilder);
		}

		
		private static void SetExpandableAttribute(PropertyBuilder propBuilder, Type converter)
		{
			ConstructorInfo ctorDescriptionInfo = typeof(TypeConverterAttribute).GetConstructor(new Type[] { typeof(Type) });
			var attDescriptionBuilder = new CustomAttributeBuilder(ctorDescriptionInfo, new object[] { converter });

			propBuilder.SetCustomAttribute(attDescriptionBuilder);
		}

		private static void SetExpandableClassAttribute(TypeBuilder propBuilder, Type converter)
		{
			ConstructorInfo ctorDescriptionInfo = typeof(TypeConverterAttribute).GetConstructor(new Type[] { typeof(Type) });
			var attDescriptionBuilder = new CustomAttributeBuilder(ctorDescriptionInfo, new object[] { converter });

			propBuilder.SetCustomAttribute(attDescriptionBuilder);
		}
		
		private static void SetCustomAttribute(PropertyBuilder propBuilder, Type tAttribute, Type[] ctorParams, Object[] param)
		{
			ConstructorInfo ctorDescriptionInfo = tAttribute.GetConstructor(ctorParams);
			var attDescriptionBuilder = new CustomAttributeBuilder(ctorDescriptionInfo, param);
			
			propBuilder.SetCustomAttribute(attDescriptionBuilder);
		}

		private static void SetCustomAttribute(TypeBuilder propBuilder, Type tAttribute, Type[] ctorParams, Object[] param)
		{
			ConstructorInfo ctorDescriptionInfo = tAttribute.GetConstructor(ctorParams);
			var attDescriptionBuilder = new CustomAttributeBuilder(ctorDescriptionInfo, param);

			propBuilder.SetCustomAttribute(attDescriptionBuilder);
		}
		
		private static TypeBuilder CreateClass(ModuleBuilder moduleBuilder, string className)
		{
			TypeBuilder typeBuilder = moduleBuilder.DefineType(className, TypeAttributes.Public | TypeAttributes.Class, typeof(Model.BaseObject));
			var defCtor = typeBuilder.DefineConstructor(MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.RTSpecialName, CallingConventions.Standard, Type.EmptyTypes);
			var ilGen = defCtor.GetILGenerator();
			
			var baseClass = typeof(Model.BaseObject).GetConstructor(new Type[0]);
			var baseMi = typeof(Model.BaseObject).GetMethod("Init", BindingFlags.Instance | BindingFlags.NonPublic, null, new Type[] { typeof(Type)}, null );
			
			ilGen.Emit(OpCodes.Ldarg_0);
			ilGen.Emit(OpCodes.Call, baseClass);
			ilGen.Emit(OpCodes.Nop);
			ilGen.Emit(OpCodes.Nop);
			ilGen.Emit(OpCodes.Ldarg_0);
			ilGen.Emit(OpCodes.Ldarg_0);
			ilGen.Emit(OpCodes.Call, baseMi);
			ilGen.Emit(OpCodes.Nop);
			ilGen.Emit(OpCodes.Nop);
			ilGen.Emit(OpCodes.Ret);

			return typeBuilder;
		}

		private static TypeBuilder CreateClass(ModuleBuilder moduleBuilder, string className, Type parent)
		{
			TypeBuilder typeBuilder = moduleBuilder.DefineType(className, TypeAttributes.Public | TypeAttributes.Class, parent);
	
			var defCtor = typeBuilder.DefineConstructor(MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.RTSpecialName, CallingConventions.Standard, Type.EmptyTypes);
			var ilGen = defCtor.GetILGenerator();

			var baseClass = typeof(Model.BaseObject).GetConstructor(new Type[0]);
			var baseMi = typeof(Model.BaseObject).GetMethod("Init", BindingFlags.Instance | BindingFlags.NonPublic, null, new Type[] { typeof(Type) }, null);

			ilGen.Emit(OpCodes.Ldarg_0);
			ilGen.Emit(OpCodes.Call, baseClass);
			ilGen.Emit(OpCodes.Nop);
			ilGen.Emit(OpCodes.Nop);
			ilGen.Emit(OpCodes.Ldarg_0);
			ilGen.Emit(OpCodes.Ldarg_0);
			ilGen.Emit(OpCodes.Call, baseMi);
			ilGen.Emit(OpCodes.Ret);

			return typeBuilder;
		}
	}
}
