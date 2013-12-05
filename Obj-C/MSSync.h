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

#import "MSAvailability.h"
#import "MSScanner.h"

/** An image signatures synchronization operation.
 *
 * You should never have to handle such an operation directly.
 * Instead use the `[MSScanner syncWithDelegate:]` method.
 */
@interface MSSync : NSOperation {
    MSScanner *_scanner;
#if __has_feature(objc_arc_weak)
    id<MSScannerDelegate> __weak _delegate;
#elif __has_feature(objc_arc)
    id<MSScannerDelegate> __unsafe_unretained _delegate;
#else
    id<MSScannerDelegate> _delegate;
#endif
#if MS_IPHONE_OS_REQUIREMENTS
    UIBackgroundTaskIdentifier _taskID;
#endif
}

/** The delegate to be notified with synchronization events.
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

/** Initialize a synchronization operation
 *
 * @param scanner the corresponding scanner instance.
 * @return the synchronization operation.
 */
- (id)initWithScanner:(MSScanner *)scanner;

@end
