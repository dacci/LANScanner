// Copyright (c) 2014 dacci.org

#include "misc/net_util.h"

#include <stdint.h>
#include <iphlpapi.h>

#include <memory>

namespace net_util {

namespace {

typedef struct {
  uint32_t network;
  uint32_t broadcast;
  NetworkScanCallback callback;
  void* param;
} ScanProgress;

}   // namespace

bool GetConnectedNetworks(std::vector<PrefixedAddress>* networks) {
  if (networks == nullptr)
    return false;

  ULONG size = 0;
  ULONG error = GetAdaptersAddresses(AF_INET, 0, nullptr, nullptr, &size);
  if (error != ERROR_BUFFER_OVERFLOW)
    return false;

  IP_ADAPTER_ADDRESSES* adapters =
      static_cast<IP_ADAPTER_ADDRESSES*>(malloc(size));
  if (adapters == nullptr)
    return false;

  error = GetAdaptersAddresses(AF_INET, 0, nullptr, adapters, &size);
  if (error != ERROR_SUCCESS) {
    free(adapters);
    return false;
  }

  for (auto adapter = adapters; adapter; adapter = adapter->Next) {
    if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
      continue;

    for (auto unicast = adapter->FirstUnicastAddress; unicast;
        unicast = unicast->Next) {
      networks->push_back(PrefixedAddress());
      PrefixedAddress& address = networks->back();

      memmove(&address.address, unicast->Address.lpSockaddr,
              unicast->Address.iSockaddrLength);
      address.prefix = unicast->OnLinkPrefixLength;
    }
  }

  free(adapters);

  return true;
}

static DWORD CALLBACK ScanNetwork(void* context) {
  std::unique_ptr<ScanProgress> progress(static_cast<ScanProgress*>(context));

  std::unique_ptr<BYTE[]> mac_address(new BYTE[8]);
  if (mac_address == nullptr)
    return __LINE__;

  sockaddr_in address = { AF_INET };

  for (uint32_t current = progress->network + 1; current < progress->broadcast;
      ++current) {
    address.sin_addr.s_addr = _byteswap_ulong(current);

    ULONG length = 8;
    DWORD result = SendARP(address.sin_addr.s_addr, INADDR_ANY,
                           mac_address.get(), &length);

    if (!progress->callback(result, &address, mac_address.get(),
                            progress->param))
      break;
  }

  progress->callback(ERROR_NO_MORE_ITEMS, nullptr, nullptr, progress->param);

  return 0;
}

HANDLE ScanNetwork(const PrefixedAddress& target, NetworkScanCallback callback,
                   void* param) {
  uint32_t mask = 0xFFFFFFFF << (32 - target.prefix);
  uint32_t address = _byteswap_ulong(target.address.sin_addr.s_addr);

  std::unique_ptr<ScanProgress> progress(new ScanProgress());
  progress->network = address & mask;
  progress->broadcast = progress->network | (0xFFFFFFFF & ~mask);
  progress->callback = callback;
  progress->param = param;

  HANDLE thread = CreateThread(nullptr, 0, ScanNetwork, progress.get(), 0,
                               nullptr);
  if (thread == NULL)
    return NULL;

  progress.release();

  return thread;
}

}   // namespace net_util
