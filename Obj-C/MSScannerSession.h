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
#import <QuartzCore/QuartzCore.h>

#import "MSAvailability.h"
#import "MSScanner.h"
#import "MSImage.h"
#import "MSResult.h"
#import "MSCaptureSession.h"
#import "MSObjC.h"

@protocol MSScannerSessionDelegate;

/** Current scanner session state.
 */
typedef enum {
    MS_SCAN_STATE_DEFAULT = 0,
    MS_SCAN_STATE_SEARCH,
    MS_SCAN_STATE_PAUSE
} MSScanState;

/** The scanner session role is to process the incoming frames delivered by the camera
 * with the scanner and notify its delegate with scan results events.
 *
 * Frames are processed according to given scan options.
 */
@interface MSScannerSession : NSObject
<MSScannerDelegate, MSCaptureSessionDelegate>
{
    int _scanOptions;
    int _extras;
    MSResult *_result;
    int _losts;
    MSScanner *_scanner;
    BOOL _snap;
    MSScanState _state;
    MSCaptureSession *_captureSession;
#if __has_feature(objc_arc_weak)
    id<MSScannerSessionDelegate> __weak _delegate;
#elif __has_feature(objc_arc)
    id<MSScannerSessionDelegate> __unsafe_unretained _delegate;
#else
    id<MSScannerSessionDelegate> _delegate;
#endif
}

/** The scan options as a bitwise-or of scanning types (see `MSResult`).
 */
@property (nonatomic, assign) int scanOptions;
/** The extra information to attach to the results, as a bitwise-or of 
 * MSResultExtra flags (see `MSResult`)
 */
@property (nonatomic, assign) int extras;
#if __has_feature(objc_arc_weak)
/** The scanner session delegate that will be notified of scan results.
 */
@property (nonatomic, weak) id<MSScannerSessionDelegate> delegate;
#elif __has_feature(objc_arc)
@property (nonatomic, unsafe_unretained) id<MSScannerSessionDelegate> delegate;
#else
@property (nonatomic, assign) id<MSScannerSessionDelegate> delegate;
#endif
/** The current state of the scanner session.
 */
@property (nonatomic, readonly) MSScanState state;
/** The layer used to display the current video capture on-screen.
 */
@property (nonatomic, readonly) CALayer *previewLayer;
/** The flag that defines how to behave vs. device orientation events.
 *
 * If `YES`, the camera frames are re-oriented according to the current device orientation, e.g if the
 * device is in landscape mode, the frame is rotated by 90 degrees to make sure the scanner input
 * image reflects what the user sees on screen.
 *
 * If `NO` (default), the device orientation is ignored, and thus the frame is *always* handled with
 * portrait orientation.
 */
@property (nonatomic, assign) BOOL useDeviceOrientation;
/**
 * The flag to disable "partial matching".
 *
 * Set this flag to `YES` to avoid returning false positive results due to partial
 * matching, for example if several of the indexed images share the exact same
 * logo.
 *
 * By default, this value is set to `NO`.
 */
@property (nonatomic, assign) BOOL noPartialMatching;
/**
 * The flag to enable recognition on smaller or farther images.
 *
 * Set this flag to `YES` to boost the scale invariance of the Scanner, in order
 * to be able to recognize smaller images or images held farther from the phone
 * than in default mode.
 *
 * Slightly slower than the default mode.
 *
 * By default, this value is set to `NO`.
 */
@property (nonatomic, assign) BOOL smallTargetSupport;

///---------------------------------------------------------------------------------------
/// @name Initialization Methods
///---------------------------------------------------------------------------------------

/** Create a new scanner session.
 *
 * You should create a new `MSScannerSession` each time a scanner is
 * presented to the user.
 *
 * @param scanner the scanner instance used to process incoming frames.
 * @return the scanner session instance.
 */
- (id)initWithScanner:(MSScanner *)scanner;

///---------------------------------------------------------------------------------------
/// @name Video Capture Methods
///---------------------------------------------------------------------------------------

/** Setup and start the camera video capture.
 *
 * This is a pre-requisite so that the scanner session can start consuming
 * and scanning frames.
 */
- (void)startCapture;

/** Stop the video capture.
 *
 * Since no more frames are consumed, scanning is completely turned off.
 *
 * Also, if you plan to re-start a capture by calling `startCapture` again
 * take care to update the preview layer (`previewLayer`) on your view, since
 * a new layer is created each time `startCapture` is called.
 */
- (void)stopCapture;

/** Unfreeze the video capture after `pauseCapture` has been called.
 *
 * There is no need to call this method by default since `startCapture`
 * automatically calls it behind the scenes.
 */
- (void)playCapture;

/**  Freeze the video capture on-screen.
 *
 * Since no more frames are consumed, scanning is completely turned off.
 *
 * This method does not flush the preview layer (`previewLayer`).
 */
- (void)pauseCapture;

///---------------------------------------------------------------------------------------
/// @name Session State Methods
///---------------------------------------------------------------------------------------

/** Pause scanning.
 *
 * This has for effect to ignore any subsequent scan / snap calls until resume
 * is called.
 *
 * One cannot pause the scanner session if an API search is pending. You must first
 * call the `cancel` method.
 *
 * @return `YES` if the scanner session has been paused, `NO` otherwise.
 *
 */
- (BOOL)pause;

/** Resume scanning.
 *
 * This has for effect to start processing again any subsequent scan / snap calls
 *
 * @return `YES` if the scanner session has been resumed, `NO` otherwise.
 */
- (BOOL)resume;

/** Snap the next incoming query frame and perform an API search with it.
 *
 * The scanner session delegate will be notified of the API search life-cycle via
 * the `MSScannerDelegate` protocol.
 *
 * @return `YES` if the snap has correctly been deferred, `NO` otherwise.
 *
 * @warning **Note:** this method will trigger an API search and thus requires an Internet
 * connection. Also, note that you cannot defer a snap when the scanner session is paused.
 */
- (BOOL)snap;

/** Cancel any pending API search triggered by `snap`.
 *
 * @return `YES` if the cancelling has been applied, `NO` otherwise.
 */
- (BOOL)cancel;

@end

/** The protocol by which the delegate is kept informed of scan results.
 */
@protocol MSScannerSessionDelegate <NSObject, MSScannerDelegate>
/** Informs the `MSScannerSessionDelegate` that a frame has succesfully been scanned.
 * @param scanner the current `MSScannerSession`
 * @param result the found `MSResult` if any, `nil` otherwise.
 */
- (void)session:(MSScannerSession *)scanner didScan:(MSResult *)result;
@optional
/** Informs the `MSScannerSessionDelegate` that a frame could not be scanned.
 * @param scanner the current `MSScannerSession`
 * @param error the error that caused the scanning to fail.
 */
- (void)session:(MSScannerSession *)scanner failedToScan:(NSError *)error;
@end
