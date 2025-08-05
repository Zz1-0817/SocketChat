#pragma once

#include <TypeDef.h>

#include <string>

Request parseRequest(const std::string& raw);
Message parseMessage(const Request& request);
std::string msgToResponse(const Message& msg);
