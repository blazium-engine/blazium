/**************************************************************************/
/*  sd_enums.h                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#pragma once

// Stream Deck device types
enum SDDeviceType {
	SD_DEVICE_TYPE_STREAM_DECK = 0,
	SD_DEVICE_TYPE_STREAM_DECK_MINI = 1,
	SD_DEVICE_TYPE_STREAM_DECK_XL = 2,
	SD_DEVICE_TYPE_STREAM_DECK_MOBILE = 3,
	SD_DEVICE_TYPE_CORSAIR_G_KEYS = 4,
	SD_DEVICE_TYPE_STREAM_DECK_PEDAL = 7,
	SD_DEVICE_TYPE_CORSAIR_VOID = 8,
	SD_DEVICE_TYPE_STREAM_DECK_PLUS = 9,
	SD_DEVICE_TYPE_STREAM_DECK_NEO = 10,
};

// Target for visual updates
enum SDTarget {
	SD_TARGET_HARDWARE_AND_SOFTWARE = 0,
	SD_TARGET_HARDWARE_ONLY = 1,
	SD_TARGET_SOFTWARE_ONLY = 2,
};

// Title alignment
enum SDTitleAlignment {
	SD_TITLE_ALIGNMENT_TOP,
	SD_TITLE_ALIGNMENT_MIDDLE,
	SD_TITLE_ALIGNMENT_BOTTOM,
};

// Event types for routing
enum SDEventType {
	// Application monitoring
	SD_EVENT_APPLICATION_DID_LAUNCH,
	SD_EVENT_APPLICATION_DID_TERMINATE,

	// Device events
	SD_EVENT_DEVICE_DID_CHANGE,
	SD_EVENT_DEVICE_DID_CONNECT,
	SD_EVENT_DEVICE_DID_DISCONNECT,

	// Dial events (encoder actions)
	SD_EVENT_DIAL_DOWN,
	SD_EVENT_DIAL_ROTATE,
	SD_EVENT_DIAL_UP,

	// Deep linking
	SD_EVENT_DID_RECEIVE_DEEP_LINK,

	// Settings events
	SD_EVENT_DID_RECEIVE_GLOBAL_SETTINGS,
	SD_EVENT_DID_RECEIVE_SETTINGS,
	SD_EVENT_DID_RECEIVE_SECRETS,

	// Property inspector events
	SD_EVENT_DID_RECEIVE_PROPERTY_INSPECTOR_MESSAGE,
	SD_EVENT_PROPERTY_INSPECTOR_DID_APPEAR,
	SD_EVENT_PROPERTY_INSPECTOR_DID_DISAPPEAR,

	// Key events
	SD_EVENT_KEY_DOWN,
	SD_EVENT_KEY_UP,

	// Touch events
	SD_EVENT_TOUCH_TAP,

	// Action lifecycle events
	SD_EVENT_WILL_APPEAR,
	SD_EVENT_WILL_DISAPPEAR,

	// System events
	SD_EVENT_SYSTEM_DID_WAKE_UP,

	// Title events
	SD_EVENT_TITLE_PARAMETERS_DID_CHANGE,
};
