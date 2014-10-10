// Copyright (c) 2014 dacci.org

#ifndef LANSCANNER_MISC_NET_UTIL_H_
#define LANSCANNER_MISC_NET_UTIL_H_

#include <ws2tcpip.h>

#include <vector>

namespace net_util {

typedef struct {
  sockaddr_in address;
  int prefix;
} PrefixedAddress;

typedef bool (*NetworkScanCallback)(DWORD result, sockaddr_in* ip_address,
                                    BYTE* mac_address, void* param);

bool GetConnectedNetworks(std::vector<PrefixedAddress>* networks);
HANDLE ScanNetwork(const PrefixedAddress& target, NetworkScanCallback callback,
                   void* param);

}   // namespace net_util

#endif  // LANSCANNER_MISC_NET_UTIL_H_
