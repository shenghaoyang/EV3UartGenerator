/**
 * \file EV3UartGenerator.hpp
 *
 * \copyright Shenghao Yang
 *
 * See LICENSE for more details
 */

/**
 * \mainpage EV3UartGenerator Library
 *
 * This library contains some C++ routines that can be used to generate
 * bytes to be sent to the EV3 for use in the EV3 UART sensor protocol.
 *
 * For further information, please consult the following pages:
 * - \ref Framing
 * - \ref Magics
 *
 * For information on the EV3 UART protocol, users can visit:
 * - http://ev3.fantastic.computer/doxygen/UartProtocol.html (UART
 * Protocol Documentation)
 * - http://ev3.fantastic.computer/doxygen/UartDriver.html (UART handshake
 * sequence Documentation)
 *
 * Byte dumps of UART protocol communications from actual LEGO sensors
 * can be found under doc/reference_bitstreams/
 *
 * Currently, there exists only reference bitstreams for the three LEGO sensors
 * in the LEGO education base set:
 * - LEGO Color sensor
 * - LEGO Ultrasonic distance sensor
 * - LEGO Gyro sensor
 * More submissions for reference bitstreams are welcome.
 *
 * \warning This library currently relies heavily on GCC defined builtins
 * and macros, for bit operations, as well as endianess conversion operations.
 * When compiling on another compiler, do note possible compatibility issues.
 *
 * This library is licensed under the MIT license. See LICENSE for more details.
 */

#ifndef EV3UARTGENERATOR_HPP_
#define EV3UARTGENERATOR_HPP_

#include <framing.hpp>


#endif /* EV3UARTGENERATOR_HPP_ */
