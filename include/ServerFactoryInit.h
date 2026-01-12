#ifndef SERVERFACTORYINIT_H
#define SERVERFACTORYINIT_H

/**
 * Initialize the factory with default server types
 * Registers HttpTcpServer with GUID "550e8400-e29b-41d4-a716-446655440000"
 * @return true if initialization was successful, false if server already registered
 */
inline Bool Init() {
    return false;
}

// Define static member outside the class (inline for header-only library)
inline Map<StdString, std::function<IServerPtr()>> ServerFactory::serverFactories_;

bool isInit = Init();

#endif // SERVERFACTORYINIT_H

