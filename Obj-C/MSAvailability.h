/**
 * Copyright (c) 2013 Moodstocks SAS
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#import <Foundation/Foundation.h>

/** Compile-time requirements checks.
 *
 * - prevent compatibility when Xcode Base SDK is lower than 4.0 (AVFoundation
 *   framework is required),
 * - restrict to ARMv7 architecture only, which excludes e.g. iPhone 3G (ARMv6)
 *   and the iPhone Simulator (that anyway lacks of video capture support).
 *
 * If compile-time requirements are not fulfilled the scanner operations are replaced
 * by NOP-s
 */
#if (__IPHONE_OS_VERSION_MAX_ALLOWED >= 40000)
  #define MS_IPHONE_OS_REQUIREMENTS 1
#else
  #define MS_IPHONE_OS_REQUIREMENTS 0
#endif

#if __ARM_NEON__ && MS_IPHONE_OS_REQUIREMENTS
  #define MS_SDK_REQUIREMENTS 1
#else
  #define MS_SDK_REQUIREMENTS 0
#endif

/** Run-time requirements checks.
 *
 * This function indicates whether the SDK can be used on the current device.
 *
 * It performs a runtime check over the iOS version to prevent using unavailable
 * frameworks and APIs (iOS 4.0 or higher is required at runtime for AVFoundation,
 * multi-tasking and Grand Central Dispatch support).
 *
 * It returns YES if the current device is compatible with the SDK requirements, and
 * NO otherwise.
 *
 * We greatly recommend using this function as soon as possible at runtime and
 * design your application so that it will *NOT* attempt to use the SDK (MSScanner, etc)
 * at all if NO is returned.
 */
BOOL MSDeviceCompatibleWithSDK(void);
