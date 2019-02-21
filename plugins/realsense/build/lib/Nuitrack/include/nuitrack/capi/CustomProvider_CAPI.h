#ifndef NUITRACK_CUSTOMPROVIDER_CAPI_H_
#define NUITRACK_CUSTOMPROVIDER_CAPI_H_

#include <stdint.h>

#include "nuitrack/types/Export.h"

extern "C" void NUITRACK_API nuitrack_sendData(uint8_t dataType, uint32_t width, uint32_t height, uint8_t* data, uint8_t pixelType, uint64_t timestamp);

#endif /* NUITRACK_CUSTOMPROVIDER_CAPI_H_ */
