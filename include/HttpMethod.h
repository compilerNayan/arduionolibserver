#ifndef HTTPMETHOD_H
#define HTTPMETHOD_H

#include "StandardDefines.h"

/**
 * Enumeration of HTTP methods
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    TRACE,
    CONNECT
};

/**
 * Helper function to convert HttpMethod enum to string
 */
inline StdString MethodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::HEAD: return "HEAD";
        case HttpMethod::OPTIONS: return "OPTIONS";
        case HttpMethod::TRACE: return "TRACE";
        case HttpMethod::CONNECT: return "CONNECT";
        default: return "UNKNOWN";
    }
}

/**
 * Helper function to convert string to HttpMethod enum
 */
inline HttpMethod StringToMethod(CStdString& methodStr) {
    if (methodStr == "GET") return HttpMethod::GET;
    if (methodStr == "POST") return HttpMethod::POST;
    if (methodStr == "PUT") return HttpMethod::PUT;
    if (methodStr == "DELETE") return HttpMethod::DELETE;
    if (methodStr == "PATCH") return HttpMethod::PATCH;
    if (methodStr == "HEAD") return HttpMethod::HEAD;
    if (methodStr == "OPTIONS") return HttpMethod::OPTIONS;
    if (methodStr == "TRACE") return HttpMethod::TRACE;
    if (methodStr == "CONNECT") return HttpMethod::CONNECT;
    return HttpMethod::GET; // Default
}

#endif // HTTPMETHOD_H

