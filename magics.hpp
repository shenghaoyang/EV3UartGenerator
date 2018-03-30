/**
 * \file magics.hpp
 *
 * Magic values that are used to construct the body of a particular UART
 * message.
 *
 * \copyright Shenghao Yang, 2018
 *
 * See LICENSE for more details
 */

/**
 * \page Magics
 *
 * There are a large number of magic values that specify that content of
 * messages that are sent and received from the EV3.
 *
 * In the library, these magic values are encapsulated into a set of
 * enumerations, contained in the file \ref magics.hpp
 *
 */

#ifndef MAGICS_HPP_
#define MAGICS_HPP_

#include <stdint.h> // We can't include <cstdint> if we want to compile under Arduino

namespace EV3UartGenerator {
namespace Magics {
	/**
	 * Magic values used in forming System messages for the EV3
	 *
	 * \c SYS_BASE is bitwise OR'd with other enumeration values in this
	 * enumeration in order to create the magic byte identifying the type
	 * of message.
	 */
	enum class SYS : uint8_t {
		SYS_BASE = 0x00, ///< Base magic value for SYS messages
		SYNC = 0x00, ///< Sync message magic value - used for clock syncs
		NACK = 0x02, ///< NACK message magic value
		ACK =  0x04, ///< ACK message magic value
		ESC =  0x06, ///< ESC message magic value - currently unused
	};

	/**
	 * Magic values used in forming Command messages for the EV3
	 *
	 * \c CMD_BASE is bitwise OR'd with other enumeration values in this
	 * enumeration, as well as the length of the payload,
	 *  in order to create the magic byte identifying the
	 * type of message.
	 */
	enum class CMD : uint8_t {
		CMD_BASE = 0x40,///< Base magic value for CMD messages
		TYPE = 0x00,	///< Type message magic value - used in messages specifying device type to the EV3
		MODES = 0x01,	///< Modes message magic value - used in messages specifying number of device modes to the EV3
		SPEED = 0x02,   ///< Speed message magic value - used in messages specifying the maximum baud rate supported by a device
		SELECT = 0x03,  ///< Select message magic value - used in messages requesting a device to operate in a particular mode
		WRITE = 0x04,	///< Write message magic value - used in messages sending data from the EV3 to the device
	};

	/**
	 * Magic values used in forming Information messages for the EV3
	 *
	 * \c INFO_BASE is bitwise OR'd with the length of the payload
	 * as well as the mode number the information message is for to
	 * create the magic byte identifying the type of message.
	 */
	enum class INFO : uint8_t {
		INFO_BASE = 0x80, ///< Base magic value for INFO messages
	};

	/**
	 * Span type magic values used in forming Information messages for the
	 * EV3.
	 */
	enum class INFO_SPAN : uint8_t {
		RAW = 0x01,		  ///< Magic byte to indicate RAW span message
		PCT = 0x02,		  ///< Magic byte to indicate PCT span message
		SI = 0x03,		  ///< Magic byte to indicate SI span message
	};

	/**
	 * Data type magic values used in forming Information messages for the
	 * EV3.
	 */
	enum class INFO_DTYPE : uint8_t {
		S8 = 0x00,	 	  ///< Magic byte to indicate 8 bit signed integers
		S16 = 0x01,		  ///< Magic byte to indicate 16 bit signed integers
		S32 = 0X02,		  ///< Magic byte to indicate 32 bit signed integers
		F32 = 0x03,		  ///< Magic byte to indicate 32 bit IEEE floating points
	};

	/**
	 * Magic values used in forming Data messages for the EV3
	 *
	 * \c DATA_BASE is bitwise OR'd with the length of the payload
	 * as well as the mode number the data message is for to create the
	 * magic byte identifying the type of message.
	 */
	enum class DATA : uint8_t {
		DATA_BASE = 0xc0, ///< Base magic value for DATA messages
	};
}
}



#endif /* MAGICS_HPP_ */
