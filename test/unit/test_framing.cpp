/**
 * \file test_framing.cpp
 *
 * Tests for the framing portion of EV3UartGenerator.
 *
 * \copyright Shenghao Yang, 2018
 * 
 * See LICENSE for details
 */

#include <framing.hpp>
#include "catch.hpp"
#include <endian.h>
#include <array>
#include <numeric>
#include <cstring>
#include <cmath>

TEST_CASE("System messages are correctly framed", "[frame] [sys]") {
	using namespace EV3UartGenerator;
	std::array<uint8_t, 1> buffer { };
	buffer.fill(0xff);

	SECTION("ACK messages are correctly framed"){
	int8_t s = Framing::frame_sys_message(buffer.data(),
			Magics::SYS::ACK);
	REQUIRE(buffer[0] == (static_cast<uint8_t>(Magics::SYS::SYS_BASE)
					| static_cast<uint8_t>(Magics::SYS::ACK)));
	REQUIRE(s == 0x01);
}

	SECTION("NACK messages are correctly framed"){
	int8_t s = Framing::frame_sys_message(buffer.data(),
			Magics::SYS::NACK);
	REQUIRE(buffer[0] == (static_cast<uint8_t>(Magics::SYS::SYS_BASE)
					| static_cast<uint8_t>(Magics::SYS::NACK)));
	REQUIRE(s == 0x01);
}

	SECTION("SYNC messages are correctly framed"){
	int8_t s = Framing::frame_sys_message(buffer.data(),
			Magics::SYS::SYNC);
	REQUIRE(buffer[0] == (static_cast<uint8_t>(Magics::SYS::SYS_BASE)
					| static_cast<uint8_t>(Magics::SYS::SYNC)));
	REQUIRE(s == 0x01);
}
}

TEST_CASE("Command messages are correctly framed", "[frame] [cmd]") {
	using namespace EV3UartGenerator;
	std::array<std::uint8_t, Framing::BUFFER_MIN> buffer { };
	buffer.fill(0xff);

	SECTION("TYPE messages are correctly framed"){
	for (uint16_t type = 0; type < 0x100; type++) {
		int8_t s = Framing::frame_cmd_type_message(buffer.data(),
				type);
		REQUIRE(buffer[0] == ((static_cast<uint8_t>(Magics::CMD::CMD_BASE))
						| static_cast<uint8_t>(Magics::CMD::TYPE)
						| Framing::length_code(1)));
		REQUIRE(buffer[1] == type);
		REQUIRE(buffer[2] == Framing::checksum(buffer.data(), 0x02));
		REQUIRE(s == 0x03);
	}
}

	SECTION("MODES messages are correctly framed"){
	for (uint16_t modes = 0; modes < 0x100; modes++) {
		for (uint16_t modes_visible = 0; modes_visible < 0x100;
				modes_visible++) {
			int8_t s = Framing::frame_cmd_modes_message(buffer.data(),
					modes, modes_visible);
			REQUIRE(buffer[0] == ((static_cast<uint8_t>(Magics::CMD::CMD_BASE))
							| static_cast<uint8_t>(Magics::CMD::MODES)
							| Framing::length_code(2)));
			REQUIRE(buffer[1] == (modes & 0x07));
			REQUIRE(buffer[2] == (modes_visible & 0x07));
			REQUIRE(buffer[3] == Framing::checksum(buffer.data(), 0x03));
			REQUIRE(s == 0x04);
		}
	}
}

	SECTION("SPEED messages are correctly framed"){
	uint32_t speed = 0xdeadbeef;
	int8_t s = Framing::frame_cmd_speed_message(buffer.data(), speed);
	REQUIRE(buffer[0] == ((static_cast<uint8_t>(Magics::CMD::CMD_BASE))
					| static_cast<uint8_t>(Magics::CMD::SPEED)
					| Framing::length_code(4)));

	// Check that speed is encoded in Little-Endian
	REQUIRE(buffer[1] == (speed & 0xff));
	REQUIRE(buffer[2] == ((speed >> 0x08) & 0xff));
	REQUIRE(buffer[3] == ((speed >> 0x10) & 0xff));
	REQUIRE(buffer[4] == ((speed >> 0x18) & 0xff));

	REQUIRE(buffer[5] == Framing::checksum(buffer.data(), 0x05));
	REQUIRE(s == 0x06);
}

	SECTION("SELECT messages are correctly framed"){
	for (uint16_t mode = 0; mode < 0x100; mode++) {
		int8_t s = Framing::frame_cmd_select_message(buffer.data(),
				mode);
		REQUIRE(buffer[0] == ((static_cast<uint8_t>(Magics::CMD::CMD_BASE))
						| static_cast<uint8_t>(Magics::CMD::SELECT)
						| Framing::length_code(1)));
		REQUIRE(buffer[1] == (mode & 0x07));
		REQUIRE(buffer[2] == Framing::checksum(buffer.data(), 0x02));
		REQUIRE(s == 0x03);
	}
}

	SECTION("WRITE messages are correctly framed"){
	SECTION("Payloads with invalid size are discarded and correct"
			" byte counts are returned") {
		for (uint16_t sz = 0; sz < 0x100; sz++) {
			uint8_t payload[sz];
			if (sz < Framing::PAYLOAD_MIN) {
				REQUIRE(Framing::frame_cmd_write_message(buffer.data(),
								payload, sz) == -1);
			} else if (sz > Framing::PAYLOAD_EV3_TO_SENSOR_MAX) {
				REQUIRE(Framing::frame_cmd_write_message(buffer.data(),
								payload, sz) == -1);
			} else {
				REQUIRE(Framing::frame_cmd_write_message(buffer.data(),
								payload, sz) == ((0x01 << Framing::log2(sz))
								+ 0x02));
			}
		}
	}
	SECTION("Data in write messages are framed correctly in the buffer") {
		for (uint16_t sz = Framing::PAYLOAD_MIN;
				sz < Framing::PAYLOAD_EV3_TO_SENSOR_MAX;
				sz++) {
			uint8_t payload[sz];
			std::iota(payload, payload + sz, 0);
			Framing::frame_cmd_write_message(buffer.data(), payload, sz);

			REQUIRE(buffer[0] == ((
									static_cast<uint8_t>(Magics::CMD::CMD_BASE))
							| static_cast<uint8_t>(Magics::CMD::WRITE)
							| Framing::length_code(sz)));
			REQUIRE(std::equal(buffer.data() + 1, buffer.data() + 1 + sz,
							payload) == true);
			const uint8_t padding = ((0x01 << Framing::log2(sz)) - sz);
			for (uint8_t pad_count = 0; pad_count < padding; pad_count++) {
				REQUIRE(buffer[1 + sz + pad_count] == 0x00);
			}
			REQUIRE(buffer[1 + sz + padding] ==
					Framing::checksum(buffer.data(), 1 + sz + padding));
		}
	}
}
}

TEST_CASE("Information messages are correctly framed", "[frame] [info]") {
	using namespace EV3UartGenerator;
	std::array<std::uint8_t, Framing::BUFFER_MIN> buffer { };
	buffer.fill(0xff);
	SECTION("NAME messages are correctly framed"){
	SECTION("Invalid payloads are discarded and "
			"return values are correct") {
		SECTION("payloads pointed to by a nullptr are discarded") {
			for (uint16_t mode = 0; mode < 0x100; mode++)
			REQUIRE(Framing::frame_info_message_name(buffer.data(),
							mode, nullptr) == -1);
		}
		SECTION("payloads with invalid sizes are discarded and "
				"payloads with valid sizes give valid byte counts") {
			for (uint16_t mode = 0; mode < 0x100; mode++) {
				for (uint8_t sz = 0;
						sz < (Framing::PAYLOAD_SENSOR_TO_EV3_MAX + 10);
						sz++) {
					char payload[sz + 1];
					std::iota(payload, payload + sz, 'A');
					payload[sz] = '\0';
					if (sz < Framing::PAYLOAD_MIN) {
						REQUIRE(Framing::frame_info_message_name(
										buffer.data(), mode, payload) == -1);
					} else if (sz > Framing::PAYLOAD_SENSOR_TO_EV3_MAX) {
						REQUIRE(Framing::frame_info_message_name(
										buffer.data(), mode, payload) == -1);
					} else {
						REQUIRE(Framing::frame_info_message_name(
										buffer.data(), mode, payload) == (
										(0x01 << Framing::log2(sz)) + 0x03));
					}
				}
			}
		}
	}
	SECTION("Data in name messages are framed correctly in the buffer") {
		for (uint16_t mode = 0; mode < 0x100; mode++) {
			for (uint16_t sz = Framing::PAYLOAD_MIN;
					sz < Framing::PAYLOAD_EV3_TO_SENSOR_MAX;
					sz++) {
				char payload[sz + 1];
				std::iota(payload, payload + sz, 'A');
				payload[sz] = '\0';

				Framing::frame_info_message_name(buffer.data(), mode,
						payload);
				REQUIRE(buffer[0] == ((
										static_cast<uint8_t>(Magics::INFO::INFO_BASE))
								| (mode & 0x07)
								| Framing::length_code(sz)));
				REQUIRE(buffer[1] == 0x00);
				REQUIRE(std::equal(buffer.data() + 2, buffer.data() + 2 + sz,
								payload) == true);
				const uint8_t padding = ((0x01 << Framing::log2(sz)) - sz);
				for (uint8_t pad_count = 0; pad_count < padding; pad_count++) {
					REQUIRE(buffer[2 + sz + pad_count] == 0x00);
				}
				REQUIRE(buffer[2 + sz + padding] ==
						Framing::checksum(buffer.data(), 2 + sz + padding));
			}
		}
	}
}

	SECTION("SPAN messages are correctly framed"){
	for (uint16_t mode = 0; mode < 0x100; mode++) {
		for (uint8_t t = static_cast<uint8_t>(Magics::INFO_SPAN::PCT);
				t <= static_cast<uint8_t>(Magics::INFO_SPAN::SI);
				t++) {
			float lower = M_PI;
			float upper = M_E;

			int8_t s = Framing::frame_info_message_span(buffer.data(),
					mode, static_cast<Magics::INFO_SPAN>(t), lower, upper);
			REQUIRE(buffer[0] == ((
									static_cast<uint8_t>(Magics::INFO::INFO_BASE))
							| (mode & 0x07)
							| Framing::length_code(8)));
			REQUIRE(buffer[1] == t);

			// Compare LE
			uint32_t temp_ref;
			uint32_t temp_written;
			memcpy(reinterpret_cast<void*>(&temp_ref),
					reinterpret_cast<const void*>(&lower), sizeof(lower));
			memcpy(reinterpret_cast<void*>(&temp_written),
					reinterpret_cast<const void*>(&buffer[2]),
					sizeof(lower));
			REQUIRE(le32toh(temp_written) == temp_ref);
			memcpy(reinterpret_cast<void*>(&temp_ref),
					reinterpret_cast<const void*>(&upper), sizeof(upper));
			memcpy(reinterpret_cast<void*>(&temp_written),
					reinterpret_cast<const void*>(&buffer[6]),
					sizeof(upper));
			REQUIRE(le32toh(temp_written) == temp_ref);

			REQUIRE(buffer[10] == Framing::checksum(buffer.data(), 10));
			REQUIRE(s == 11);
		}
	}
}

	SECTION("SYMBOL messages are correctly framed"){
	SECTION("Invalid payloads are discarded and "
			"return values are correct") {
		SECTION("payloads pointed to by a nullptr are discarded") {
			for (uint16_t mode = 0; mode < 0x100; mode++)
			REQUIRE(Framing::frame_info_message_symbol(buffer.data(),
							mode, nullptr) == -1);
		}
		SECTION("payloads with invalid sizes are discarded and "
				"payloads with valid sizes give valid byte counts") {
			for (uint16_t mode = 0; mode < 0x100; mode++) {
				for (uint8_t sz = 0;
						sz < (Framing::PAYLOAD_SENSOR_TO_EV3_MAX + 10);
						sz++) {
					char payload[sz + 1];
					std::iota(payload, payload + sz, 'A');
					payload[sz] = '\0';
					if (sz < Framing::PAYLOAD_MIN) {
						REQUIRE(Framing::frame_info_message_symbol(
										buffer.data(), mode, payload) == -1);
					} else if (sz > Framing::SYMBOL_MAX) {
						REQUIRE(Framing::frame_info_message_symbol(
										buffer.data(), mode, payload) == -1);
					} else {
						REQUIRE(Framing::frame_info_message_symbol(
										buffer.data(), mode, payload) == (
										11));
					}
				}
			}
		}
	}
	SECTION("Data in symbol messages are framed correctly in the buffer") {
		for (uint16_t mode = 0; mode < 0x100; mode++) {
			for (uint16_t sz = Framing::PAYLOAD_MIN;
					sz < Framing::SYMBOL_MAX;
					sz++) {
				char payload[sz + 1];
				std::iota(payload, payload + sz, 'A');
				payload[sz] = '\0';

				Framing::frame_info_message_symbol(buffer.data(), mode,
						payload);
				REQUIRE(buffer[0] == ((
										static_cast<uint8_t>(Magics::INFO::INFO_BASE))
								| (mode & 0x07)
								| Framing::length_code(8)));
				REQUIRE(buffer[1] == 0x04);
				REQUIRE(std::equal(buffer.data() + 2, buffer.data() + 2 + sz,
								payload) == true);
				const uint8_t padding = (8 - sz);
				for (uint8_t pad_count = 0; pad_count < padding; pad_count++) {
					REQUIRE(buffer[2 + sz + pad_count] == 0x00);
				}
				REQUIRE(buffer[2 + sz + padding] ==
						Framing::checksum(buffer.data(), 2 + sz + padding));
			}
		}
	}
}

	SECTION("FORMAT messages are properly framed"){

	for (uint8_t dtype = static_cast<uint8_t>(Magics::INFO_DTYPE::S8);
			dtype <= static_cast<uint8_t>(Magics::INFO_DTYPE::F32);
			dtype++) {
		for (uint8_t mode = 0; mode < 0x08; mode++) {
			for (uint8_t elems = 0; elems < 0x20; elems++) {
				for (uint8_t width = 0; width < 0x10; width++) {
					for (uint8_t decimals = 0; decimals < 0x10; decimals++) {
						int8_t s = Framing::frame_info_message_format(buffer.data(),
								mode, elems, static_cast<Magics::INFO_DTYPE>(dtype),
								width, decimals);

						REQUIRE(buffer[0] == (
										static_cast<uint8_t>(Magics::INFO::INFO_BASE)
										| (mode & 0x07)
										| Framing::length_code(4)));
						REQUIRE(buffer[1] == 0x80);
						REQUIRE(buffer[2] == (elems & 0x3f));
						REQUIRE(buffer[3] == (dtype));
						REQUIRE(buffer[4] == (width & 0x0f));
						REQUIRE(buffer[5] == (decimals & 0x0f));
						REQUIRE(buffer[6] == Framing::checksum(buffer.data(), 0x06));

						REQUIRE(s == 7);
					}
				}
			}
		}
	}
}
}

TEST_CASE("DATA messages are correctly framed", "[frame] [data]") {
	using namespace EV3UartGenerator;
	std::array<std::uint8_t, Framing::BUFFER_MIN> buffer { };
	buffer.fill(0xff);

	SECTION("Payloads with invalid size are discarded and correct"
			" byte counts are returned") {
		for (uint16_t mode = 0; mode < 0x100; mode++) {
			for (uint16_t sz = 0; sz < 0x100; sz++) {
				uint8_t payload[sz];
				if (sz < Framing::PAYLOAD_MIN) {
					REQUIRE(Framing::frame_data_message(buffer.data(),
							mode, payload, sz) == -1);
				} else if (sz > Framing::PAYLOAD_SENSOR_TO_EV3_MAX) {
					REQUIRE(Framing::frame_data_message(buffer.data(),
							mode, payload, sz) == -1);
				} else {
					REQUIRE(Framing::frame_data_message(buffer.data(),
							mode, payload, sz) == ((0x01 << Framing::log2(sz))
							+ 0x02));
				}
			}
		}
	}
	SECTION("Data in data messages are framed correctly in the buffer") {
		for (uint16_t mode = 0; mode < 0x100; mode++) {
		for (uint16_t sz = Framing::PAYLOAD_MIN;
				sz < Framing::PAYLOAD_EV3_TO_SENSOR_MAX;
				sz++) {
			uint8_t payload[sz];
			std::iota(payload, payload + sz, 0);
			Framing::frame_data_message(buffer.data(), mode, payload, sz);
			REQUIRE(buffer[0] == (static_cast<uint8_t>(Magics::DATA::DATA_BASE)
								  | (mode & 0x07)
								  | Framing::length_code(sz)));
			REQUIRE(std::equal(buffer.data() + 1, buffer.data() + 1 + sz,
							payload) == true);
			const uint8_t padding = ((0x01 << Framing::log2(sz)) - sz);
			for (uint8_t pad_count = 0; pad_count < padding; pad_count++) {
				REQUIRE(buffer[1 + sz + pad_count] == 0x00);
			}
			REQUIRE(buffer[1 + sz + padding] ==
					Framing::checksum(buffer.data(), 1 + sz + padding));
		}
		}
	}
}

TEST_CASE("checksum() returns correct results", "[frame] [checksum()]") {
	using namespace EV3UartGenerator;
	std::array<uint8_t, 0xff> buffer {};
	std::iota(buffer.begin(), buffer.end(), 0);

	for (uint16_t i = 0; i < 0x100; i++) {
		uint8_t checksum_ref = 0xff;
		for (uint8_t j = 0; j < i; j++) {
			checksum_ref ^= buffer[j];
		}
		REQUIRE(Framing::checksum(buffer.data(), i) == checksum_ref);
	}
}

TEST_CASE("log2() returns correct results", "[frame] [log2()]") {
	using namespace EV3UartGenerator;

	for (uint16_t arg = 1; arg < 0x100; arg++) {
		if (arg < 2) {
			REQUIRE(Framing::log2(arg) == 0);
		} else if (arg < 3) {
			REQUIRE(Framing::log2(arg) == 1);
		} else if (arg < 5) {
			REQUIRE(Framing::log2(arg) == 2);
		} else if (arg < 9) {
			REQUIRE(Framing::log2(arg) == 3);
		} else if (arg < 17) {
			REQUIRE(Framing::log2(arg) == 4);
		} else if (arg < 33) {
			REQUIRE(Framing::log2(arg) == 5);
		} else if (arg < 65) {
			REQUIRE(Framing::log2(arg) == 6);
		} else if (arg < 129) {
			REQUIRE(Framing::log2(arg) == 7);
		} else if (arg < 256) {
			REQUIRE(Framing::log2(arg) == 8);
		}
	}
}

TEST_CASE("length_code() returns correct results", "[frame] [length_code()]") {
	using namespace EV3UartGenerator;
	for (uint8_t arg = 1; arg <= 0x20; arg++) {
		REQUIRE(Framing::length_code(arg) == (Framing::log2(arg) << 0x03));
	}
}

TEST_CASE("insert_padding() returns correct results and inserts the correct"
		" amount and type of padding", "[frame] [insert_padding()]") {
	using namespace EV3UartGenerator;
	for (uint8_t i = Framing::PAYLOAD_MIN;
			i <= Framing::PAYLOAD_SENSOR_TO_EV3_MAX; i++) {
		std::array<uint8_t, 0x20> buffer {};
		buffer.fill(0xff);

		uint8_t padding = Framing::insert_padding(buffer.data(), i);
		REQUIRE(padding == ((0x01 << Framing::log2(i)) - i));

		for (uint8_t pad_count = 0; pad_count < padding; pad_count++)
			REQUIRE(buffer[pad_count] == 0x00);

		REQUIRE(buffer[padding] == 0xff);
	}
}



