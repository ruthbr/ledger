#ifndef NETWORK_NODE_CORE_HPP
#define NETWORK_NODE_CORE_HPP

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "http/middleware/allow_origin.hpp"
#include "http/middleware/color_log.hpp"
#include "http/server.hpp"
#include "network/service/client.hpp"
#include "network/service/server.hpp"
#include "network/swarm/swarm_peer_location.hpp"

namespace fetch
{

namespace network
{

class NetworkNodeCore
{
public:
    typedef std::recursive_mutex mutex_type;
    typedef std::lock_guard<mutex_type> lock_type;
    typedef fetch::service::ServiceClient<network::TCPClient> client_type;
    typedef uint32_t protocol_number_type;

public:
    NetworkNodeCore(const NetworkNodeCore &rhs)           = delete;
    NetworkNodeCore(NetworkNodeCore &&rhs)           = delete;
    NetworkNodeCore operator=(const NetworkNodeCore &rhs)  = delete;
    NetworkNodeCore operator=(NetworkNodeCore &&rhs) = delete;
    bool operator==(const NetworkNodeCore &rhs) const = delete;
    bool operator<(const NetworkNodeCore &rhs) const = delete;

    explicit NetworkNodeCore(
                             size_t threads,
                             uint16_t httpPort,
                             uint16_t rpcPort
                             )
        :
        tm_(threads)
    {
        lock_type mlock(mutex_);
        tm_. Start();

        rpcPort_ = rpcPort;
        rpcServer_ = std::make_shared<service::ServiceServer<
            fetch::network::TCPServer>>(rpcPort, tm_);

        httpServer_ = std::make_shared<fetch::http::HTTPServer>(httpPort, tm_);

        // Add middleware to the HTTP server - allow requests from any address,
        // and print requests to the terminal in colour
        httpServer_->AddMiddleware(fetch::http::middleware::AllowOrigin("*"));
        httpServer_->AddMiddleware(fetch::http::middleware::ColorLog);
    }

    virtual ~NetworkNodeCore()
    {
    }

    typedef std::pair<std::string, int> remote_host_identifier_type;
    typedef std::shared_ptr<client_type> client_ptr;
    typedef std::map<remote_host_identifier_type, client_ptr> cache_type;

    cache_type cache_;

    typedef std::shared_ptr<fetch::service::Protocol> protocol_ptr;
    typedef std::map<uint32_t, protocol_ptr> protocol_cache_type;

    virtual std::shared_ptr<client_type> ConnectToPeer(
        const fetch::swarm::SwarmPeerLocation &peer)
    {
        return ConnectTo(peer.GetHost(), peer.GetPort());
    }

    virtual std::shared_ptr<client_type> ConnectTo(const std::string &host)
    {
        return ConnectToPeer(fetch::swarm::SwarmPeerLocation(host));
    }

    virtual client_ptr ConnectTo(const std::string &host, unsigned short port)
    {
        lock_type mlock(mutex_);
        std::cout << "ConnectTo " << host << ":" << port << "" << std::endl;
        auto remote_host_identifier = std::make_pair(host, port);
        auto iter = cache_.find(remote_host_identifier);
        if (iter != cache_.end())
        {
            std::cout << "ConnectTo " << host << ":" << port << " FOUND" << std::endl;
            if (iter -> second)
            {
                return iter -> second;
            }
            else
            {
                std::cout << "ConnectTo " << host << ":" << port << " dead" << std::endl;
                cache_.erase(remote_host_identifier);
            }
        }
        std::cout << "ConnectTo " << host << ":" << port << " making" << std::endl;
        auto new_client_conn = ActuallyConnectTo(host, port);
        if (new_client_conn)
        {
            std::cout << "ConnectTo " << host << ":" << port << " stored" << std::endl;
            cache_[remote_host_identifier] = new_client_conn;
        }
        return new_client_conn;
    }

    virtual client_ptr ActuallyConnectTo(
        const std::string &host, unsigned short port)
    {
        lock_type mlock(mutex_);
        client_ptr client = std::make_shared<client_type>(host, port, tm_);

        std::cout << "ActuallyConnectTo " << host << ":" << port << std::endl;

        int waits = 100;
        while(!client->is_alive())
        {
            usleep(100);
            waits--;
            if (waits <= 0)
            {
                std::cout << "ActuallyConnectTo " << host << ":" << port << " - FAIL" << std::endl;
                throw std::invalid_argument(std::string("Timeout while connecting " + host + ":" + std::to_string(port)).c_str());
            }
        }
        std::cout << "ActuallyConnectTo " << host << ":" << port << " - ALIVE" << std::endl;
        return client;
    }

    void Start()
    {
        // We do nothing here. It exists only
        // for symmetry with the Stop().
    }

    void Stop()
    {
        tm_ . Stop();
    }

    void AddModule(fetch::http::HTTPModule *handler)
    {
        lock_type mlock(mutex_);
        httpServer_->AddModule(*handler);
    }

    template<class MODULE>
    void AddModule(std::shared_ptr<MODULE> module_p)
    {
        lock_type mlock(mutex_);
        fetch::http::HTTPModule *h = module_p.get();
        AddModule(h);
    }

    template<class INTERFACE_CLASS, class PROTOCOL_CLASS>
    void AddProtocol(INTERFACE_CLASS *interface, uint32_t protocolNumber)
    {
        lock_type mlock(mutex_);
        auto protocolInstance = std::make_shared<PROTOCOL_CLASS>(interface);
        auto baseProtocolPtr = std::static_pointer_cast<fetch::service::Protocol>(protocolInstance);
        protocolCache_[protocolNumber] = baseProtocolPtr;
        PROTOCOL_CLASS *proto_ptr = protocolInstance.get();
        fetch::service::Protocol *base_ptr = proto_ptr;
        rpcServer_ -> Add(protocolNumber, base_ptr);
    }

    template<class INTERFACE_CLASS>
    void AddProtocol(INTERFACE_CLASS *interface, uint32_t protocolNumber)
    {
        lock_type mlock(mutex_);
        AddProtocol<INTERFACE_CLASS,
                    typename INTERFACE_CLASS::protocol_class_type>(
            interface, protocolNumber);
    }

    template<class INTERFACE_CLASS>
    void AddProtocol(std::shared_ptr<INTERFACE_CLASS> interface)
    {
        lock_type mlock(mutex_);
        auto protocolNumber = INTERFACE_CLASS::protocol_number;
        INTERFACE_CLASS *interface_ptr = interface.get();
        AddProtocol<INTERFACE_CLASS>(interface_ptr, protocolNumber);
    }

    template<class INTERFACE_CLASS>
    void AddProtocol(INTERFACE_CLASS *interface)
    {
        lock_type mlock(mutex_);
        auto protocolNumber = INTERFACE_CLASS::protocol_number;
        AddProtocol<INTERFACE_CLASS>(interface, protocolNumber);
    }

    template<class INTERFACE_CLASS>
    INTERFACE_CLASS *GetProtocol()
    {
        lock_type mlock(mutex_);
        // Nasty. TODO(katie) Make this not suck later.
        auto protocolNumber = INTERFACE_CLASS::protocol_number;
        return (INTERFACE_CLASS*)(interfaces_[protocolNumber]);
    }
    
    virtual void Post(std::function<void ()> workload)
    {
        tm_ . Post(workload);
    }

protected:
    fetch::network::ThreadManager tm_;
    uint16_t rpcPort_;
    mutex_type mutex_;
    std:: map<protocol_number_type, void*> interfaces_;
    std::shared_ptr<fetch::http::HTTPServer> httpServer_;
    std::shared_ptr<
        service::ServiceServer<
            fetch::network::TCPServer>> rpcServer_;
    protocol_cache_type protocolCache_;
};

}
}

#endif //NETWORK_NODE_CORE_HPP
