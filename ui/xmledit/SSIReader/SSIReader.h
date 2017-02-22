// SSIReader.h

#pragma once

#include <windows.h>
#include <base/IObject.h>
#include <vcclr.h>

bool Register (const ssi_char_t *name, ssi::IObject::create_fptr_t create_fptr);

using namespace System;
using namespace System::Runtime::InteropServices;

namespace ssi { class ISensor; }

namespace XMLReader {

	[Serializable]
	public ref class Option
	{
		private:
			String^ m_Name;
			String^ m_Help;
			unsigned char m_Type;
			int		m_Num;
			void*	m_pValue;
			String^ m_Value;			

		public:
			property String^ Name
			{
			public: String^ get() { return m_Name; }
			public: void set(String^ name) { m_Name = name; }
			}

			property String^ Help
			{
			public: String^ get() { return m_Help; }
			public: void set(String^ help) { m_Help = help; }
			}

			property int Type 
			{
			public: int get() { return m_Type; }
			public: void set(int type) { m_Type = type; }
			}

			property int Num
			{
			public: int get() { return m_Num; }
			public: void set(int num) { m_Num = num; }
			}

			/*property void* pValue
			{
			public: void* get() { return m_pValue; }
			public: void set(void* value) 
					{ 
						m_pValue = value; 
					}
			}*/

			property String^ Value
			{
			public: String^ get() { return m_Value; }
			public: void set(String^ value) { m_Value = value; }
			}

	};

	[Serializable]
	public ref class Channel
	{
		public:
			String^ Name;
			String^ Info;
	};

	[Serializable]
	public ref class MetaData
	{
		public: 
			String^ Lib;
			String^ Name;
			String^ Info;
			int		Type;
			System::Collections::Generic::List<Option^>^ Options;
	};

	[Serializable]
	public ref class SensorMetaData : MetaData
	{
		public:		
		   System::Collections::Generic::List<Channel^>^ Channels;
	};

	public ref class SSIReader
	{
	//public: static System::Collections::Generic::List<MetaData^>^ Data = gcnew System::Collections::Generic::List<MetaData^>();
		public: static System::Collections::Generic::List<String^>^ Data = gcnew System::Collections::Generic::List<String^>();

	public: 
		
		SSIReader();
		~SSIReader();
		System::Collections::Generic::List<String^>^ ReadDll(String^ path);
		MetaData^ ReadMetaData(String^ name);

	private:
		
		void ReadChannelData(ssi::ISensor& sensor, SensorMetaData^ data);
		FILE *m_log;
	};

	
}
