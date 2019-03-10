#include "rest_utils.h"
#include <string>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <nghttp2/asio_http2_client.h>
#include <boost/asio/ip/tcp.hpp>

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::client;

namespace {
Json::FastWriter fastWriter;
Json::Reader reader;
}

Json::Value touint64(uint64_t val) { return Json::Value(Json::UInt64(val)); }

Json::Value touint(uint32_t val) { return Json::Value(Json::UInt(val)); }
Json::Value touint(uint16_t val) { return Json::Value(Json::UInt(val)); }
Json::Value touint(uint8_t val) { return Json::Value(Json::UInt(val)); }

Json::Value toint(int val) { return Json::Value(Json::Int(val)); }
Json::Value tobool(bool val) { return Json::Value(val); }

bool send_and_receive(std::string ip_addr, int port, std::string route, Json::Value &sendPkt, Json::Value &recvPkt) {
    bool sess_failed = false;
	boost::asio::io_service io_service;

	std::string jsonPkt = fastWriter.write(sendPkt);
	std::string jsonResponsePkt = "";

	session sess(io_service, ip_addr, std::to_string(port));

	sess.on_connect([&sess, &ip_addr, port, &route, &jsonPkt, &jsonResponsePkt, &sess_failed](boost::asio::ip::tcp::resolver::iterator endpoint_it) {
		boost::system::error_code ec;
		std::string uri = "http://" + ip_addr + ":" + std::to_string(port) + route;
		auto req = sess.submit(ec, "POST", uri, jsonPkt);

		req->on_response([&jsonPkt, &uri, &jsonResponsePkt, &sess, &sess_failed](const response &res) {
			// TODO handle response errors
			if(res.status_code() != 200) {
                sess_failed = true;
				std::cout << "Error Status Code Received from " << uri << " : " << res.status_code() << std::endl;
			}
			res.on_data([&jsonPkt, &jsonResponsePkt, &sess](const uint8_t *data, std::size_t len) {
				if (len > 0) {
					const char *s = "";
					s = reinterpret_cast<const char *>(data);
					if(len > 0) {
						jsonResponsePkt.append(s, len);
					}
				}
			});
		});
		req->on_close([&jsonResponsePkt, &uri, &sess](uint32_t error_code) {
			sess.shutdown();
		});
	});

	sess.on_error([&sess_failed](const boost::system::error_code &ec) {
        sess_failed = true;
		std::cerr << "error: " << ec.message() << std::endl;
	});

	io_service.run();

    if(sess_failed) return false;

    return reader.parse(jsonResponsePkt, recvPkt);
}