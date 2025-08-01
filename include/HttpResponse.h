#pragma once

#include <map>
#include <string>

namespace Http {
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

Method parse_method(const std::string& str);

class Response {
   private:
    Method method;
    std::string uri;
    std::map<std::string, std::string> headers;
    std::string body;

   public:
    Response() = default;
    /*
     * Parsing HTTP responses. For example, an HTTP response has the form as
     * following: POST /messages HTTP/1.1\r\n Host: localhost\r\n Content-Type:
     * application/json\r\n Content-Length:
     * 42\r\n\r\n{"user":"Adam","message":"Hello World!"}
     */
    Response(std::string raw);
    std::string getContentType(const std::string& filename);
    std::string readFile(const std::string& filename);
    std::string httpRoute();
    std::string handleGetRequest();
    std::string handlePostRequest();
};
}  // namespace Http
