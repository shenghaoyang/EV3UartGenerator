/**
 * \file ColorSensorInitialization.cpp
 *
 * Populates a buffer with data equivalent to that sent by a
 * EV3 color sensor upon initialization.
 *
 * \copyright Shenghao Yang, 2018
 *
 * See LICENSE for more details
 */

#include <framing.hpp>
#include <magics.hpp>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdint>

int main(int argc, char** argv) {
	using namespace std;
	using namespace EV3UartGenerator::Framing;
	using namespace EV3UartGenerator::Magics;
	uint8_t buffer[1024];

	memset(reinterpret_cast<void*>(buffer), 0, 1024);
	uint32_t sz_tally = 0;

	sz_tally += frame_cmd_type_message(buffer, 0x1d);
	sz_tally += frame_cmd_modes_message(buffer + sz_tally, 0x05, 0x02);
	sz_tally += frame_cmd_speed_message(buffer + sz_tally, 57600);

	sz_tally += frame_info_message_name(buffer + sz_tally, 5, "COL-CAL");
	sz_tally += frame_info_message_span(buffer + sz_tally, 5, INFO_SPAN::RAW, 0,
			65535);
	sz_tally += frame_info_message_span(buffer + sz_tally, 5, INFO_SPAN::SI, 0,
				65535);
	sz_tally += frame_info_message_format(buffer + sz_tally, 5, 4,
			INFO_DTYPE::S16, 5, 0);

	sz_tally += frame_info_message_name(buffer + sz_tally, 4, "RGB-RAW");
	sz_tally += frame_info_message_span(buffer + sz_tally, 4, INFO_SPAN::RAW, 0,
			1020.188);
	sz_tally += frame_info_message_span(buffer + sz_tally, 4, INFO_SPAN::SI, 0,
			1020.188);
	sz_tally += frame_info_message_format(buffer + sz_tally, 4, 3,
			INFO_DTYPE::S16, 4, 0);

	sz_tally += frame_info_message_name(buffer + sz_tally, 3, "REF-RAW");
	sz_tally += frame_info_message_span(buffer + sz_tally, 3, INFO_SPAN::RAW, 0,
			1020.188);
	sz_tally += frame_info_message_span(buffer + sz_tally, 3, INFO_SPAN::SI, 0,
			1020.188);
	sz_tally += frame_info_message_format(buffer + sz_tally, 3, 2,
			INFO_DTYPE::S16, 4, 0);

	sz_tally += frame_info_message_name(buffer + sz_tally, 2, "COL-COLOR");
	sz_tally += frame_info_message_span(buffer + sz_tally, 2, INFO_SPAN::RAW, 0,
			8);
	sz_tally += frame_info_message_span(buffer + sz_tally, 2, INFO_SPAN::SI, 0,
			8);
	sz_tally += frame_info_message_symbol(buffer + sz_tally, 2, "col");
	sz_tally += frame_info_message_format(buffer + sz_tally, 2, 1,
			INFO_DTYPE::S8, 2, 0);

	sz_tally += frame_info_message_name(buffer + sz_tally, 1, "COL-AMBIENT");
	sz_tally += frame_info_message_span(buffer + sz_tally, 1, INFO_SPAN::RAW, 0,
			100);
	sz_tally += frame_info_message_span(buffer + sz_tally, 1, INFO_SPAN::SI, 0,
				100);
	sz_tally += frame_info_message_symbol(buffer + sz_tally, 1, "pct");
	sz_tally += frame_info_message_format(buffer + sz_tally, 1, 1,
			INFO_DTYPE::S8, 3, 0);

	sz_tally += frame_info_message_name(buffer + sz_tally, 0, "COL-REFLECT");
	sz_tally += frame_info_message_span(buffer + sz_tally, 0, INFO_SPAN::RAW, 0,
			100);
	sz_tally += frame_info_message_span(buffer + sz_tally, 0, INFO_SPAN::SI, 0,
				100);
	sz_tally += frame_info_message_symbol(buffer + sz_tally, 0, "pct");
	sz_tally += frame_info_message_format(buffer + sz_tally, 0, 1,
			INFO_DTYPE::S8, 3, 0);

	sz_tally += frame_sys_message(buffer + sz_tally, SYS::ACK);
	std::cout << sz_tally << std::endl;
}
