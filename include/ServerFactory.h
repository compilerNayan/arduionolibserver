#ifndef SERVERFACTORY_H
#define SERVERFACTORY_H

#include "IServer.h"
#include <StandardDefines.h>
#include <functional>
#include <memory>

/**
 * Factory class for creating server instances
 * Allows registration of server types and creation of server instances by ID
 */
class ServerFactory {


    // Static map to store factory functions for each server ID
    Private Static Map<StdString, std::function<IServerPtr()>> serverFactories_;


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
    }
    
    /**
     * Get the default server (first registered server)
     * @return IServerPtr (shared_ptr) to the first server instance, or nullptr if no servers are registered
     */
    Public Static IServerPtr GetDefaultServer() {
        if (serverFactories_.empty()) {
            return nullptr;
        }
        // Return the first server in the map
        return serverFactories_.begin()->second();
    }
};

#include "ServerFactoryInit.h"

#endif // SERVERFACTORY_H


