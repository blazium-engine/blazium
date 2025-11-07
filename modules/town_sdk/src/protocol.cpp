/**************************************************************************/
/*  protocol.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/
/* Copyright (c) 2024-present Blazium Engine contributors.                */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "core/error/error_macros.h"

namespace turnbattle {
namespace protocol {

// Message type constants (must match server)
namespace MessageType {
constexpr uint16_t VERSION_CHECK = 0;
constexpr uint16_t HELLO = 1;
constexpr uint16_t HELLO_ACK = 2;
constexpr uint16_t DISCONNECT = 3;
constexpr uint16_t VERSION_MISMATCH = 4;

constexpr uint16_t REGION_ENTER = 10;
constexpr uint16_t REGION_LEAVE = 11;
constexpr uint16_t MOVE_INPUT = 12;

constexpr uint16_t REGION_SNAPSHOT = 20;
constexpr uint16_t MOVE_STATE = 23;

constexpr uint16_t BATTLE_INDICATOR_SPAWN = 30;
constexpr uint16_t BATTLE_INDICATOR_DESPAWN = 31;
constexpr uint16_t BATTLE_START = 34;
constexpr uint16_t BATTLE_STATE = 35;
constexpr uint16_t BATTLE_ACTION = 36;
constexpr uint16_t BATTLE_RESULT = 37;
constexpr uint16_t BATTLE_END = 38;
constexpr uint16_t BATTLE_LOG = 39;
constexpr uint16_t BATTLE_LEAVE = 33;

constexpr uint16_t ERROR_MSG = 200;
} //namespace MessageType

// Channel constants
namespace Channel {
constexpr uint8_t CONTROL = 0;
constexpr uint8_t REGION = 1;
constexpr uint8_t BATTLE = 2;
constexpr uint8_t NOTIFICATIONS = 3;
} //namespace Channel

// Encode message: [type:uint16][len:uint16] + payload
std::string encode_message(uint16_t type, const std::string &payload) {
	uint16_t len = static_cast<uint16_t>(payload.size());
	std::string result(4 + payload.size(), '\0');

	// Little-endian encoding
	result[0] = static_cast<char>(type & 0xFF);
	result[1] = static_cast<char>((type >> 8) & 0xFF);
	result[2] = static_cast<char>(len & 0xFF);
	result[3] = static_cast<char>((len >> 8) & 0xFF);

	if (!payload.empty()) {
		std::memcpy(&result[4], payload.data(), payload.size());
	}

	return result;
}

// Decode message header (returns {type, payload_length})
std::pair<uint16_t, uint16_t> decode_header(const char *data, size_t len) {
	if (len < 4) {
		ERR_PRINT("Message too short for header");
		return { 0, 0 };
	}

	uint16_t type = static_cast<uint16_t>(static_cast<unsigned char>(data[0])) |
			(static_cast<uint16_t>(static_cast<unsigned char>(data[1])) << 8);
	uint16_t payload_len = static_cast<uint16_t>(static_cast<unsigned char>(data[2])) |
			(static_cast<uint16_t>(static_cast<unsigned char>(data[3])) << 8);

	return { type, payload_len };
}

} // namespace protocol
} // namespace turnbattle
