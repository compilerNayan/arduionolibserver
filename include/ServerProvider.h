#ifndef SERVERPROVIDER_H
#define SERVERPROVIDER_H

#include "IServer.h"
#include <StandardDefines.h>
#include <functional>
#include <memory>

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <iostream>
#endif

/**
 * Provider class for managing server instances
 * Manages server lifecycle and provides singleton access to the default server
 */
class ServerProvider {

    // Static map to store factory functions for each server ID
    Private Static Map<StdString, std::function<IServerPtr()>> serverFactories_;
    
    // Cached singleton instance of the default server
    Private Static IServerPtr defaultServerInstance_;

    /**
     * Register a server type with a factory function
     * @tparam ServerType The server class type (must inherit from IServer)
     * @param serverId Unique identifier for the server type
     * @return true if registration was successful, false if ID already exists
     */
    
    Public template<typename ServerType>
    Static Bool RegisterServer(CStdString& serverId) {
        // Check if server ID already exists
        if (serverFactories_.find(StdString(serverId)) != serverFactories_.end()) {
            return false;
        }
        
        // Register factory function that creates an instance of ServerType
        serverFactories_[StdString(serverId)] = []() -> IServerPtr {
            return make_ptr<ServerType>();
        };
        
        return true;
    }
    
    /**
     * Register a server type with a factory function and constructor arguments
     * @tparam ServerType The server class type (must inherit from IServer)
     * @tparam Args Argument types for the constructor
     * @param serverId Unique identifier for the server type
     * @param args Arguments to pass to the server constructor
     * @return true if registration was successful, false if ID already exists
     */
    Public template<typename ServerType, typename... Args>
    Static Bool RegisterServer(CStdString& serverId, Args&&... args) {
        // Check if server ID already exists
        if (serverFactories_.find(StdString(serverId)) != serverFactories_.end()) {
            return false;
        }
        
        // Register factory function that creates an instance with arguments
        serverFactories_[StdString(serverId)] = [args...]() -> IServerPtr {
            return make_ptr<ServerType>(args...);
        };
        
        return true;
    }
    
    /**
     * Create and return a server instance by ID
     * @param serverId The unique identifier for the server type
     * @return IServerPtr (shared_ptr) to the server instance, or nullptr if ID not found
     */
    Public Static IServerPtr GetServer(CStdString& serverId) {
        auto it = serverFactories_.find(StdString(serverId));
        if (it != serverFactories_.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    /**
     * Check if a server type is registered
     * @param serverId The unique identifier for the server type
     * @return true if server ID is registered, false otherwise
     */
    Public Static Bool IsRegistered(CStdString& serverId) {
        return serverFactories_.find(StdString(serverId)) != serverFactories_.end();
    }
    
    /**
     * Unregister a server type
     * @param serverId The unique identifier for the server type to unregister
     * @return true if server was unregistered, false if ID not found
     */
    Public Static Bool UnregisterServer(CStdString& serverId) {
        auto it = serverFactories_.find(StdString(serverId));
        if (it != serverFactories_.end()) {
            serverFactories_.erase(it);
            // Clear cached instance if it was from this server type
            defaultServerInstance_.reset();
            return true;
        }
        return false;
    }
    
    /**
     * Get the number of registered server types
     * @return Number of registered server types
     */
    Public Static Size GetRegisteredCount() {
        return serverFactories_.size();
    }
    
    /**
     * Clear all registered server types
     */
    Public Static Void Clear() {
        serverFactories_.clear();
        defaultServerInstance_.reset();
    }
    
    /**
     * Get the default server (first registered server)
     * Returns a singleton instance - the same instance is returned on every call
     * Note: This method does NOT start the server. The caller is responsible for starting it.
     * @return IServerPtr (shared_ptr) to the default server instance, or nullptr if no servers are registered
     */
    Public Static IServerPtr GetDefaultServer() {
#ifdef ARDUINO
        Serial.println("[ServerProvider] GetDefaultServer() called");
#else
        std::cout << "[ServerProvider] GetDefaultServer() called" << std::endl;
#endif
        
        // If we already have a cached instance, return it
        if (defaultServerInstance_ != nullptr) {
#ifdef ARDUINO
            Serial.println("[ServerProvider] Returning cached server instance");
#else
            std::cout << "[ServerProvider] Returning cached server instance" << std::endl;
#endif
            return defaultServerInstance_;
        }
        
        // No cached instance, create one
        if (serverFactories_.empty()) {
#ifdef ARDUINO
            Serial.println("[ServerProvider] ERROR: No server factories registered! serverFactories_ is empty.");
#else
            std::cout << "[ServerProvider] ERROR: No server factories registered! serverFactories_ is empty." << std::endl;
#endif
            return nullptr;
        }
        
#ifdef ARDUINO
        Serial.print("[ServerProvider] Found ");
        Serial.print(serverFactories_.size());
        Serial.println(" server factory(ies), creating server instance from first factory...");
#else
        std::cout << "[ServerProvider] Found " << serverFactories_.size() << " server factory(ies), creating server instance from first factory..." << std::endl;
#endif
        
        // Create the server instance using the first factory
        defaultServerInstance_ = serverFactories_.begin()->second();
        
        if (defaultServerInstance_ == nullptr) {
#ifdef ARDUINO
            Serial.println("[ServerProvider] ERROR: Factory returned nullptr server instance!");
#else
            std::cout << "[ServerProvider] ERROR: Factory returned nullptr server instance!" << std::endl;
#endif
        } else {
#ifdef ARDUINO
            Serial.println("[ServerProvider] Successfully created server instance");
#else
            std::cout << "[ServerProvider] Successfully created server instance" << std::endl;
#endif
        }
        
        return defaultServerInstance_;
    }
};

#include "ServerProviderInit.h"

#endif // SERVERPROVIDER_H

