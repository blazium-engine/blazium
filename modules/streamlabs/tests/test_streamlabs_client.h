/**************************************************************************/
/*  test_streamlabs_client.h                                              */
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

#pragma once

#include "modules/streamlabs/streamlabs_client.h"
#include "tests/test_macros.h"

namespace TestStreamlabsClient {

TEST_CASE("[Streamlabs][StreamlabsClient] Singleton initialization") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	CHECK_NE(client, nullptr);
	CHECK_EQ(client->get_connection_state(), StreamlabsClient::STATE_DISCONNECTED);
	CHECK_FALSE(client->is_streamlabs_connected());
}

TEST_CASE("[Streamlabs][StreamlabsClient] Connection state enum values") {
	CHECK_EQ(StreamlabsClient::STATE_DISCONNECTED, 0);
	CHECK_EQ(StreamlabsClient::STATE_CONNECTING, 1);
	CHECK_EQ(StreamlabsClient::STATE_CONNECTED, 2);
}

TEST_CASE("[Streamlabs][StreamlabsClient] Empty token rejection") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	REQUIRE_NE(client, nullptr);

	// Attempting to connect with empty token should fail
	Error err = client->connect_to_streamlabs("");
	CHECK_NE(err, OK);
	CHECK_EQ(client->get_connection_state(), StreamlabsClient::STATE_DISCONNECTED);
}

TEST_CASE("[Streamlabs][StreamlabsClient] Signal registration") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	REQUIRE_NE(client, nullptr);

	// Verify that all expected signals are registered
	CHECK(client->has_signal("connected"));
	CHECK(client->has_signal("disconnected"));
	CHECK(client->has_signal("connection_error"));
	CHECK(client->has_signal("donation"));
	CHECK(client->has_signal("twitch_follow"));
	CHECK(client->has_signal("twitch_subscription"));
	CHECK(client->has_signal("twitch_bits"));
	CHECK(client->has_signal("twitch_raid"));
	CHECK(client->has_signal("twitch_host"));
	CHECK(client->has_signal("youtube_follow"));
	CHECK(client->has_signal("youtube_subscription"));
	CHECK(client->has_signal("youtube_superchat"));
	CHECK(client->has_signal("mixer_follow"));
	CHECK(client->has_signal("mixer_subscription"));
	CHECK(client->has_signal("mixer_host"));
}

TEST_CASE("[Streamlabs][StreamlabsClient] Event parsing - Donation") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	REQUIRE_NE(client, nullptr);

	// Simulate donation event
	Dictionary event_data;
	event_data["type"] = "donation";

	Array message_array;
	Dictionary donation_msg;
	donation_msg["id"] = 12345;
	donation_msg["name"] = "TestDonor";
	donation_msg["amount"] = "10.00";
	donation_msg["formatted_amount"] = "$10.00";
	donation_msg["message"] = "Test donation message";
	donation_msg["currency"] = "USD";
	donation_msg["from"] = "TestDonor";
	donation_msg["_id"] = "test_event_id";

	Dictionary to_dict;
	to_dict["name"] = "TestStreamer";
	donation_msg["to"] = to_dict;

	message_array.push_back(donation_msg);
	event_data["message"] = message_array;

	// Test event structure
	CHECK(event_data.has("type"));
	CHECK_EQ(String(event_data["type"]), "donation");
	CHECK(event_data.has("message"));

	Array msgs = event_data["message"];
	CHECK_EQ(msgs.size(), 1);

	Dictionary msg = msgs[0];
	CHECK_EQ(String(msg["name"]), "TestDonor");
	CHECK_EQ(String(msg["formatted_amount"]), "$10.00");
	CHECK_EQ(String(msg["message"]), "Test donation message");
}

TEST_CASE("[Streamlabs][StreamlabsClient] Event parsing - Twitch Follow") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	REQUIRE_NE(client, nullptr);

	// Simulate twitch follow event
	Dictionary event_data;
	event_data["type"] = "follow";
	event_data["for"] = "twitch_account";

	Array message_array;
	Dictionary follow_msg;
	follow_msg["name"] = "TestFollower";
	follow_msg["id"] = "follower_123";
	follow_msg["created_at"] = "2025-10-18 00:00:00";
	follow_msg["_id"] = "test_follow_id";

	message_array.push_back(follow_msg);
	event_data["message"] = message_array;

	// Test event structure
	CHECK(event_data.has("type"));
	CHECK_EQ(String(event_data["type"]), "follow");
	CHECK(event_data.has("for"));
	CHECK_EQ(String(event_data["for"]), "twitch_account");

	Array msgs = event_data["message"];
	CHECK_EQ(msgs.size(), 1);

	Dictionary msg = msgs[0];
	CHECK_EQ(String(msg["name"]), "TestFollower");
	CHECK_EQ(String(msg["id"]), "follower_123");
}

TEST_CASE("[Streamlabs][StreamlabsClient] Event parsing - Twitch Subscription") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	REQUIRE_NE(client, nullptr);

	// Simulate twitch subscription event
	Dictionary event_data;
	event_data["type"] = "subscription";
	event_data["for"] = "twitch_account";

	Array message_array;
	Dictionary sub_msg;
	sub_msg["name"] = "TestSubscriber";
	sub_msg["months"] = 5;
	sub_msg["message"] = "Love the stream!";
	sub_msg["sub_plan"] = "1000";
	sub_msg["sub_plan_name"] = "Channel Subscription";
	sub_msg["sub_type"] = "resub";
	sub_msg["_id"] = "test_sub_id";

	message_array.push_back(sub_msg);
	event_data["message"] = message_array;

	// Test event structure
	CHECK(event_data.has("type"));
	CHECK_EQ(String(event_data["type"]), "subscription");
	CHECK(event_data.has("for"));
	CHECK_EQ(String(event_data["for"]), "twitch_account");

	Array msgs = event_data["message"];
	CHECK_EQ(msgs.size(), 1);

	Dictionary msg = msgs[0];
	CHECK_EQ(String(msg["name"]), "TestSubscriber");
	CHECK_EQ(int(msg["months"]), 5);
	CHECK_EQ(String(msg["sub_plan"]), "1000");
}

TEST_CASE("[Streamlabs][StreamlabsClient] Event parsing - YouTube Superchat") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	REQUIRE_NE(client, nullptr);

	// Simulate YouTube superchat event
	Dictionary event_data;
	event_data["type"] = "superchat";
	event_data["for"] = "youtube_account";

	Array message_array;
	Dictionary superchat_msg;
	superchat_msg["id"] = "superchat_123";
	superchat_msg["name"] = "TestSuperchatUser";
	superchat_msg["comment"] = "Great content!";
	superchat_msg["amount"] = "5000000";
	superchat_msg["currency"] = "USD";
	superchat_msg["displayString"] = "$5.00";
	superchat_msg["_id"] = "test_superchat_id";

	message_array.push_back(superchat_msg);
	event_data["message"] = message_array;

	// Test event structure
	CHECK(event_data.has("type"));
	CHECK_EQ(String(event_data["type"]), "superchat");
	CHECK(event_data.has("for"));
	CHECK_EQ(String(event_data["for"]), "youtube_account");

	Array msgs = event_data["message"];
	CHECK_EQ(msgs.size(), 1);

	Dictionary msg = msgs[0];
	CHECK_EQ(String(msg["name"]), "TestSuperchatUser");
	CHECK_EQ(String(msg["displayString"]), "$5.00");
	CHECK_EQ(String(msg["comment"]), "Great content!");
}

TEST_CASE("[Streamlabs][StreamlabsClient] Multiple messages in single event") {
	StreamlabsClient *client = StreamlabsClient::get_singleton();
	REQUIRE_NE(client, nullptr);

	// Simulate event with multiple follows
	Dictionary event_data;
	event_data["type"] = "follow";
	event_data["for"] = "twitch_account";

	Array message_array;
	for (int i = 0; i < 3; i++) {
		Dictionary follow_msg;
		follow_msg["name"] = "Follower" + String::num_int64(i);
		follow_msg["id"] = "follower_" + String::num_int64(i);
		follow_msg["created_at"] = "2025-10-18 00:00:0" + String::num_int64(i);
		follow_msg["_id"] = "test_follow_id_" + String::num_int64(i);
		message_array.push_back(follow_msg);
	}

	event_data["message"] = message_array;

	// Test multiple messages
	CHECK(event_data.has("message"));
	Array msgs = event_data["message"];
	CHECK_EQ(msgs.size(), 3);

	for (int i = 0; i < 3; i++) {
		Dictionary msg = msgs[i];
		String expected_name = "Follower" + String::num_int64(i);
		CHECK_EQ(String(msg["name"]), expected_name);
	}
}

} // namespace TestStreamlabsClient
