// Dies ist die Haupt-DLL.

//#include "stdafx.h"

#include "SSIReader.h"
#include <ssi_cons.h>
#include <base/Factory.h>
#include <base/ISensor.h>
#include <base/IConsumer.h>


using namespace XMLReader;

SSIReader::SSIReader()
{
	System::Threading::Thread::CurrentThread->CurrentCulture = gcnew System::Globalization::CultureInfo("en-US");

	m_log = fopen("xmledit.log", "w");
}

SSIReader::~SSIReader()
{
	fclose(m_log);
}

System::Collections::Generic::List<String^>^ SSIReader::ReadDll(String^ path)
{
	fprintf(m_log, "LOAD DLL %s\n", path);

	Data->Clear();
		
	pin_ptr<const wchar_t> szPath = PtrToStringChars(path);					
	
	HMODULE hDll = LoadLibraryW(szPath);
	if(INVALID_HANDLE_VALUE == hDll)
		return Data;

	ssi::Factory::register_fptr_from_dll_t register_fptr = (ssi::Factory::register_fptr_from_dll_t) GetProcAddress (hDll, "Register");
	if (0 == register_fptr)
	{
		fprintf(m_log, "FAILED\n");
		FreeLibrary(hDll);
		return Data;
	}	  
	FreeLibrary(hDll);

	char* szDll = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(path);

	ssi::Factory::RegisterDLL(szDll, 0);
	ssi_size_t n_objects = 0;
	ssi_char_t **object_names = 0;
	n_objects = ssi::Factory::GetObjectNames (&object_names);
	if (n_objects > 0) {
		fprintf(m_log, "FOUND %u OBJECTS\n", n_objects);
		for (ssi_size_t i = 0; i < n_objects; i++) {
			fprintf (m_log, "\t%s\n", object_names[i]);
			Data->Add(gcnew String (object_names[i]));
			delete[] object_names[i];
		}
		delete[] object_names;
	}

	System::Runtime::InteropServices::Marshal::FreeHGlobal((System::IntPtr)(void*)szDll);

	fprintf(m_log, "DONE\n\n");

	return Data;
}

MetaData^ SSIReader::ReadMetaData(String^ name)
{
	fprintf(m_log, "LOAD OPTIONS %s\n", name);

	char* szName = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name);
	ssi::IObject* pObject = ssi::Factory::Create(szName, 0, true);
	System::Runtime::InteropServices::Marshal::FreeHGlobal((System::IntPtr)(void*)szName);

	MetaData^ tmp;

	if((unsigned char)pObject->getType() == SSI_SENSOR)
	{
		tmp = gcnew SensorMetaData();
		ssi::ISensor* pSensor = dynamic_cast<ssi::ISensor*>(pObject);

		ReadChannelData(*pSensor, (SensorMetaData^)tmp);
	}
	else
		tmp = gcnew MetaData();
	
	tmp->Name = gcnew String(name);
	tmp->Info = gcnew String(pObject->getInfo());
	
	tmp->Options = gcnew System::Collections::Generic::List<Option^>();
	tmp->Type = (unsigned char)pObject->getType();
	
	ssi::IOptions* pOptions = pObject->getOptions();

	if(pOptions != 0)
	{
		fprintf(m_log, "FOUND %u OPTIONS\n", pOptions->getSize());

		for(ssi_size_t i = 0; i < pOptions->getSize(); ++i)
		{
			ssi_option_t* pOpt = pOptions->getOption(i);
			Option^ opt = gcnew Option();  				

			opt->Name = gcnew String(pOpt->name);
			opt->Help = gcnew String(pOpt->help);
			opt->Num = pOpt->num;
			opt->Type = (unsigned char)pOpt->type;
			//opt->pValue = pOpt->ptr;
			switch(pOpt->type)
			{
			case SSI_CHAR:
				{
					if (pOpt->num == 1)
					{
						opt->Value = System::Convert::ToString(*(char*)pOpt->ptr);
					}
					else
					{
						opt->Value = gcnew String(reinterpret_cast<const char*>(pOpt->ptr));
					}
				}
				break;
			case SSI_UCHAR:
				{
					if(pOpt->num == 1)
					{
						opt->Value = System::Convert::ToString(*(unsigned char*)pOpt->ptr);
					}
					else
					{
						opt->Value = gcnew String(reinterpret_cast<const char*>(pOpt->ptr));
					}
				}
				break;
			case SSI_SHORT:
				{
					short *value = new short[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				
				break;
			case SSI_USHORT:
				{
					unsigned short *value = new unsigned short[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;
			case SSI_INT:
				{					
					int *value = new int[pOpt->num];					
					pOptions->getOptionValue(pOpt->name, (void*)value);					
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;					
				}
				break;
			case SSI_UINT:
				{
					unsigned int *value = new unsigned int[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;
			case SSI_LONG:
				{
					long *value = new long[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;
			case SSI_ULONG:
				{
					unsigned long *value = new unsigned long[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString((unsigned __int64)value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString((unsigned __int64)value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;
			case SSI_FLOAT:
				{
					float *value = new float[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;
			case SSI_DOUBLE:
				{
					double *value = new double[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;
			case SSI_LDOUBLE:
				{
					long double *value = new long double[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString((double) value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString((double)value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;
			case SSI_STRUCT:
			case SSI_IMAGE:
				break;
			case SSI_BOOL:
				{
					bool *value = new bool[pOpt->num];
					pOptions->getOptionValue(pOpt->name, (void*)value);
					System::String^ string;
					string = System::Convert::ToString(value[0]);
					for (ssi_size_t j = 1; j < pOpt->num; j++) {
						string += ",";
						string += System::Convert::ToString(value[j]);
					}
					opt->Value = string;
					delete[] value;
				}
				break;			
			};
			
			tmp->Options->Add(opt);
			fprintf(m_log, "\t%s\n", opt->Name);
		}
	}

	delete pObject;

	fprintf(m_log, "DONE\n\n");
	
	return tmp;
}

void SSIReader::ReadChannelData(ssi::ISensor& sensor, SensorMetaData^ data)
{
	size_t cChannels = sensor.getChannelSize();
	data->Channels = gcnew System::Collections::Generic::List<Channel^>();

	for(ssi_size_t i = 0; i < cChannels; ++i)
	{
		ssi::IChannel* pChannel = sensor.getChannel(i);
		const char* name = pChannel->getName();
		const char* info = pChannel->getInfo();

		Channel^ channel = gcnew Channel();
		channel->Name = gcnew String(name);
		channel->Info = gcnew String(info);

		data->Channels->Add(channel);
	}
}

