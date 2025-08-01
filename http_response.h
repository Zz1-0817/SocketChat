#pragma once

#include <unistd.h>
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>

namespace HttpR
{
    #define MAX_EVENTS 1024
    #define PORT 18080
    class HttpResponse
    {
        private:
            std::string method;
            std::string uri;
            std::map<std::string, std::string> headers;
            std::string body;
        public:
            HttpResponse() = default;
            HttpResponse(std::string HR){}
            void HandleResponse();
            std::string GetContentType(const std::string& filename);
            std::string ReadFile(const std::string& filename);
            std::string HttepRoute();
            std::string HandleGetRequest();
            std::string HandlePostRequest();
    };
}