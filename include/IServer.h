#ifndef ISERVER_H
#define ISERVER_H

#include <StandardDefines.h>
#include "ServerType.h"

// Forward declaration and pointer types
DefineStandardPointers(IHttpRequest)

// Default port number if not specified
#ifndef DEFAULT_SERVER_PORT
#define DEFAULT_SERVER_PORT 8080
#endif

/**
 * Interface for network servers (TCP/UDP)
 * Defines common operations that all server implementations should support
 */
DefineStandardPointers(IServer)
class IServer {
    Public Virtual ~IServer() = default;
    
    // ========== Server Lifecycle ==========
    
    /**
     * Start the server and bind to the specified port
     * @param port Port number to listen on (default: DEFAULT_SERVER_PORT)
     * @return true if server started successfully, false otherwise
     */
    Public Virtual Bool Start(CUInt port = DEFAULT_SERVER_PORT) = 0;
    
    /**
     * Stop the server and release resources
     */
    Public Virtual Void Stop() = 0;
    
    /**
     * Check if the server is currently running
     * @return true if server is running, false otherwise
     */
    Public Virtual Bool IsRunning() const = 0;
    
    // ========== Port Configuration ==========
    
    /**
     * Get the port number the server is listening on
     * @return Port number (0 if not set)
     */
    Public Virtual UInt GetPort() const = 0;
    
    // ========== IP Address Configuration ==========
    
    /**
     * Get the IP address the server is bound to
     * @return IP address as string (e.g., "0.0.0.0" for all interfaces, "127.0.0.1" for localhost)
     */
    Public Virtual StdString GetIpAddress() const = 0;
    
    /**
     * Set the IP address to bind to (only works if server is not running)
     * @param ip IP address as string (e.g., "0.0.0.0", "127.0.0.1", "192.168.1.100")
     * @return true if IP was set successfully, false if server is running or invalid IP
     */
    Public Virtual Bool SetIpAddress(CStdString& ip) = 0;
    
    // ========== Message Operations ==========
    
    /**
     * Receive a message from a client
     * @return IHttpRequestPtr (shared_ptr), nullptr on error or no message
     */
    Public Virtual IHttpRequestPtr ReceiveMessage() = 0;
    
    /**
     * Send a message to a client
     * @param requestId The unique request ID (GUID) to identify the client connection
     * @param message Message to send
     * @return true if message was sent successfully, false otherwise
     */
    Public Virtual Bool SendMessage(CStdString& requestId, CStdString& message) = 0;
    
    // ========== Client Information ==========
    
    /**
     * Get the last client's IP address that sent a message
     * @return Client IP address as string, empty string if no client
     */
    Public Virtual StdString GetLastClientIp() const = 0;
    
    /**
     * Get the last client's port that sent a message
     * @return Client port number, 0 if no client
     */
    Public Virtual UInt GetLastClientPort() const = 0;
    
    // ========== Server Statistics ==========
    
    /**
     * Get the number of messages received since server started
     * @return Count of received messages
     */
    Public Virtual ULong GetReceivedMessageCount() const = 0;
    
    /**
     * Get the number of messages sent since server started
     * @return Count of sent messages
     */
    Public Virtual ULong GetSentMessageCount() const = 0;
    
    /**
     * Reset message counters
     */
    Public Virtual Void ResetStatistics() = 0;
    
    // ========== Server Configuration ==========
    
    /**
     * Get the maximum message size that can be received
     * @return Maximum message size in bytes
     */
    Public Virtual UInt GetMaxMessageSize() const = 0;
    
    /**
     * Set the maximum message size that can be received
     * @param size Maximum message size in bytes
     * @return true if size was set successfully, false if server is running
     */
    Public Virtual Bool SetMaxMessageSize(Size size) = 0;
    
    /**
     * Get the receive timeout in milliseconds
     * @return Timeout in milliseconds, 0 means blocking indefinitely
     */
    Public Virtual UInt GetReceiveTimeout() const = 0;
    
    /**
     * Set the receive timeout in milliseconds
     * @param timeoutMs Timeout in milliseconds, 0 means blocking indefinitely
     * @return true if timeout was set successfully
     */
    Public Virtual Bool SetReceiveTimeout(CUInt timeoutMs) = 0;
    
    // ========== Server Type Information ==========
    
    /**
     * Get the server type
     * @return Server type enum (TCP, UDP, etc.)
     */
    Public Virtual ServerType GetServerType() const = 0;
};

#endif // ISERVER_H

