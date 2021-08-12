#include "Shimmer3LogAndStreamDevice.h"

namespace ssi {
	namespace shimmer3 {


		template <typename T>
		T twos_complement(T val,
			// "allow this template to be instantiated only for unsigned types"
			typename std::enable_if<std::is_unsigned<T>::value>::type* = 0)
		{
			return -std::uintmax_t(val);
		}

		/// converts the raw byte representation of a shimmer3 sensor value to a long
		/// NOTE: this is a best effort copy of the conversion logic found in the Shimmer C# API
		/// Since there is a lot of bit shuffling and big/little endian conversion it might not be completely correct!
		long convert(const unsigned char* dataStart, const SensorSampleInfo& sampleInfo) {
			long returnValue = 0;

			switch (sampleInfo.representedType)
			{
			case SENSORDATATYPE::u8:
				returnValue = static_cast<long>(*reinterpret_cast<const uint8_t*>(dataStart));
				break;
			case SENSORDATATYPE::u12:
				if (sampleInfo.isLittleEndian) {
					//byte @dataStart + ((byte @dataStart+1) << 8) => [(dataStart+1)(dataStart)]
					uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart));
					uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart + 1)) << 8; //should the 4 most significant bits be masked out? Or do we assume they are always 0?
					returnValue = static_cast<long>(msb + lsb);
				}
				else {
					//byte @dataStart+1 + ((byte @dataStart) << 8) => [(dataStart)(dataStart+1)]
					uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart + 1)) << 8;
					uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart));
					returnValue = static_cast<long>(msb + lsb);
				}
				break;
			case SENSORDATATYPE::i16:
			// scope local variables to this case only otherwise compiler will cry out on "redefinitions" for variables with the same names in later scopes
			{ 
				uint16_t combined;
				if (sampleInfo.isLittleEndian) {
					//byte @dataStart + ((byte @dataStart+1) << 8) => [(dataStart+1)(dataStart)]
					uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart));
					uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart + 1)) << 8;
					combined = msb + lsb;
				}
				else {
					//byte @dataStart+1 + ((byte @dataStart) << 8) => [(dataStart)(dataStart+1)]
					uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart + 1)) << 8;
					uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart));
					combined = msb + lsb;
				}
				//take care of the fact that i16 is a signed value
				bool inversionNeeded = (combined >> 15) == 1;
				returnValue = static_cast<long>((inversionNeeded ? twos_complement<uint16_t>(combined) : combined));
			}
			break;
			case SENSORDATATYPE::u16:
			{
				uint16_t lsb, msb;
				if (sampleInfo.isLittleEndian) {
					//byte @dataStart + ((byte @dataStart+1) << 8) => [(dataStart+1)(dataStart)]
					lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart));
					msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart + 1)) << 8;
				}
				else {
					//byte @dataStart+1 + ((byte @dataStart) << 8) => [(dataStart)(dataStart+1)]
					lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart + 1)) << 8;
					msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataStart));
				}
				returnValue = static_cast<long>(msb + lsb);
			}
			break;
			case SENSORDATATYPE::u24:
			{
				uint8_t firstByte = *reinterpret_cast<const uint8_t*>(dataStart);
				uint8_t secondByte = *reinterpret_cast<const uint8_t*>(dataStart + 1);
				uint8_t thirdByte = *reinterpret_cast<const uint8_t*>(dataStart + 2);
				uint32_t xmsb, msb, lsb;
				if (sampleInfo.isLittleEndian) {
					xmsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF));
				}
				else {
					xmsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF));
				}
				returnValue = xmsb + msb + lsb;
			}
			break;
			case SENSORDATATYPE::i24:
			{
				uint8_t firstByte = *reinterpret_cast<const uint8_t*>(dataStart);
				uint8_t secondByte = *reinterpret_cast<const uint8_t*>(dataStart + 1);
				uint8_t thirdByte = *reinterpret_cast<const uint8_t*>(dataStart + 2);
				uint32_t xmsb, msb, lsb;
				if (sampleInfo.isLittleEndian) {
					xmsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF));
				}
				else {
					xmsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF));
				}
				//take care of the fact that i24 is a signed value
				uint32_t combined = xmsb + msb + lsb;
				bool inversionNeeded = (combined >> 23) == 1;
				returnValue = static_cast<long>((inversionNeeded ? twos_complement<uint32_t>(combined) : combined));
			}
			break;
			default:
				break;
			}

			return returnValue;
		}

		long LogAndStreamDevice::DataPacket::get(const SENSORID& sensor) const {
			auto& offsetIterator = device.m_sensorOffsetWithinPacketMap.find(sensor);
			auto& sampleInfoIt = LogAndStreamDevice::sensorSampleInfoMap.find(sensor);

			//check if sensor is actually available in the current configuration
			if (offsetIterator == device.m_sensorOffsetWithinPacketMap.end() || sampleInfoIt == LogAndStreamDevice::sensorSampleInfoMap.end()) {
				throw std::exception("this sensor is not supported");
			}

			const auto& offset = offsetIterator->second;
			const auto& sampleInfo = sampleInfoIt->second;

			if ((offset + sampleInfo.nrOfBytes) > rawData.size()) {
				throw std::exception("The value is represented by more bytes than are available in the raw data packet");
			}

			const unsigned char* dataPtr = rawData.data() + offset;

			return convert(dataPtr, sampleInfo);
		}

		long LogAndStreamDevice::DataPacket::getRawTimestamp() const {
			//the timestamp is always stored in the first 3 bytes of the package
			return convert(rawData.data(), { 3, SENSORDATATYPE::u24, false });
		}
	} //namespace shimmer3
} //namespace ssi