#ifndef IHTTPRESPONSE_H
#define IHTTPRESPONSE_H

#include <StandardDefines.h>

/**
 * Interface representing a complete HTTP response
 * Contains all possible components of an HTTP response
 */
DefineStandardPointers(IHttpResponse)
class IHttpResponse {
    Public Virtual ~IHttpResponse() = default;
    
    // ========== Status Line Components ==========
    
    /**
     * Get the HTTP version (e.g., "HTTP/1.1", "HTTP/2.0")
     */
    Public Virtual CStdString& GetHttpVersion() const = 0;
    
    /**
     * Get the HTTP status code (e.g., 200, 404, 500)
     */
    Public Virtual UInt GetStatusCode() const = 0;
    
    /**
     * Get the HTTP status message (e.g., "OK", "Not Found", "Internal Server Error")
     */
    Public Virtual CStdString& GetStatusMessage() const = 0;
    
    // ========== Headers ==========
    
    /**
     * Get a header value by name (case-insensitive)
     * @param name The header name (e.g., "Content-Type", "Location", "Set-Cookie")
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
    
    // ========== Response Body/Payload ==========
    
    /**
     * Get the raw response body as string
     */
    Public Virtual CStdString& GetBody() const = 0;
    
    /**
     * Get the response body as bytes/raw data
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
     * Get a Set-Cookie header value by name
     * @param name The cookie name
     * @return The Set-Cookie header value, or empty string if not found
     */
    Public Virtual StdString GetSetCookie(CStdString& name) const = 0;
    
    /**
     * Get all Set-Cookie headers as a map
     */
    Public Virtual const Map<StdString, StdString>& GetSetCookies() const = 0;
    
    /**
     * Check if a Set-Cookie header exists
     */
    Public Virtual Bool HasSetCookie(CStdString& name) const = 0;
    
    // ========== Common Response Headers ==========
    
    /**
     * Get the Location header value (for redirects)
     */
    Public Virtual StdString GetLocation() const = 0;
    
    /**
     * Get the Server header value
     */
    Public Virtual StdString GetServer() const = 0;
    
    /**
     * Get the Date header value
     */
    Public Virtual StdString GetDate() const = 0;
    
    /**
     * Get the Last-Modified header value
     */
    Public Virtual StdString GetLastModified() const = 0;
    
    /**
     * Get the ETag header value
     */
    Public Virtual StdString GetETag() const = 0;
    
    /**
     * Get the Cache-Control header value
     */
    Public Virtual StdString GetCacheControl() const = 0;
    
    /**
     * Get the Expires header value
     */
    Public Virtual StdString GetExpires() const = 0;
    
    /**
     * Get the Allow header value (for 405 Method Not Allowed)
     */
    Public Virtual StdString GetAllow() const = 0;
    
    /**
     * Get the WWW-Authenticate header value (for 401 Unauthorized)
     */
    Public Virtual StdString GetWwwAuthenticate() const = 0;
    
    /**
     * Get the Content-Encoding header value (e.g., "gzip", "deflate")
     */
    Public Virtual StdString GetContentEncoding() const = 0;
    
    /**
     * Get the Content-Language header value
     */
    Public Virtual StdString GetContentLanguage() const = 0;
    
    /**
     * Get the Content-Disposition header value
     */
    Public Virtual StdString GetContentDisposition() const = 0;
    
    /**
     * Get the Content-Range header value
     */
    Public Virtual StdString GetContentRange() const = 0;
    
    // ========== Raw Response ==========
    
    /**
     * Get the complete raw HTTP response as string
     * This includes status line, headers, and body
     */
    Public Virtual CStdString& GetRawResponse() const = 0;
    
    /**
     * Convert the HTTP response to its string representation
     * This builds a proper HTTP response string with status line, headers, and body
     * @return HTTP response as string in proper HTTP format
     */
    Public Virtual StdString ToHttpString() const = 0;
    
    // ========== Utility Methods ==========
    
    /**
     * Check if the response has a body
     */
    Public Virtual Bool HasBody() const = 0;
    
    /**
     * Check if the status code indicates success (2xx)
     */
    Public Virtual Bool IsSuccess() const = 0;
    
    /**
     * Check if the status code indicates a redirect (3xx)
     */
    Public Virtual Bool IsRedirect() const = 0;
    
    /**
     * Check if the status code indicates a client error (4xx)
     */
    Public Virtual Bool IsClientError() const = 0;
    
    /**
     * Check if the status code indicates a server error (5xx)
     */
    Public Virtual Bool IsServerError() const = 0;
    
    /**
     * Check if Content-Type is JSON
     */
    Public Virtual Bool IsJson() const = 0;
    
    /**
     * Check if Content-Type is HTML
     */
    Public Virtual Bool IsHtml() const = 0;
    
    /**
     * Check if Content-Type is XML
     */
    Public Virtual Bool IsXml() const = 0;
    
    /**
     * Check if Content-Type is text
     */
    Public Virtual Bool IsText() const = 0;
    
    /**
     * Get the response timestamp (when it was created)
     */
    Public Virtual ULong GetTimestamp() const = 0;
    
    /**
     * Get the request ID (GUID) associated with this response
     */
    Public Virtual CStdString& GetRequestId() const = 0;
    
    /**
     * Set the request ID (GUID) for this response
     * Can be called after construction to set the request ID
     */
    Public Virtual Void SetRequestId(CStdString& requestId) = 0;
    
    // ========== Static Factory Method ==========
    
    /**
     * Create an IHttpResponse object with request ID and response body
     * @param requestId The unique request ID (GUID) for this response
     * @param body The response body content
     * @return IHttpResponsePtr (shared_ptr), or nullptr if requestId is empty
     */
    Static inline IHttpResponsePtr GetResponse(CStdString& requestId, CStdString& body);
};

// Include SimpleHttpResponse for inline implementation
#include "SimpleHttpResponse.h"

// Inline implementation
inline IHttpResponsePtr IHttpResponse::GetResponse(CStdString& requestId, CStdString& body) {
    if (requestId.empty()) {
        return nullptr;
    }
    return make_ptr<SimpleHttpResponse>(requestId, body);
}

#endif // IHTTPRESPONSE_H

