/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_BASE_IFADDRS_CONVERTER_H_
#define WEBRTC_BASE_IFADDRS_CONVERTER_H_

#if defined(WEBRTC_ANDROID)
#include "webrtc/base/ifaddrs-android.h"
#else
#include <ifaddrs.h>
#endif  // WEBRTC_ANDROID

#include "webrtc/base/ipaddress.h"

namespace rtc {

// This class converts native interface addresses to our internal IPAddress
// class. Subclasses should override ConvertNativeToIPAttributes to implement
// the different ways of retrieving IPv6 attributes for various POSIX platforms.
class IfAddrsConverter {
 public:
  IfAddrsConverter();
  virtual ~IfAddrsConverter();
  virtual bool ConvertIfAddrsToIPAddress(const struct ifaddrs* interface,
                                         InterfaceAddress* ipaddress,
                                         IPAddress* mask);

 protected:
  virtual bool ConvertNativeAttributesToIPAttributes(
      const struct ifaddrs* interface,
      int* ip_attributes);
};

IfAddrsConverter* CreateIfAddrsConverter();

}  // namespace rtc

#endif  // WEBRTC_BASE_IFADDRS_CONVERTER_H_
