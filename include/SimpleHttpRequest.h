#ifndef SIMPLEHTTPREQUEST_H
#define SIMPLEHTTPREQUEST_H

#include <StandardDefines.h>
#include "HttpMethod.h"
#include <sstream>
#include <algorithm>
#include <ctime>

// Include IHttpRequest - if already included, the guard will prevent re-inclusion
// but the class will be fully defined
#include "IHttpRequest.h"

/**
 * Simple concrete implementation of IHttpRequest interface
 */
class SimpleHttpRequest : public IHttpRequest {

    Private HttpMethod method_;
    Private StdString path_;
    Private StdString fullUrl_;
    Private StdString httpVersion_;
    Private Map<StdString, StdString> queryParameters_;
    Private Map<StdString, StdString> headers_;
    Private Map<StdString, StdString> cookies_;
    Private StdString body_;
    Private Vector<UInt8> bodyBytes_;
    Private StdString clientIp_;
    Private UInt clientPort_;
    Private ULong timestamp_;
    Private StdString rawRequest_;
    Private StdString requestId_;
    
    Private StdString ToLower(CStdString& str) const {
        StdString result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    Private Void ParseQueryParameters(CStdString& queryString) {
        if (queryString.empty()) return;
        
        Size start = 0;
        while (start < queryString.length()) {
            Size eqPos = queryString.find('=', start);
            if (eqPos == StdString::npos) break;
            
            Size ampPos = queryString.find('&', eqPos);
            if (ampPos == StdString::npos) ampPos = queryString.length();
            
            StdString key = queryString.substr(start, eqPos - start);
            StdString value = queryString.substr(eqPos + 1, ampPos - eqPos - 1);
            
            // URL decode would go here if needed
            queryParameters_[key] = value;
            
            start = ampPos + 1;
        }
    }
    
    Private Void ParseCookies(CStdString& cookieHeader) {
        if (cookieHeader.empty()) return;
        
        Size start = 0;
        while (start < cookieHeader.length()) {
            // Skip whitespace
            while (start < cookieHeader.length() && (cookieHeader[start] == ' ' || cookieHeader[start] == '\t')) {
                start++;
            }
            
            Size eqPos = cookieHeader.find('=', start);
            if (eqPos == StdString::npos) break;
            
            Size semicolonPos = cookieHeader.find(';', eqPos);
            if (semicolonPos == StdString::npos) semicolonPos = cookieHeader.length();
            
            StdString key = cookieHeader.substr(start, eqPos - start);
            StdString value = cookieHeader.substr(eqPos + 1, semicolonPos - eqPos - 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            cookies_[key] = value;
            
            start = semicolonPos + 1;
        }
    }


    Public SimpleHttpRequest(CStdString& requestId, CStdString& rawRequest) 
        : method_(HttpMethod::GET), clientPort_(0), timestamp_(0) {
        rawRequest_ = rawRequest;
        requestId_ = requestId;
        timestamp_ = static_cast<ULong>(std::time(nullptr));
        
        if (rawRequest.empty()) return;
        
        // Find header-body separator
        Size headerEnd = rawRequest.find("\r\n\r\n");
        if (headerEnd == StdString::npos) {
            headerEnd = rawRequest.find("\n\n");
            if (headerEnd != StdString::npos) headerEnd += 2;
        } else {
            headerEnd += 4;
        }
        
        StdString headerSection = (headerEnd != StdString::npos) ? 
            rawRequest.substr(0, headerEnd - (headerEnd >= 4 ? 4 : 0)) : rawRequest;
        
        // Parse request line
        Size firstLineEnd = headerSection.find("\r\n");
        if (firstLineEnd == StdString::npos) firstLineEnd = headerSection.find('\n');
        if (firstLineEnd == StdString::npos) firstLineEnd = headerSection.length();
        
        StdString requestLine = headerSection.substr(0, firstLineEnd);
        std::istringstream lineStream(requestLine);
        StdString methodStr, url, version;
        lineStream >> methodStr >> url >> version;
        
        method_ = StringToMethod(methodStr);
        httpVersion_ = version;
        
        // Parse URL and query parameters
        Size queryPos = url.find('?');
        if (queryPos != StdString::npos) {
            path_ = url.substr(0, queryPos);
            StdString queryString = url.substr(queryPos + 1);
            ParseQueryParameters(queryString);
        } else {
            path_ = url;
        }
        fullUrl_ = url;
        
        // Parse headers
        Size headerStart = firstLineEnd + (headerSection[firstLineEnd] == '\r' ? 2 : 1);
        while (headerStart < headerSection.length()) {
            Size lineEnd = headerSection.find("\r\n", headerStart);
            if (lineEnd == StdString::npos) {
                lineEnd = headerSection.find('\n', headerStart);
                if (lineEnd == StdString::npos) lineEnd = headerSection.length();
            }
            
            if (lineEnd == headerStart) break; // Empty line
            
            StdString headerLine = headerSection.substr(headerStart, lineEnd - headerStart);
            Size colonPos = headerLine.find(':');
            if (colonPos != StdString::npos) {
                StdString headerName = headerLine.substr(0, colonPos);
                StdString headerValue = headerLine.substr(colonPos + 1);
                
                // Trim whitespace
                headerName.erase(0, headerName.find_first_not_of(" \t"));
                headerName.erase(headerName.find_last_not_of(" \t") + 1);
                headerValue.erase(0, headerValue.find_first_not_of(" \t"));
                headerValue.erase(headerValue.find_last_not_of(" \t") + 1);
                
                headers_[headerName] = headerValue;
                
                // Parse cookies
                if (ToLower(headerName) == "cookie") {
                    ParseCookies(headerValue);
                }
            }
            
            headerStart = lineEnd + (headerSection[lineEnd] == '\r' ? 2 : 1);
        }
        
        // Parse body
        if (headerEnd != StdString::npos && headerEnd < rawRequest.length()) {
            body_ = rawRequest.substr(headerEnd);
            bodyBytes_.reserve(body_.length());
            for (Size i = 0; i < body_.length(); ++i) {
                bodyBytes_.push_back(static_cast<UInt8>(body_[i]));
            }
        }
    }
    
    Public Virtual HttpMethod GetMethod() const override { return method_; }
    Public Virtual CStdString& GetPath() const override { 
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(path_)); 
    }
    Public Virtual CStdString& GetFullUrl() const override { 
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(fullUrl_)); 
    }
    Public Virtual CStdString& GetHttpVersion() const override { 
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(httpVersion_)); 
    }
    
    Public Virtual StdString GetQueryParameter(CStdString& name) const override {
        auto it = queryParameters_.find(StdString(name));
        return (it != queryParameters_.end()) ? it->second : StdString();
    }
    
    Public Virtual const Map<StdString, StdString>& GetQueryParameters() const override {
        return queryParameters_;
    }
    
    Public Virtual Bool HasQueryParameter(CStdString& name) const override {
        return queryParameters_.find(StdString(name)) != queryParameters_.end();
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
    
    Public Virtual StdString GetAuthorization() const override {
        return GetHeader("Authorization");
    }
    
    Public Virtual StdString GetBearerToken() const override {
        StdString auth = GetAuthorization();
        if (auth.empty()) return StdString();
        
        Size bearerPos = auth.find("Bearer ");
        if (bearerPos != StdString::npos) {
            return auth.substr(bearerPos + 7);
        }
        return StdString();
    }
    
    Public Virtual StdString GetBasicAuth() const override {
        StdString auth = GetAuthorization();
        if (auth.empty()) return StdString();
        
        Size basicPos = auth.find("Basic ");
        if (basicPos != StdString::npos) {
            return auth.substr(basicPos + 6);
        }
        return StdString();
    }
    
    Public Virtual StdString GetApiKey(CStdString& headerName) const override {
        return GetHeader(headerName);
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
    
    Public Virtual StdString GetCookie(CStdString& name) const override {
        auto it = cookies_.find(StdString(name));
        return (it != cookies_.end()) ? it->second : StdString();
    }
    
    Public Virtual const Map<StdString, StdString>& GetCookies() const override {
        return cookies_;
    }
    
    Public Virtual Bool HasCookie(CStdString& name) const override {
        return cookies_.find(StdString(name)) != cookies_.end();
    }
    
    Public Virtual CStdString& GetClientIp() const override {
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(clientIp_));
    }
    
    Public Virtual UInt GetClientPort() const override {
        return clientPort_;
    }
    
    Public Virtual StdString GetUserAgent() const override {
        return GetHeader("User-Agent");
    }
    
    Public Virtual StdString GetReferer() const override {
        return GetHeader("Referer");
    }
    
    Public Virtual StdString GetHost() const override {
        return GetHeader("Host");
    }
    
    Public Virtual CStdString& GetRawRequest() const override {
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(rawRequest_));
    }
    
    Public Virtual Bool HasBody() const override {
        return !body_.empty();
    }
    
    Public Virtual Bool IsMethod(HttpMethod method) const override {
        return method_ == method;
    }
    
    Public Virtual Bool IsJson() const override {
        StdString contentType = GetContentType();
        StdString lower = ToLower(contentType);
        return lower.find("application/json") != StdString::npos;
    }
    
    Public Virtual Bool IsFormData() const override {
        StdString contentType = GetContentType();
        StdString lower = ToLower(contentType);
        return lower.find("application/x-www-form-urlencoded") != StdString::npos;
    }
    
    Public Virtual Bool IsMultipart() const override {
        StdString contentType = GetContentType();
        StdString lower = ToLower(contentType);
        return lower.find("multipart/") != StdString::npos;
    }
    
    Public Virtual ULong GetTimestamp() const override {
        return timestamp_;
    }
    
    Public Virtual CStdString& GetRequestId() const override {
        return const_cast<CStdString&>(reinterpret_cast<const CStdString&>(requestId_));
    }
    
    Public Void SetClientIp(CStdString& ip) { clientIp_ = ip; }
    Public Void SetClientPort(CUInt port) { clientPort_ = port; }
};

#endif // SIMPLEHTTPREQUEST_H

