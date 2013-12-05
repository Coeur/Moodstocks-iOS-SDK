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

#import "MSScannerSession.h"

@interface MSScannerSession ()

- (MSResult *)scan:(MSImage *)qry options:(int)options error:(NSError **)error;
- (void)reset;

@end

@implementation MSScannerSession

@synthesize scanOptions = _scanOptions;
@synthesize delegate = _delegate;
@synthesize state = _state;

- (id)initWithScanner:(MSScanner *)scanner {
    self = [super init];
    if (self) {
        _scanOptions = MS_RESULT_TYPE_IMAGE;
        _extras = MS_RESULT_EXTRA_NONE;
        _result = nil;
        _losts = 0;
        _snap = NO;
        _state = MS_SCAN_STATE_DEFAULT;
        _scanner = scanner;
        _captureSession = [[MSCaptureSession alloc] init];
        _delegate = nil;
    }
    return self;
}

- (void)dealloc {
    [_result release_stub];
    _result = nil;

    [_captureSession release_stub];

    _delegate = nil;

#if ! __has_feature(objc_arc)
    [super dealloc];
#endif
}

- (void)reset {
    [_result release_stub];
    _result = nil;
    _losts = 0;
    _snap = NO;
}

- (CALayer *)previewLayer {
    CALayer *layer = nil;
#if MS_IPHONE_OS_REQUIREMENTS
    layer = [_captureSession previewLayer];
#endif
    return layer;
}

- (void)startCapture {
    [_captureSession setDelegate:self];
    [_captureSession start];
}

- (void)stopCapture {
    [_captureSession stop];
    [_captureSession setDelegate:nil];
}

- (void)playCapture {
    [_captureSession play];
}

- (void)pauseCapture {
    [_captureSession pause];
}

- (BOOL)pause {
    if (_state != MS_SCAN_STATE_DEFAULT) return NO;
    _state = MS_SCAN_STATE_PAUSE;
    return YES;
}

- (BOOL)resume {
    if (_state != MS_SCAN_STATE_PAUSE) return NO;
    [self reset];
    _state = MS_SCAN_STATE_DEFAULT;
    return YES;
}

- (MSResult *)scan:(MSImage *)qry options:(int)options error:(NSError **)error {
    MSResult *result = nil;
#if MS_SDK_REQUIREMENTS
    int flags = (self.noPartialMatching ? MS_SEARCH_NOPARTIAL : 0) |
                (self.smallTargetSupport ? MS_SEARCH_SMALLTARGET : 0);
    BOOL lock = NO;
    MSResult *rlock = nil;
    if (_result != nil && _losts < 2) {
        int _resultType = [_result getType];
        NSInteger found = 0;
        if (_resultType == MS_RESULT_TYPE_IMAGE) {
            if (flags)
                rlock = [_scanner match2:qry ref:_result options:flags error:nil];
            else
                rlock = [_scanner match:qry ref:_result error:nil];
            found = (rlock != nil) ? 1 : -1;
        }
        else if (_resultType == MS_RESULT_TYPE_QRCODE) {
            rlock = [_scanner decode:qry formats:MS_RESULT_TYPE_QRCODE error:nil];
            found = [rlock isEqualToResult:_result] ? 1 : -1;
        }
        else if (_resultType == MS_RESULT_TYPE_DMTX) {
            rlock = [_scanner decode:qry formats:MS_RESULT_TYPE_DMTX error:nil];
            found = [rlock isEqualToResult:_result] ? 1 : -1;
        }

        if (found == 1) {
            // The current frame matches with the previous result
            lock = YES;
            _losts = 0;
        }
        else if (found == -1) {
            // The current frame looks different so release the lock
            // if there is enough consecutive "no match"
            _losts++;
            lock = (_losts >= 2) ? NO : YES;
        }
    }

    if (lock) {
        if (rlock != nil) {
            // If the result was found: update result to have latest geometry.
            result = rlock;
        }
        else {
            // Otherwise, return non-updated previous result
            result = [[_result copy] autorelease_stub];
        }
    }

    // -------------------------------------------------
    // Image search
    // -------------------------------------------------
    if (result == nil && (options & MS_RESULT_TYPE_IMAGE)) {
        NSError *err  = nil;
        if (flags)
            result = [_scanner search2:qry options:flags error:&err];
        else
            result = [_scanner search:qry error:&err];
        if (err != nil && [err code] != MS_EMPTY) {
            if (error) *error = err;
            return nil;
        }
        if (result != nil) {
            _losts = 0;
        }
    }

    // -------------------------------------------------
    // Barcode decoding
    // -------------------------------------------------
    if (result == nil) {
        NSError *err  = nil;
        result = [_scanner decode:qry formats:options error:&err];
        if (err != nil) {
            if (error) *error = err;
            return nil;
        }
        if (result != nil) {
            _losts = 0;
        }
    }


    // Store current result.
    [_result release_stub];
    _result = [result copy];

#endif
    return result;
}

- (BOOL)snap {
    if (_state != MS_SCAN_STATE_DEFAULT) return NO;
    _snap = YES;
    return YES;
}

- (BOOL)cancel {
    if (_state != MS_SCAN_STATE_SEARCH) return NO;
    [_scanner cancelApiSearch];
    return YES;
}

#pragma mark - MSCaptureSessionDelegate

#if MS_IPHONE_OS_REQUIREMENTS
- (void)session:(MSCaptureSession *)session didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer {
    if (_state != MS_SCAN_STATE_DEFAULT) return;

    AVCaptureVideoOrientation orientation = (self.useDeviceOrientation) ? session.orientation : AVCaptureVideoOrientationPortrait;
    MSImage *qry = [[MSImage alloc] initWithBuffer:sampleBuffer orientation:orientation];

    if (_snap) {
        _snap = NO;
        _state = MS_SCAN_STATE_SEARCH;
        [_scanner apiSearch:qry withDelegate:self];
        [qry release_stub];
        return;
    }

    NSError *error = nil;
    MSResult *result = [self scan:qry options:_scanOptions error:&error];
    if (!error) {
        if (result) {
            if (_extras & MS_RESULT_EXTRA_IMAGE) {
                CGImageRef frame = [MSImage newCGImageFromBuffer:sampleBuffer];
                [result setImage:frame withOrientation:orientation];
                CGImageRelease(frame);
            }
        }
        [_delegate session:self didScan:result];
    }
    else if ([_delegate respondsToSelector:@selector(session:failedToScan:)])
        [_delegate performSelector:@selector(session:failedToScan:) withObject:error];

    [qry release_stub];
}
#endif

#pragma mark - MSScannerDelegate

- (void)scannerWillSearch:(MSScanner *)scanner {
    [self reset];

    if ([_delegate respondsToSelector:@selector(scannerWillSearch:)]) {
        [_delegate scannerWillSearch:_scanner];
    }
}

- (void)scanner:(MSScanner *)scanner didSearchWithResult:(MSResult *)result {
    _state = MS_SCAN_STATE_DEFAULT;

    if ([_delegate respondsToSelector:@selector(scanner:didSearchWithResult:)]) {
        [_delegate scanner:_scanner didSearchWithResult:result];
    }
}

- (void)scanner:(MSScanner *)scanner failedToSearchWithError:(NSError *)error {
    _state = MS_SCAN_STATE_DEFAULT;

    if ([_delegate respondsToSelector:@selector(scanner:failedToSearchWithError:)]) {
        [_delegate scanner:_scanner failedToSearchWithError:error];
    }
}

@end
