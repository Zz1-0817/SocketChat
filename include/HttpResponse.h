#pragma once

#include <string>
#include <map>

namespace Http {
    #define MAX_EVENTS 1024
    #define PORT 18080

    enum class Method {
        GET,
        POST,
        PUT,
        PATCH,
        DELETE,
        HEAD,
        OPTIONS,
        TRACE,
        CONNECT
    };

    Method parse_method(const std::string &str);

    class Response {
        private:
            Method method;
            std::string uri;
            std::map<std::string, std::string> headers;
            std::string body;
        public:
            Response() = default;
            Response(std::string raw);
            void handleResponse();
            std::string getContentType(const std::string& filename);
            std::string readFile(const std::string& filename);
            std::string httpRoute();
            std::string handleGetRequest();
            std::string handlePostRequest();
    };
}

