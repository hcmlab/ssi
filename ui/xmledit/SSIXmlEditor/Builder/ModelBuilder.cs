using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Reflection.Emit;
using System.ComponentModel;

namespace SSIXmlEditor.Builder
{
    class ModelBuilder
    {
        /// <summary>
        /// Creates a new dynamic type 
        /// </summary>
        /// <param name="source">type to inherit from</param>
        /// <param name="data"></param>
        /// <returns></returns>
        public static TypeBuilder Build(Type source, MetaData data)
        {
            AppDomain domain = System.Threading.Thread.GetDomain();
            AssemblyName asmName = new AssemblyName();
            asmName.Name = data.Name;

            AssemblyBuilder asmBuilder = domain.DefineDynamicAssembly(asmName, System.Reflection.Emit.AssemblyBuilderAccess.Run);
            ModuleBuilder moduleBuilder = asmBuilder.DefineDynamicModule(data.Name + "Module");

			//TypeBuilder typeBuilder = moduleBuilder.DefineType(data.Name, 
			//    TypeAttributes.Public | TypeAttributes.Class,               
			//    source, new Type[] { typeof(ISSIModel) });

			TypeBuilder typeBuilder = moduleBuilder.DefineType(data.Name, TypeAttributes.Public | TypeAttributes.Class, source);

            ConstructorInfo ctorCategoryInfo = typeof(CategoryAttribute).GetConstructor(new Type[] { typeof(string) });
            ConstructorInfo ctorDescriptionInfo = typeof(DescriptionAttribute).GetConstructor(new Type[] { typeof(string) });
            CustomAttributeBuilder attCategoryBuilder = new CustomAttributeBuilder(ctorCategoryInfo, new object[] { "Options" });

            foreach (var item in data.Options.Collection)
            {
                var opt = item as Option;
                
                var attDescriptionBuilder = new CustomAttributeBuilder(ctorDescriptionInfo, new object[] { opt.Help });

                var prop = CreateProperty(typeBuilder, opt.Name, false);
                prop.SetCustomAttribute(attCategoryBuilder);
                prop.SetCustomAttribute(attDescriptionBuilder);
            }

            if (data.Type == ObjectType.SSI_SENSOR)
            {
                var sensorMd = new SensorMetaData(data.Reference as XMLReader.SensorMetaData);
                foreach (Channel channel in sensorMd.Channels.Collection)
                {
                    var attDescriptionBuilder = new CustomAttributeBuilder(ctorDescriptionInfo, new object[] { channel.Info });
                    attCategoryBuilder = new CustomAttributeBuilder(ctorCategoryInfo, new object[] { "Channels" });
					
                    var prop = CreateProperty(typeBuilder, channel.Name, false);
                    prop.SetCustomAttribute(attDescriptionBuilder);
                    prop.SetCustomAttribute(attCategoryBuilder);
                }
            }
            
            return typeBuilder;
        }

        public static PropertyBuilder CreateProperty(TypeBuilder typeBuilder, string name, bool bReadOnly)
        {
            FieldBuilder field = typeBuilder.DefineField("m_" + name, typeof(string), FieldAttributes.Private);
            PropertyBuilder prop = typeBuilder.DefineProperty(name, PropertyAttributes.HasDefault, typeof(string), new Type[] { typeof(string) });

            MethodBuilder getProp = typeBuilder.DefineMethod("Get" + name, MethodAttributes.Public, typeof(string), new Type[] { });

            ILGenerator getPropIl = getProp.GetILGenerator();
            getPropIl.Emit(OpCodes.Ldarg_0);
            getPropIl.Emit(OpCodes.Ldfld, field);
            getPropIl.Emit(OpCodes.Ret);
            prop.SetGetMethod(getProp);

            if (!bReadOnly)
            {
                MethodBuilder setProp = typeBuilder.DefineMethod("Set" + name, MethodAttributes.Public, null, new Type[] { typeof(string) });
                ILGenerator setPropIl = setProp.GetILGenerator();

                setPropIl.Emit(OpCodes.Ldarg_0);
                setPropIl.Emit(OpCodes.Ldarg_1);
                setPropIl.Emit(OpCodes.Stfld, field);
                setPropIl.Emit(OpCodes.Ret);
                
                prop.SetSetMethod(setProp);
            }

            return prop;
        }

		public static PropertyBuilder CreateProperty(TypeBuilder typeBuilder, string name, Type type, bool bReadOnly)
		{
			FieldBuilder field = typeBuilder.DefineField("m_" + name, typeof(string), FieldAttributes.Private);
			PropertyBuilder prop = typeBuilder.DefineProperty(name, PropertyAttributes.HasDefault, type, new Type[] { type });

			MethodBuilder getProp = typeBuilder.DefineMethod("Get" + name, MethodAttributes.Public, type, new Type[] { });

			ILGenerator getPropIl = getProp.GetILGenerator();
			getPropIl.Emit(OpCodes.Ldarg_0);
			getPropIl.Emit(OpCodes.Ldfld, field);
			getPropIl.Emit(OpCodes.Ret);
			prop.SetGetMethod(getProp);

			if (!bReadOnly)
			{
				MethodBuilder setProp = typeBuilder.DefineMethod("Set" + name, MethodAttributes.Public, null, new Type[] { type });
				ILGenerator setPropIl = setProp.GetILGenerator();

				setPropIl.Emit(OpCodes.Ldarg_0);
				setPropIl.Emit(OpCodes.Ldarg_1);
				setPropIl.Emit(OpCodes.Stfld, field);
				setPropIl.Emit(OpCodes.Ret);

				prop.SetSetMethod(setProp);
			}

			return prop;
		}
    }
}
