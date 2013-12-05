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

#include "moodstocks_sdk.h"

#import "MSAvailability.h"

/** Debug-only general purpose logger
 */
#ifdef DEBUG
  #define MSDLog(...) NSLog(__VA_ARGS__)
#else
  #define MSDLog(...) ((void)0)
#endif

/** Error code to message string
 */
#if MS_SDK_REQUIREMENTS
  #define MSErrMsg(MS_ecode) \
    [NSString stringWithCString:ms_errmsg((int) (MS_ecode)) encoding:NSUTF8StringEncoding]
#else
  #define MSErrMsg(MS_ecode) \
    [NSString stringWithFormat:@"error (%d)", (int) (MS_ecode)]
#endif
