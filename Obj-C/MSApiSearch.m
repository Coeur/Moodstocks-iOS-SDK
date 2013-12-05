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
#import "MSApiSearch.h"
#import "MSObjC.h"

@interface MSApiSearch ()
- (void)willSearch;
- (void)didSearchWithResult:(MSResult *)result;
- (void)failedToSearchWithError:(NSError *)error;
@end

@implementation MSApiSearch

@synthesize delegate = _delegate;

- (id)initWithScanner:(MSScanner *)scanner query:(MSImage *)qry {
    self = [super init];
    if (self) {
        _scanner = scanner;
        _query = [qry retain_stub];
        _request = NULL;
        _delegate = nil;
        
    }
    return self;
}

- (void)dealloc {
    _scanner = nil;
    [_query release_stub];
    _query = nil;
    _delegate = nil;
    
#if ! __has_feature(objc_arc)
    [super dealloc];
#endif
}

- (void)cancel {
    NSError *error = [NSError errorWithDomain:@"moodstocks-sdk" code:-1 /* cancel error */ userInfo:nil];
    [self performSelectorOnMainThread:@selector(failedToSearchWithError:) withObject:error waitUntilDone:YES];
    
#if MS_SDK_REQUIREMENTS
    if (_request) ms_api_handle_cancel(_request);
#endif

    [super cancel];
}

- (void)main {
#if __has_feature(objc_arc)
    @autoreleasepool {
#else
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
#endif
    
#if MS_SDK_REQUIREMENTS
    MSResult *result = nil;
    NSError *error = nil;

    if (![self isCancelled]) {
        [self performSelectorOnMainThread:@selector(willSearch) withObject:nil waitUntilDone:YES];

        ms_result_t *res = NULL;
        ms_errcode ecode = ms_scanner_api_handle([_scanner handle], &_request);
        if (ecode == MS_SUCCESS)
            ecode = ms_api_handle_search(_request, [_query image], &res);
        
        if (ecode == MS_SUCCESS) {
            if (res != NULL) {
                result = [[[MSResult alloc] initWithResult:res] autorelease_stub];
                ms_result_del(res);
            }
        }
        else {
            error = [NSError errorWithDomain:@"moodstocks-sdk" code:ecode userInfo:nil];
        }
        
        if (_request != NULL) {
            ms_api_handle_release(_request);
            _request = NULL;
        }
    }

    if (![self isCancelled]) {
        if (!error) {
            [self performSelectorOnMainThread:@selector(didSearchWithResult:) withObject:result waitUntilDone:YES];
        }
        else {
            [self performSelectorOnMainThread:@selector(failedToSearchWithError:) withObject:error waitUntilDone:YES];
        }
    }
#endif
    
#if __has_feature(objc_arc)
    } /* end of @autoreleasepool block */
#else
    [pool release];
#endif
}

#pragma mark - Private

- (void)willSearch {
    if ([_delegate respondsToSelector:@selector(scannerWillSearch:)]) {
        [_delegate scannerWillSearch:_scanner];
    }
}

- (void)didSearchWithResult:(MSResult *)result {
    if ([_delegate respondsToSelector:@selector(scanner:didSearchWithResult:)]) {
        [_delegate scanner:_scanner didSearchWithResult:result];
    }
}

- (void)failedToSearchWithError:(NSError *)error {
    if ([_delegate respondsToSelector:@selector(scanner:failedToSearchWithError:)]) {
        [_delegate scanner:_scanner failedToSearchWithError:error];
    }
}

@end
