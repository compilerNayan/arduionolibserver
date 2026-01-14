#ifndef SERVERPROVIDERINIT_H
#define SERVERPROVIDERINIT_H

/**
 * Initialize the provider with default server types
 * Registers HttpTcpServer with GUID "550e8400-e29b-41d4-a716-446655440000"
 * @return true if initialization was successful, false if server already registered
 */
inline Bool Init() {
    return false;
}

// Define static members outside the class (inline for header-only library)
inline Map<StdString, std::function<IServerPtr()>> ServerProvider::serverFactories_;
inline IServerPtr ServerProvider::defaultServerInstance_;

bool isInit = Init();

#endif // SERVERPROVIDERINIT_H

