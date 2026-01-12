#ifndef IHTTPREQUEST_H
#define IHTTPREQUEST_H

#include <StandardDefines.h>
#include "HttpMethod.h"

/**
 * Interface representing a complete HTTP request
 * Contains all possible components of an HTTP request
 */
DefineStandardPointers(IHttpRequest)
class IHttpRequest {
    Public Virtual ~IHttpRequest() = default;
    
    // ========== Request Line Components ==========
    
    /**
     * Get the HTTP method (GET, POST, PUT, DELETE, etc.)
     */
    Public Virtual HttpMethod GetMethod() const = 0;
    
    /**
     * Get the request path/URL (e.g., "/api/users", "/index.html")
     */
    Public Virtual CStdString& GetPath() const = 0;
    
    /**
     * Get the full URL including query string
     */
    Public Virtual CStdString& GetFullUrl() const = 0;
    
    /**
     * Get the HTTP version (e.g., "HTTP/1.1", "HTTP/2.0")
     */
    Public Virtual CStdString& GetHttpVersion() const = 0;
    
    // ========== Query Parameters ==========
    
    /**
     * Get a query parameter value by name
     * @param name The parameter name
     * @return The parameter value, or empty string if not found
     */
    Public Virtual StdString GetQueryParameter(CStdString& name) const = 0;
    
    /**
     * Get all query parameters as a map
     */
    Public Virtual const Map<StdString, StdString>& GetQueryParameters() const = 0;
    
    /**
     * Check if a query parameter exists
     */
    Public Virtual Bool HasQueryParameter(CStdString& name) const = 0;
    
    // ========== Headers ==========
    
    /**
     * Get a header value by name (case-insensitive)
     * @param name The header name (e.g., "Content-Type", "Authorization")
     * @return The header value, or empty string if not found
     */
    Public Virtual StdString GetHeader(CStdString& name) const = 0;
    
    /**
     * Get all headers as a map
     */
    Public Virtual const Map<StdString, StdString>& GetHeaders() const = 0;
    
    /**
     * Check if a header exists
     */
    Public Virtual Bool HasHeader(CStdString& name) const = 0;
    
    // ========== Authentication & Tokens ==========
    
    /**
     * Get the Authorization header value
     */
    Public Virtual StdString GetAuthorization() const = 0;
    
    /**
     * Get the Bearer token from Authorization header
     * Extracts token from "Bearer <token>" format
     */
    Public Virtual StdString GetBearerToken() const = 0;
    
    /**
     * Get the Basic Auth credentials (base64 encoded)
     */
    Public Virtual StdString GetBasicAuth() const = 0;
    
    /**
     * Get API key/token from custom header (e.g., "X-API-Key")
     */
    Public Virtual StdString GetApiKey(CStdString& headerName = "X-API-Key") const = 0;
    
    // ========== Request Body/Payload ==========
    
    /**
     * Get the raw request body as string
     */
    Public Virtual CStdString& GetBody() const = 0;
    
    /**
     * Get the request body as bytes/raw data
     */
    Public Virtual const Vector<UInt8>& GetBodyBytes() const = 0;
    
    /**
     * Get the Content-Type header value
     */
    Public Virtual StdString GetContentType() const = 0;
    
    /**
     * Get the Content-Length header value
     */
    Public Virtual ULong GetContentLength() const = 0;
    
    // ========== Cookies ==========
    
    /**
     * Get a cookie value by name
     */
    Public Virtual StdString GetCookie(CStdString& name) const = 0;
    
    /**
     * Get all cookies as a map
     */
    Public Virtual const Map<StdString, StdString>& GetCookies() const = 0;
    
    /**
     * Check if a cookie exists
     */
    Public Virtual Bool HasCookie(CStdString& name) const = 0;
    
    // ========== Client Information ==========
    
    /**
     * Get the client's IP address
     */
    Public Virtual CStdString& GetClientIp() const = 0;
    
    /**
     * Get the client's port number
     */
    Public Virtual UInt GetClientPort() const = 0;
    
    /**
     * Get the User-Agent header value
     */
    Public Virtual StdString GetUserAgent() const = 0;
    
    /**
     * Get the Referer header value
     */
    Public Virtual StdString GetReferer() const = 0;
    
    /**
     * Get the Host header value
     */
    Public Virtual StdString GetHost() const = 0;
    
    // ========== Raw Request ==========
    
    /**
     * Get the complete raw HTTP request as received
     * This includes request line, headers, and body exactly as received
     */
    Public Virtual CStdString& GetRawRequest() const = 0;
    
    // ========== Utility Methods ==========
    
    /**
     * Check if the request has a body
     */
    Public Virtual Bool HasBody() const = 0;
    
    /**
     * Check if the request is a specific HTTP method
     */
    Public Virtual Bool IsMethod(HttpMethod method) const = 0;
    
    /**
     * Check if Content-Type is JSON
     */
    Public Virtual Bool IsJson() const = 0;
    
    /**
     * Check if Content-Type is form data
     */
    Public Virtual Bool IsFormData() const = 0;
    
    /**
     * Check if Content-Type is multipart
     */
    Public Virtual Bool IsMultipart() const = 0;
    
    /**
     * Get the request timestamp (when it was received)
     */
    Public Virtual ULong GetTimestamp() const = 0;
    
    /**
     * Get the request ID (GUID) associated with this request
     */
    Public Virtual CStdString& GetRequestId() const = 0;
    
    // ========== Static Factory Method ==========
    
    /**
     * Parse raw HTTP request string and create IHttpRequest object
     * @param requestId The unique request ID (GUID) for this request
     * @param rawRequest The raw HTTP request string from IServer::ReceiveMessage()
     * @return IHttpRequestPtr (shared_ptr), or nullptr if parsing fails
     */
    Static inline IHttpRequestPtr GetRequest(CStdString& requestId, CStdString& rawRequest);
};

// Include SimpleHttpRequest for inline implementation
#include "SimpleHttpRequest.h"

// Inline implementation
inline IHttpRequestPtr IHttpRequest::GetRequest(CStdString& requestId, CStdString& rawRequest) {
    if (rawRequest.empty()) {
        return nullptr;
    }
    return make_ptr<SimpleHttpRequest>(requestId, rawRequest);
}

#endif // IHTTPREQUEST_H

