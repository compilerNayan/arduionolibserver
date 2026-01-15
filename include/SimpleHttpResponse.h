#ifndef SIMPLEHTTPRESPONSE_H
#define SIMPLEHTTPRESPONSE_H

#include <StandardDefines.h>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

// Include IHttpResponse - if already included, the guard will prevent re-inclusion
// but the class will be fully defined
#include "IHttpResponse.h"

/**
 * Simple concrete implementation of IHttpResponse interface
 */
class SimpleHttpResponse : public IHttpResponse {

    Private StdString httpVersion_;
    Private UInt statusCode_;
    Private StdString statusMessage_;
    Private Map<StdString, StdString> headers_;
    Private Map<StdString, StdString> setCookies_;
    Private StdString body_;
    Private Vector<UInt8> bodyBytes_;
    Private ULong timestamp_;
    Private StdString rawResponse_;
    Private StdString requestId_;
    
    Private StdString ToLower(CStdString& str) const {
        StdString result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    Private StdString GetStatusMessageForCode(CUInt code) const {
        switch (code) {
            case 200: return "OK";
            case 201: return "Created";
            case 204: return "No Content";
            case 301: return "Moved Permanently";
            case 302: return "Found";
            case 304: return "Not Modified";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 500: return "Internal Server Error";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            default: return "Unknown";
        }
    }

    Public SimpleHttpResponse(CStdString& requestId, CStdString& body) 
        : httpVersion_("HTTP/1.1"), statusCode_(200), statusMessage_("OK"), timestamp_(0) {
        requestId_ = requestId;
        body_ = body;
        timestamp_ = static_cast<ULong>(std::time(nullptr));
        
        // Convert body to bytes
        if (!body_.empty()) {
            bodyBytes_.reserve(body_.length());
            for (Size i = 0; i < body_.length(); ++i) {
                bodyBytes_.push_back(static_cast<UInt8>(body_[i]));
            }
            
            // Set default Content-Type if body is not empty
            headers_["Content-Type"] = "text/plain";
            headers_["Content-Length"] = std::to_string(body_.length());
        }
    }

    /**
     * Constructor with status code, headers, and body
     * Used for creating responses from ResponseEntity<T>
     */
    Public SimpleHttpResponse(
        CStdString& requestId, 
        CUInt statusCode,
        CStdString& statusMessage,
        const Map<StdString, StdString>& headers,
        CStdString& body
    ) 
        : httpVersion_("HTTP/1.1"), statusCode_(statusCode), statusMessage_(statusMessage), timestamp_(0) {
        requestId_ = requestId;
        body_ = body;
        headers_ = headers;
        timestamp_ = static_cast<ULong>(std::time(nullptr));
        
        // Convert body to bytes
        if (!body_.empty()) {
            bodyBytes_.reserve(body_.length());
            for (Size i = 0; i < body_.length(); ++i) {
                bodyBytes_.push_back(static_cast<UInt8>(body_[i]));
            }
        }
        
        // Ensure Content-Length header is set
        if (!headers_.count("Content-Length")) {
            headers_["Content-Length"] = std::to_string(body_.length());
        }
        
        // Set default Content-Type if not provided
        if (!headers_.count("Content-Type") && !body_.empty()) {
            headers_["Content-Type"] = "application/json";
        }
    }
    
    Public Virtual CStdString& GetHttpVersion() const override { 
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(httpVersion_)); 
    }
    
    Public Virtual UInt GetStatusCode() const override {
        return statusCode_;
    }
    
    Public Virtual CStdString& GetStatusMessage() const override {
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(statusMessage_)); 
    }
    
    Public Virtual StdString GetHeader(CStdString& name) const override {
        StdString lowerName = ToLower(name);
        for (const auto& pair : headers_) {
            if (ToLower(pair.first) == lowerName) {
                return pair.second;
            }
        }
        return StdString();
    }
    
    Public Virtual const Map<StdString, StdString>& GetHeaders() const override {
        return headers_;
    }
    
    Public Virtual Bool HasHeader(CStdString& name) const override {
        StdString lowerName = ToLower(name);
        for (const auto& pair : headers_) {
            if (ToLower(pair.first) == lowerName) {
                return true;
            }
        }
        return false;
    }
    
    Public Virtual CStdString& GetBody() const override {
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(body_));
    }
    
    Public Virtual const Vector<UInt8>& GetBodyBytes() const override {
        return bodyBytes_;
    }
    
    Public Virtual StdString GetContentType() const override {
        return GetHeader("Content-Type");
    }
    
    Public Virtual ULong GetContentLength() const override {
        StdString lengthStr = GetHeader("Content-Length");
        if (lengthStr.empty()) return 0;
        try {
            return std::stoull(lengthStr);
        } catch (...) {
            return 0;
        }
    }
    
    Public Virtual StdString GetSetCookie(CStdString& name) const override {
        auto it = setCookies_.find(StdString(name));
        return (it != setCookies_.end()) ? it->second : StdString();
    }
    
    Public Virtual const Map<StdString, StdString>& GetSetCookies() const override {
        return setCookies_;
    }
    
    Public Virtual Bool HasSetCookie(CStdString& name) const override {
        return setCookies_.find(StdString(name)) != setCookies_.end();
    }
    
    Public Virtual StdString GetLocation() const override {
        return GetHeader("Location");
    }
    
    Public Virtual StdString GetServer() const override {
        return GetHeader("Server");
    }
    
    Public Virtual StdString GetDate() const override {
        return GetHeader("Date");
    }
    
    Public Virtual StdString GetLastModified() const override {
        return GetHeader("Last-Modified");
    }
    
    Public Virtual StdString GetETag() const override {
        return GetHeader("ETag");
    }
    
    Public Virtual StdString GetCacheControl() const override {
        return GetHeader("Cache-Control");
    }
    
    Public Virtual StdString GetExpires() const override {
        return GetHeader("Expires");
    }
    
    Public Virtual StdString GetAllow() const override {
        return GetHeader("Allow");
    }
    
    Public Virtual StdString GetWwwAuthenticate() const override {
        return GetHeader("WWW-Authenticate");
    }
    
    Public Virtual StdString GetContentEncoding() const override {
        return GetHeader("Content-Encoding");
    }
    
    Public Virtual StdString GetContentLanguage() const override {
        return GetHeader("Content-Language");
    }
    
    Public Virtual StdString GetContentDisposition() const override {
        return GetHeader("Content-Disposition");
    }
    
    Public Virtual StdString GetContentRange() const override {
        return GetHeader("Content-Range");
    }
    
    Public Virtual CStdString& GetRawResponse() const override {
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(rawResponse_));
    }
    
    Public Virtual StdString ToHttpString() const override {
        std::ostringstream oss;
        
        // Status line: HTTP/1.1 200 OK
        oss << httpVersion_ << " " 
            << statusCode_ << " " 
            << statusMessage_ << "\r\n";
        
        // Headers
        for (const auto& pair : headers_) {
            oss << pair.first << ": " << pair.second << "\r\n";
        }
        
        // Set-Cookie headers (if any)
        for (const auto& pair : setCookies_) {
            oss << "Set-Cookie: " << pair.second << "\r\n";
        }
        
        // Ensure Content-Length is set if body exists
        if (HasBody() && !HasHeader("Content-Length")) {
            oss << "Content-Length: " << body_.length() << "\r\n";
        }
        
        // Empty line to separate headers from body
        oss << "\r\n";
        
        // Body
        if (HasBody()) {
            oss << body_;
        }
        
        return oss.str();
    }
    
    Public Virtual Bool HasBody() const override {
        return !body_.empty();
    }
    
    Public Virtual Bool IsSuccess() const override {
        return statusCode_ >= 200 && statusCode_ < 300;
    }
    
    Public Virtual Bool IsRedirect() const override {
        return statusCode_ >= 300 && statusCode_ < 400;
    }
    
    Public Virtual Bool IsClientError() const override {
        return statusCode_ >= 400 && statusCode_ < 500;
    }
    
    Public Virtual Bool IsServerError() const override {
        return statusCode_ >= 500 && statusCode_ < 600;
    }
    
    Public Virtual Bool IsJson() const override {
        StdString contentType = GetContentType();
        StdString lower = ToLower(contentType);
        return lower.find("application/json") != StdString::npos;
    }
    
    Public Virtual Bool IsHtml() const override {
        StdString contentType = GetContentType();
        StdString lower = ToLower(contentType);
        return lower.find("text/html") != StdString::npos;
    }
    
    Public Virtual Bool IsXml() const override {
        StdString contentType = GetContentType();
        StdString lower = ToLower(contentType);
        return lower.find("application/xml") != StdString::npos || 
               lower.find("text/xml") != StdString::npos;
    }
    
    Public Virtual Bool IsText() const override {
        StdString contentType = GetContentType();
        StdString lower = ToLower(contentType);
        return lower.find("text/") != StdString::npos;
    }
    
    Public Virtual ULong GetTimestamp() const override {
        return timestamp_;
    }
    
    Public Virtual CStdString& GetRequestId() const override {
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(requestId_));
    }
    
    // Setters for customization
    Public Void SetStatusCode(CUInt code) { 
        statusCode_ = code; 
        statusMessage_ = GetStatusMessageForCode(code);
    }
    
    Public Void SetStatusMessage(CStdString& message) { 
        statusMessage_ = message; 
    }
    
    Public Void SetHeader(CStdString& name, CStdString& value) {
        headers_[StdString(name)] = StdString(value);
    }
    
    Public Void SetContentType(CStdString& contentType) {
        headers_["Content-Type"] = StdString(contentType);
    }
    
    /**
     * Set the request ID (can be set after construction)
     */
    Public Virtual Void SetRequestId(CStdString& requestId) override {
        requestId_ = StdString(requestId);
    }
};

#endif // SIMPLEHTTPRESPONSE_H

