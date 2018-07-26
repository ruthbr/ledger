#include "network/adapters.hpp"

#include <ifaddrs.h>
#include <sys/types.h>

namespace fetch {
namespace network {

Adapter::adapter_list_type Adapter::GetAdapters()
{
  adapter_list_type adapters;

  struct ifaddrs *iflist_start = nullptr;
  if (::getifaddrs(&iflist_start))
  {
    throw std::runtime_error("Unable to query interface information from operating system");
  }

  for (struct ifaddrs *curr = iflist_start; curr; curr = curr->ifa_next)
  {

    // extract basic flag information
    auto const is_down     = !static_cast<bool>(curr->ifa_flags & IFF_UP);
    auto const is_loopback = static_cast<bool>(curr->ifa_flags & IFF_LOOPBACK);

    // discard all interfaces which are down or are just loopback
    if (is_down || is_loopback) continue;

    // discard all interfaces which are not INET based
    bool const is_inet = (curr->ifa_addr && curr->ifa_addr->sa_family == AF_INET);
    if (!is_inet) continue;

    auto *inet_addr     = reinterpret_cast<struct sockaddr_in *>(curr->ifa_addr);
    auto *net_mask_addr = reinterpret_cast<struct sockaddr_in *>(curr->ifa_netmask);

    // extract the address
    address_type const address{ntohl(inet_addr->sin_addr.s_addr)};
    address_type const netmask{ntohl(net_mask_addr->sin_addr.s_addr)};

    adapters.emplace_back(address, netmask);
  }

  ::freeifaddrs(iflist_start);

  return adapters;
}

}  // namespace network
}  // namespace fetch