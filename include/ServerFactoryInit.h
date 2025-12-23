#ifndef SERVERFACTORYINIT_H
#define SERVERFACTORYINIT_H

#include "HttpTcpServer.h"

/**
 * Initialize the factory with default server types
 * Registers HttpTcpServer with GUID "550e8400-e29b-41d4-a716-446655440000"
 * @return true if initialization was successful, false if server already registered
 */
inline Bool Init() {
    StdString httpTcpServerGuid = "550e8400-e29b-41d4-a716-446655440000";
    return ServerFactory::RegisterServer<HttpTcpServer>(httpTcpServerGuid);
}

// Define static member outside the class (inline for header-only library)
inline std_map<StdString, std::function<IServerPtr()>> ServerFactory::serverFactories_;

Init();

#endif // SERVERFACTORYINIT_H

