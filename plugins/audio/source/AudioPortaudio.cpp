#include "AudioPortaudio.h"
#if _WIN32||_WIN64
#include "PortAudio.h"
#include "graphic/DialogLibGateway.h"
#else
#include "portaudio.h"
#endif
#include "base/IProvider.h"
#include "thread/Lock.h"
#include "ioput/file/StringList.h"


#include <sstream>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *AudioPortaudio::ssi_log_name = "audioporta";

AudioPortaudio::AudioPortaudio(const ssi_char_t* file) 
	: _file(NULL), 
	_pData(NULL),  
	_offsets (0), 
	_selected (0), 
	_n_selected (0), 
	_n_max_channels (0), 
	_n_sel_channels (0),
	_max_selected (0),
	_dataProvider (0)
{
	if(file)
	{
		if(!OptionList::LoadXML(file, _options))
			OptionList::SaveXML(file, _options);
		_file = ssi_strcpy(file);
	}
}

AudioPortaudio::~AudioPortaudio()
{
	if(_file)
	{
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}

	delete[] _offsets;
	delete[] _selected;
}

bool AudioPortaudio::setProvider(const ssi_char_t *name, ssi::IProvider *provider)
{
	SSI_ASSERT(provider);

	if (_dataProvider) {
		ssi_wrn ("already set");
		return false;
	}

    if (strcmp (name, SSI_AUDIOPORTAUDIO_PROVIDER_NAME) != 0) {
		ssi_wrn ("unkown provider name %s", name);
		return false;
	}

	delete[] _selected;	
	_selected = 0;
	_n_sel_channels = 0;
	_max_selected = 0;

	if (Parse (_options.channels)) {
		
		//generate offset values
		_offsets = new int[_n_selected+1];
		_n_sel_channels = _n_selected;

		_offsets[0] = _selected[0];
		_max_selected = _selected[0];
		for(ssi_size_t i = 1; i < _n_selected; i++) {
			_offsets[i] = (_selected[i] - _selected[i-1]); 	
			if (_selected[i] > _max_selected) {
				_max_selected = _selected[i];
			}
		}

		//leave space for one extra offset to close the gap to the next sample
		_offsets[_n_selected] = 0;
		
	} else {
		ssi_wrn ("could not parse channel list");
	}

	_dataProvider = provider;
	_audio_channel.stream.sr = _options.sr;
	_audio_channel.stream.dim = _n_sel_channels;
	_dataProvider->init (&_audio_channel);
	
	return true;
}

bool AudioPortaudio::connect()
{
	ssi_msg(SSI_LOG_LEVEL_DETAIL, "try to connect sensor...");

	_frame_size = _options.block * _n_sel_channels;

	_pData = new AudioPortaudioData;
	_pData->Instance = this;
	_pData->Data = new float[_frame_size];
	
	PaError status;
	if((status = Pa_Initialize()) != paNoError)
	{
		ssi_wrn("Could not initialize portaudio: %s", Pa_GetErrorText(status));
		return false;
	}

	int cDevices = Pa_GetDeviceCount();

	PaStreamParameters params = { NULL };
	int device_id = -1;
	if (_options.device[0] == '\0') {
		device_id = selectDevice ();
	} else {
		device_id = getDevice (_options.device);
	}
	params.device = device_id == -1 ? Pa_GetDefaultInputDevice() : device_id;	

	PaDeviceInfo const* info = Pa_GetDeviceInfo(params.device);
	_n_max_channels = info->maxInputChannels;

	if (_max_selected >= _n_max_channels) {
		ssi_wrn ("#selected channels '%d' exceeds #available channels '%d'", _max_selected, _n_max_channels);
		return false;
	}

	params.channelCount = _n_max_channels;
	params.sampleFormat = paFloat32;
	params.suggestedLatency = Pa_GetDeviceInfo( params.device )->defaultLowInputLatency;
	params.hostApiSpecificStreamInfo = NULL;

	// now that we know the number of channels we can calculate the offeset to the next sample
	_offsets[_n_selected] = (_n_max_channels - _selected[_n_selected-1]);

	status = Pa_OpenStream(&_stream, &params, NULL, _options.sr,
		_options.block, paClipOff, &AudioPortaudio::PaStreamCallback, _pData);


	if(status != paNoError)
	{
		ssi_wrn("Could not open stream: %s", Pa_GetErrorText(status));
		return false;
	}

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");
	
	if(_dataProvider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL)
	{
		ssi_print ("\
             device\t= %s\n\
             rate\t= %.2lf\n\
             block\t= %d\n\
             dim\t= %d\n\
             bytes\t= %d\n",
			 _options.device,
		     _options.sr, 
			 _options.block,
			 _n_sel_channels, 
			 sizeof(float));		
	}

	return true;
}

bool AudioPortaudio::start () {

	PaError status = Pa_StartStream(_stream);
	if(status != paNoError)
	{
		ssi_wrn("Could not start stream: %s", Pa_GetErrorText(status));
		Pa_Terminate();
		return false;
	}

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "start streaming");

	return true;
}

bool AudioPortaudio::stop () {

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "stop streaming");

	PaError status;
	status = Pa_StopStream(_stream);
	if(status != paNoError)
	{
		ssi_wrn("Disconnect failed: %s", Pa_GetErrorText(status));
		Pa_Terminate();
		return false;
	}

	Pa_Terminate();

	return true;
}

bool AudioPortaudio::disconnect()
{		

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "disconnect");

	if(_pData != NULL)
	{
		delete[] _pData->Data;
		delete _pData;
		_pData = NULL;
	}

	return true;
}

bool AudioPortaudio::supportsReconnect()
{
	return false;
}

int AudioPortaudio::VPaStreamCallback(const void *input, void *output, unsigned long frameCount, 
								 const PaStreamCallbackTimeInfo *timeInfo, 
								 PaStreamCallbackFlags statusFlags, void *userData)
{
	float* pData = _pData->Data;
	float const* pInput = ssi_pcast(const float, input);
	
	for (ssi_size_t i = 0; i < frameCount; i++) {
		for (ssi_size_t j = 0; j < _n_selected; j++) {
			pInput += _offsets[j];
			*pData++ = *pInput;		
		}
		pInput += _offsets[_n_selected];
	}

	_dataProvider->provide(ssi_pcast(ssi_byte_t, _pData->Data), _options.block);

	return paContinue;
}

int AudioPortaudio::PaStreamCallback(const void *input, void *output, unsigned long frameCount, 
								const PaStreamCallbackTimeInfo *timeInfo, 
								PaStreamCallbackFlags statusFlags, void *userData)
{
	PAudioPortaudioData pData = ssi_pcast(AudioPortaudioData, userData);
	return pData->Instance->VPaStreamCallback(input, output, frameCount, timeInfo, statusFlags, userData);	
}

bool AudioPortaudio::Parse (const ssi_char_t *indices) {
	
	if (!indices || indices[0] == '\0') {
		return false;
	}

	ssi_char_t string[SSI_MAX_CHAR];
	
	char *pch;
	strcpy (string, indices);
	pch = strtok (string, ", ");
	int index;

	std::vector<int> items;
	
	while (pch != NULL) {
		index = atoi (pch);
		items.push_back(index);
		pch = strtok (NULL, ", ");
	}

	_n_selected = (ssi_size_t)items.size();
	_selected = new int[_n_selected];

	for(size_t i = 0; i < items.size(); i++)
		_selected[i] = items[i];		

	return true;
}


int AudioPortaudio::selectDevice () {

	StringList devices;
	PaDeviceIndex n_devices = Pa_GetDeviceCount ();
	int *map = new int[n_devices];
	int map_count = 0;

	//check if we have to restrict api choice
	bool usePreferredApi = false;
	if (_options.api > 0)
	{
		usePreferredApi = numDevicesForApi(_options.api) > 0;
		if (!usePreferredApi)
			ssi_wrn("found no devices for preferred api");
	}


	const PaDeviceInfo *device;
	const PaHostApiInfo *host;
	for (PaDeviceIndex i = 0; i < n_devices; i++) {		

		device = Pa_GetDeviceInfo (i);
		host = Pa_GetHostApiInfo(device->hostApi);

		if (device->maxInputChannels > 0 && (!usePreferredApi || _options.api == host->type)) {

			std::stringstream name;
			name << host->name << " - " << device->name;

			devices.add (name.str().c_str());	
			map[map_count++] = i;
		}
	}

	int id = LetUserSelectDevice (devices);
	int result = 0;
	if (id >= 0) {
		if (_options.remember) {
			_options.setDevice (devices.get (id));
		}
		result = map[id];
	} else {
		ssi_wrn ("could not select a valid device");
	}

	delete[] map;

	return result;
}

int AudioPortaudio::getDevice (const ssi_char_t *name) {

	PaDeviceIndex n_devices = Pa_GetDeviceCount ();

	const PaDeviceInfo *device;
	const PaHostApiInfo *host;
	for (PaDeviceIndex i = 0; i < n_devices; i++) {

		device = Pa_GetDeviceInfo(i);
		host = Pa_GetHostApiInfo(device->hostApi);

		std::stringstream name_dev;
		name_dev << host->name << " - " << device->name;

		if (strcmp(name, name_dev.str().c_str()) == 0) {
			return i;
		}
	}

	ssi_wrn ("could not find device '%s'", name);
	return -1;
}

unsigned int AudioPortaudio::numDevicesForApi(int api_type)
{
	PaDeviceIndex n_devices = Pa_GetDeviceCount();
	const PaDeviceInfo *device;
	unsigned int count = 0;

	for (PaDeviceIndex i = 0; i < n_devices; i++) {

		device = Pa_GetDeviceInfo(i);
		if (Pa_GetHostApiInfo(device->hostApi)->type == api_type && device->maxInputChannels > 0) {
			count++;
		}
	}

	return count;
}


int AudioPortaudio::LetUserSelectDevice (StringList &list) {

	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}
#if _WIN32||_WIN64
	DialogLibGateway dialogGateway;

	if(!dialogGateway.didInitWork())
	{
		return LetUserSelectDeviceOnConsole (list);
	}

	if(!dialogGateway.SetNewDialogType("SimpleSelectionDialog"))
	{
		return LetUserSelectDeviceOnConsole (list);
	}


	int intHandle = dialogGateway.AlterExistingItem ("Caption", -1, "Select an audio device (input)");

	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		intHandle = dialogGateway.AppendItem("Item", list.get (i));
		if(intHandle < 0) {
			return LetUserSelectDeviceOnConsole (list);
		}
	}

	return dialogGateway.RunDialog();
	#else
    return LetUserSelectDeviceOnConsole(list);

	#endif
}

int AudioPortaudio::LetUserSelectDeviceOnConsole(StringList &list) {

	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}

	ssi_print("Select an audio device:\n");
	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		ssi_print("%d: %s\n", i, list.get (i));
	}

	int selection = -1;
	ssi_print("Your selection: ");
	scanf("%d", &selection);
	if(selection == EOF || selection < 0 || selection > ssi_cast (int, list.size ())) {
		ssi_wrn_static ("invalid selection");
		return -1;
	}
	
	return (selection - 1);
}

}
