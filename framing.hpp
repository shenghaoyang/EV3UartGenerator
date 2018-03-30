/**
 * \file framing.hpp
 *
 * Functions that help to create buffers of data that can be sent directly
 * to the EV3.
 *
 * \copyright Shenghao Yang, 2018
 *
 * See LICENSE for more details
 */

/**
 * \page Framing
 *
 * The library includes various functions that can be used to populate
 * buffers that can be sent to the EV3 as UART protocol messages.
 *
 * By using named constants, and by providing the data to be contained in the
 * messages to the EV3, the framing functions can automatically calculate
 * the size of the message, include checksums and padding, and populate
 * user-provided buffers with the message bytes to be delivered to the EV3.
 *
 * The functions that are used to frame messages for transmission to the EV3
 * are declared in the file \ref framing.hpp
 *
 * \warning The library currently CANNOT correctly frame floating point numbers
 * for delivery to the EV3 on systems that do not represent floating point
 * numbers in IEEE format, and on systems that do not use little-endian
 * representation for these multi-byte values.
 */

#ifndef FRAMING_HPP_
#define FRAMING_HPP_

#include <magics.hpp>

namespace EV3UartGenerator {
namespace Framing {
	constexpr uint8_t BUFFER_MIN { 0x23 }; ///< Minimum size of the buffer (in bytes) that the user has to provide to each of the framing functions, to avoid any chance of a buffer overflow.
	constexpr uint8_t PAYLOAD_SENSOR_TO_EV3_MAX { 0x20 }; ///< Maximum size of any payload sent in the EV3 UART sensor protocol, in bytes, to the EV3
	constexpr uint8_t PAYLOAD_EV3_TO_SENSOR_MAX { 0x18 }; ///< Maximum size of any payload sent in the EV3 UART sensor protocol, in bytes, from the EV3 to the sensor.
	constexpr uint8_t PAYLOAD_MIN { 0x01 }; ///< Minimum size of any payload sent in the EV3 UART sensor protocol, regardless of direction, in bytes.
	constexpr uint8_t SYMBOL_MAX { 0x08 }; ///< Maximum length of the string representation (ASCII) of any symbol referencing a the SI unit used to represent the data output from a sensor, in a particular mode.
	/**
	 * Frame an EV3 system message.
	 *
	 * @param dest destination buffer
	 * @param sys_type type of the system message
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (unused)
	 */
	int8_t frame_sys_message(uint8_t* dest, Magics::SYS sys_type);

	/**
	 * Frame an EV3 command message, containing sensor type information
	 *
	 * @param dest destination buffer
	 * @param type sensor type index [0, 255]
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (unused)
	 */
	int8_t frame_cmd_type_message(uint8_t* dest, const uint8_t type);

	/**
	 * Frame an EV3 command message, containing sensor mode information.
	 *
	 * For sensor mode set upper bounds, an upper bound of n implies that modes
	 * n - 1, n - 2, n - 3 ... 0 are contained in the set of modes
	 * bounded by the upper bound.
	 *
	 * @param dest destination buffer
	 * @param modes upper bound of sensor modes in sensor mode set [0, 7]
	 * @param modes_visible upper bound of sensor modes in the set of sensor
	 * modes visible [0, 7] to the user
	 * @return  length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (not used)
	 */
	int8_t frame_cmd_modes_message(uint8_t* dest, const uint8_t modes,
			const uint8_t modes_visible);

	/**
	 * Frame an EV3 commmand message, containing UART baudrate information.
	 *
	 * @param dest destination buffer
	 * @param speed baudrate requested / maximum baudrate supported by the
	 * device.
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (not used)
	 */
	int8_t frame_cmd_speed_message(uint8_t* dest, const uint32_t speed);

	/**
	 * Frame an EV3 command message, selecting a particular mode of the sensor
	 *
	 * @param dest destination buffer
	 * @param mode mode to select [0, 7]
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (not used)
	 */
	int8_t frame_cmd_select_message(uint8_t* dest, const uint8_t mode);

	/**
	 * Frame an EV3 command message, writing arbitrary data to the sensor.
	 *
	 * @param dest destination buffer
	 * @param data data to write to the sensor
	 * @param len length of array of data to write to the sensor [1, 32]
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (length overrun / underrun)
	 */
	int8_t frame_cmd_write_message(uint8_t* dest, const uint8_t* data,
			const uint8_t len);

	/**
	 * Frame an EV3 information message, informing the EV3 of the mode name
	 * for a particular mode of the sensor.
	 *
	 * @param dest destination buffer
	 * @param mode mode index [0, 7]
	 * @param name mode name [1 to 32 ASCII byte characters in length]
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (length overrun / underrun / name == nullptr)
	 */
	int8_t frame_info_message_name(uint8_t* dest, const uint8_t mode,
			const char* name);

	/**
	 * Frame an EV3 information message, informing the EV3 of the span of
	 * values returned from this sensor, for different units of readings
	 * from the sensor, for a particular mode of the sensor
	 *
	 * This message is optional, and may be omitted during the initialization
	 * process, where mode information is sent from the sensor to the EV3.
	 *
	 * @param dest destination buffer
	 * @param mode mode index [0, 7]
	 * @param span_type type of span for which information is to be sent for
	 * @param lower lower bound of the span
	 * @param upper upper bound of the span
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (unused)
	 *
	 * \warning this function MUST NOT be used in environments where the
	 * representation of single precision floating point numbers does not
	 * follow the IEEE standard.
	 */
	int8_t frame_info_message_span(uint8_t* dest, const uint8_t mode,
			Magics::INFO_SPAN span_type,
			const float lower, const float upper);

	/**
	 * Frame an EV3 information message, informing the EV3 of the text
	 * representation of the symbol (unit) used to represent the SI
	 * unit that can be used to quantify readings from the sensor, for readings
	 * from the sensor coming from a particular mode index.
	 *
	 * @param dest destination buffer
	 * @param mode mode index [0 - 7]
	 * @param symbol symbol text representation string [1 to 8 ASCII bytes in
	 * length]
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (length overrun / underrun / symbol == nullptr)
	 */
	int8_t frame_info_message_symbol(uint8_t* dest, const uint8_t mode,
			const char* symbol);

	/**
	 * Frame an EV3 information message, informing the EV3 of the type and
	 * number of data elements contained in
	 * the data messages coming from the sensor, as well as the number of
	 * decimal places and width to use when displaying readings for
	 * the sensor, for a particular mode of the sensor.
	 *
	 * @param dest destination buffer
	 * @param mode mode index [0, 7]
	 * @param elems number of data elements in a DATA message, limited to:
	 * Data type | Number of data elements
	 * --------- | -----------------------
	 * S8		 | [1, 32]
	 * S16		 | [1, 16]
	 * Sr32		 | [1, 8]
	 * F32		 | [1, 8]
	 * @param data_type type of data elements
	 * @param width number of characters (including decimal separator) used
	 * to display readings from the sensor [0, 15]
	 * @param decimals number of characters after the decimal place used
	 * to display readings from the sensor [0, 15]
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (not used)
	 */
	int8_t frame_info_message_format(uint8_t* dest, const uint8_t mode,
			const uint8_t elems,
			Magics::INFO_DTYPE data_type, const uint8_t width,
			const uint8_t decimals);

	/**
	 * Frame an EV3 data message, containing information from a sensor
	 * to be sent to an EV3, for the sensor running in a particular mode
	 *
	 * @param dest destination buffer
	 * @param mode mode index [0, 7]
	 * @param data data to be sent
	 * @param len length of data to be sent [1, 32]
	 * @return length of framed message (written to the buffer), if positive.
	 * @retval -1 on error (length overrun)
	 */
	int8_t frame_data_message(uint8_t* dest, const uint8_t mode,
			const uint8_t* data,
			const uint8_t len);

	/**
	 * Calculates the checksum for an EV3 data message.
	 *
	 * @param buf source buffer
	 * @param len length of data message in the source buffer
	 * @return checksum of \c len bytes of data in the
	 * source buffer
	 */
	uint8_t checksum(const uint8_t* buf, const uint8_t len);

	/**
	 * Calculates the integer log_2 of a particular unsigned
	 * integer value, rounded up.
	 *
	 * @param val value to calculate log_2 for [1, 255]
	 * @return result of log_2 of \c val rounded up to the next
	 * biggest integer.
	 *
	 * @warning undefined for \c val of 0.
	 */
	constexpr uint8_t log2(uint8_t val) {
		return (0x1f - __builtin_clz(val)) +
				(val - (0x01 << (0x1f -__builtin_clz(val))) != 0 ? 1 : 0);
	}

	/**
	 * Calculates the value that is to be OR'd into the message type byte
	 * to represent the length of the payload in the message.
	 *
	 * @param len length of the payload [1, 32]
	 * @return value to be OR'd into the message type byte to represent
	 * the length of the payload in the message.
	 */
	constexpr uint8_t length_code(uint8_t len) {
		return log2(len) << 0x03;
	}

	/**
	 * Inserts padding bytes at the end of a payload segment, so that
	 * the size of the payload segment is a non-negative power of two.
	 *
	 * @param dest address of the byte right after the end of the
	 * payload segment
	 * @param len length of payload segment [1, 32]
	 * @return number of padding bytes written
	 */
	uint8_t insert_padding(uint8_t* dest, uint8_t len);
}
}

#endif /* FRAMING_HPP_ */
