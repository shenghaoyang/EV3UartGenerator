/**
 * \file framing.cpp
 *
 * Function definitions for functions in \ref framing.hpp
 *
 * \copyright Shenghao Yang, 2018
 *
 * See LICENSE for more details
 */

#include <simple_endian.hpp>
#include <framing.hpp>
#include <string.h> // Need to include bare string.h for compatibility with Arduino platforms

namespace EV3UartGenerator {
namespace Framing {
	int8_t frame_sys_message(uint8_t* dest, Magics::SYS sys_type) {
		*dest = (static_cast<uint8_t>(sys_type)
				| static_cast<uint8_t>(Magics::SYS::SYS_BASE));
		return 0x01;
	}

	int8_t frame_cmd_type_message(uint8_t* dest, const uint8_t type) {
		const uint8_t* orig_dest { dest };
		*(dest++) = (static_cast<uint8_t>(Magics::CMD::CMD_BASE)
				| static_cast<uint8_t>(Magics::CMD::TYPE)
				| length_code(0x01));
		*(dest++) = type;
		*dest = checksum(orig_dest, 0x02);
		return 0x03;
	}

	int8_t frame_cmd_modes_message(uint8_t* dest, const uint8_t modes,
			const uint8_t modes_available) {
		const uint8_t* orig_dest { dest };
		*(dest++) = (static_cast<uint8_t>(Magics::CMD::CMD_BASE)
				| static_cast<uint8_t>(Magics::CMD::MODES)
				| length_code(0x02));
		*(dest++) = (0x07 & modes);				// Mask out unused bits
		*(dest++) = (0x07 & modes_available);	// Mask out unused bits
		*dest = checksum(orig_dest, 0x03);
		return 0x04;
	}

	int8_t frame_cmd_speed_message(uint8_t* dest, const uint32_t speed) {
		const uint8_t* orig_dest { dest };
		*(dest++) = (static_cast<uint8_t>(Magics::CMD::CMD_BASE)
				| static_cast<uint8_t>(Magics::CMD::SPEED)
				| length_code(sizeof(speed)));
		uint32_t speed_conv = SimpleEndian::htole32(speed);
		memcpy(reinterpret_cast<void*>(dest),
				reinterpret_cast<const void*>(&speed_conv), sizeof(speed_conv));
		dest += 4;
		*dest = checksum(orig_dest, 0x05);
		return 0x06;
	}

	int8_t frame_cmd_select_message(uint8_t* dest, const uint8_t mode) {
		const uint8_t* orig_dest { dest };
		*(dest++) = (static_cast<uint8_t>(Magics::CMD::CMD_BASE)
				| static_cast<uint8_t>(Magics::CMD::SELECT)
				| length_code(sizeof(mode)));
		*(dest++) = (0x07 & mode); 				// Mask out unused bits
		*dest = checksum(orig_dest, 0x02);
		return 0x03;
	}

	int8_t frame_cmd_write_message(uint8_t* dest, const uint8_t* data,
			const uint8_t len) {
		if ((len < PAYLOAD_MIN) || (len > PAYLOAD_EV3_TO_SENSOR_MAX)) {
			return -1; // Payload size doesn't fall into limits
		} else {
			const uint8_t* orig_dest { dest };
			*(dest++) = (static_cast<uint8_t>(Magics::CMD::CMD_BASE)
						| static_cast<uint8_t>(Magics::CMD::WRITE)
						| length_code(len));
			memcpy(reinterpret_cast<void*>(dest),
					reinterpret_cast<const void*>(data), len);
			dest += len;
			const uint8_t padding { insert_padding(dest, len) };
			dest += padding;
			*dest = checksum(orig_dest, 0x01 + len + padding);
			return (0x02 + len + padding);
		}
	}

	int8_t frame_info_message_name(uint8_t* dest, const uint8_t mode,
			const char* name) {
		const size_t name_length { name != nullptr ? strlen(name) : 0 };
		if ((name_length < PAYLOAD_MIN) ||
				(name_length > PAYLOAD_SENSOR_TO_EV3_MAX)) {
			return -1; // Name length doesn't fall into limits
		} else {
			const uint8_t* orig_dest { dest };
			*(dest++) = (static_cast<uint8_t>(Magics::INFO::INFO_BASE)
						| (0x07 & mode)
						| length_code(name_length));
			*(dest++) = 0x00; // Special case for INFO messages - INFO type byte after type byte
			memcpy(reinterpret_cast<void*>(dest),
					reinterpret_cast<const void*>(name), name_length);
			dest += name_length;
			const uint8_t padding { insert_padding(dest, name_length) };
			dest += padding;
			*dest = checksum(orig_dest, 0x02 + name_length + padding);
			return (0x03 + name_length + padding);
		}
	}

	int8_t frame_info_message_span(uint8_t* dest, const uint8_t mode,
			Magics::INFO_SPAN span_type, const float lower, const float upper) {
		const uint8_t* orig_dest { dest };
		*(dest++) = (static_cast<uint8_t>(Magics::INFO::INFO_BASE)
					| (0x07 & mode)
					| length_code(sizeof(lower) + sizeof(upper)));
		*(dest++) = static_cast<uint8_t>(span_type); // Special case for INFO messages - INFO type byte after type byte

		uint32_t temp;	// Memcpying between two temporaries is needed in case float is not aligned the same as a 4-byte integer
		float tempf;

		memcpy(reinterpret_cast<void*>(&temp),
				reinterpret_cast<const void*>(&lower), sizeof(lower));
		temp = SimpleEndian::htole32(temp);
		memcpy(reinterpret_cast<void*>(&tempf),
				reinterpret_cast<const void*>(&temp), sizeof(temp));
		memcpy(reinterpret_cast<void*>(dest),
				reinterpret_cast<const void*>(&tempf), sizeof(tempf));
		dest += sizeof(tempf);

		memcpy(reinterpret_cast<void*>(&temp),
				reinterpret_cast<const void*>(&upper), sizeof(upper));
		temp = SimpleEndian::htole32(temp);
		memcpy(reinterpret_cast<void*>(&tempf),
				reinterpret_cast<const void*>(&temp), sizeof(temp));
		memcpy(reinterpret_cast<void*>(dest),
				reinterpret_cast<const void*>(&tempf), sizeof(tempf));
		dest += sizeof(tempf);

		*dest = checksum(orig_dest, 0x0a);
		return 0x0b;
	}

	int8_t frame_info_message_symbol(uint8_t* dest, const uint8_t mode,
			const char* symbol) {
		const size_t symbol_length { symbol != nullptr ? strlen(symbol) : 0 };
		if ((symbol_length < PAYLOAD_MIN) ||
				(symbol_length > SYMBOL_MAX)) {
			return -1; // Name length doesn't fall into limits
		} else {
			const uint8_t* orig_dest { dest };
			*(dest++) = (static_cast<uint8_t>(Magics::INFO::INFO_BASE)
						| (0x07 & mode)
						| length_code(0x08)); // Length hardcoded to 8
			*(dest++) = 0x04; // Special case for INFO messages - INFO type byte after type byte
			memcpy(reinterpret_cast<void*>(dest),
					reinterpret_cast<const void*>(symbol), symbol_length);
			dest += symbol_length;

			const uint8_t padding { 0x08 - symbol_length };
			for (uint8_t i = 0; i < padding; i++)
				*(dest++) = 0x00;

			*dest = checksum(orig_dest, 0x02 + symbol_length + padding);
			return (0x03 + symbol_length + padding);
		}
	}

	int8_t frame_info_message_format(uint8_t* dest, const uint8_t mode,
				const uint8_t elems,
				Magics::INFO_DTYPE data_type, const uint8_t width,
				const uint8_t decimals) {
		const uint8_t* orig_dest { dest };
		*(dest++) = (static_cast<uint8_t>(Magics::INFO::INFO_BASE)
					| (0x07 & mode)
					| length_code(0x04));
		*(dest++) = 0x80; // Special case for INFO messages - INFO type byte after type byte
		*(dest++) = (0x3f & elems);
		*(dest++) = (0x03 & static_cast<uint8_t>(data_type));
		*(dest++) = (0x0f & width);
		*(dest++) = (0x0f & decimals);
		*dest = checksum(orig_dest, 0x02 + 0x04);
		return (0x07);
	}

	int8_t frame_data_message(uint8_t* dest, const uint8_t mode,
			const uint8_t* data, const uint8_t len) {
		if ((len < PAYLOAD_MIN) || (len > PAYLOAD_SENSOR_TO_EV3_MAX)) {
			return -1;
		} else {
			const uint8_t* orig_dest { dest };
			*(dest++) = (static_cast<uint8_t>(Magics::DATA::DATA_BASE)
						| (0x07 & mode)
						| length_code(len));
			memcpy(reinterpret_cast<void*>(dest),
					reinterpret_cast<const void*>(data), len);
			dest += len;
			const uint8_t padding { insert_padding(dest, len) };
			dest += padding;
			*dest = checksum(orig_dest, 0x01 + len + padding);
			return (0x02 + len + padding);
		}
	}

	uint8_t checksum(const uint8_t* buf, const uint8_t len) {
		uint8_t acc { 0xff };
		for (uint8_t i = 0; i < len; i++) {
			acc ^= buf[i];
		}
		return acc;
	}

	uint8_t insert_padding(uint8_t* dest, uint8_t len) {
		const uint8_t padding = { ((0x01 << log2(len)) - len) };
		for (uint8_t i = 0; i < padding; i++) {
			*(dest++) = 0x00;
		}
		return padding;
	}

}
}




