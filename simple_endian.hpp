/**
 * \file simple_endian.hpp
 *
 * Contains simple endianess conversion functions necessary for the EV3
 * UART sensor protocol library.
 *
 * \copyright Shenghao Yang, 2018
 *
 * See LICENSE for more details
 */

#ifndef SIMPLE_ENDIAN_HPP
#define SIMPLE_ENDIAN_HPP

#include <stdint.h> // We can't include <cstdint> if we want to compile under Arduino

#ifndef __BYTE_ORDER__
#error "__BYTE_ORDER__ is not defined by your system - are you compiling under GCC?"
#endif

namespace EV3UartGenerator {
namespace SimpleEndian {
uint32_t htole32(uint32_t host_32bits) {
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	return host_32bits;
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	return __builtin_bswap32(host_32bits);
#elif (__BYTE_ORDER__ == __ORDER_PDP_ENDIAN__)
	return (((host_32bits & 0xffff0000) >> 0x10) | ((host_32bits & 0x0000ffff) << 0x10));
#elif 1
#error "__BYTE_ORDER__ currently defined is not supported - please submit a bug report"
#endif
}
}
}

#endif /* SIMPLE_ENDAIN_HPP */
