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

#import "MSScanner.h"
#import "MSImage.h"

/** A server-side image matching operation.
 *
 * You should never have to handle such an operation directly.
 * Instead use the `[MSScanner apiSearch:withDelegate:]` method.
 */
@interface MSApiSearch : NSOperation {
    MSScanner *_scanner;
    MSImage *_query;
    ms_api_handle_t *_request;
#if __has_feature(objc_arc_weak)
    id<MSScannerDelegate> __weak _delegate;
#elif __has_feature(objc_arc)
    id<MSScannerDelegate> __unsafe_unretained _delegate;
#else
    id<MSScannerDelegate> _delegate;
#endif
}

/** The delegate to be notified with API search events.
 */
#if __has_feature(objc_arc_weak)
@property (nonatomic, weak) id<MSScannerDelegate> delegate;
#elif __has_feature(objc_arc)
@property (nonatomic, unsafe_unretained) id<MSScannerDelegate> delegate;
#else
@property (nonatomic, assign) id<MSScannerDelegate> delegate;
#endif

///---------------------------------------------------------------------------------------
/// @name Initialization Methods
///---------------------------------------------------------------------------------------

/** Initialize an API search operation
 *
 * @param scanner the corresponding scanner instance.
 * @param qry the query image.
 * @return the API search operation.
 */
- (id)initWithScanner:(MSScanner *)scanner query:(MSImage *)qry;

@end
