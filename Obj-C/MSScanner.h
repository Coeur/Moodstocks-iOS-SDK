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

#include "moodstocks_sdk.h"

#import "MSImage.h"
#import "MSResult.h"

@protocol MSScannerDelegate;

/** On-device image and barcode scanner.
 *
 * This class provides an unified interface to perform:
 *
 * - image signatures synchronization to the device,
 * - on-device image matching over the local database of image signatures, in **real time**,
 * - server-side image matching on Moodstocks API,
 * - on-device barcode decoding (EAN-8/EAN-13/QRCode/Datamatrix), in **real time**
 *
 * On-device image matching means that given a camera frame from a mobile device the
 * scanner retrieves the ID of the exact match (if any) within a database of reference
 * images while being invariant to common noises.
 *
 * Server-side image matching means the same, except the camera frame is sent to the
 * Moodtocks HTTP API for server-side recognition, which requires an Internet
 * connection.
 *
 * On-device barcode decoding means that given a camera frame from a mobile device that
 * contains a 1D or 2D barcode the scanner returns a string made of its raw content
 * while being invariant to common noises.
 *
 * @warning **Prerequisite:** you first need to register for a Moodstocks developper account
 * on https://developers.moodstocks.com/register and obtain an API key / secret pair. We provide
 * a **free plan**!
 */
@interface MSScanner : NSObject {
    NSString *_dbPath;
    ms_scanner_t *_scanner;
    NSOperationQueue *_syncQueue;
    NSMutableArray *_syncDelegates;
    NSOperationQueue *_searchQueue;
}

/** Internal scanner handle.
 */
@property (nonatomic, readonly) ms_scanner_t *handle;

/** Array of non-retained objects that receive messages about the current synchronization.
 * This is useful if you need to register *extra* delegate(s) that are supposed to be notified
 * each time a synchronization is triggered such as an UI component, etc
 */
@property (nonatomic, readonly) NSMutableArray *syncDelegates;

/** The main scanner instance (singleton).
 */
+ (MSScanner *)sharedInstance;

///---------------------------------------------------------------------------------------
/// @name Initialization and disposal
///---------------------------------------------------------------------------------------

/** Open the scanner and connect it to the database file.
 *
 * @param key a valid Moodstocks API key
 * @param secret a valid Moodstocks API secret
 * @param error the pointer to the error object, if any.
 * @return `YES` if it succeeded, `NO` otherwise.
 */
- (BOOL)openWithKey:(NSString *)key secret:(NSString *)secret error:(NSError **)error;

/** Similar to the above function, but allows to choose the database filename.
 *
 * You should only have to use it in an advanced context where the application needs
 * to manage several different API key/secret pairs.
 * @param key a valid Moodstocks API key
 * @param secret a valid Moodstocks API secret
 * @param filename the filename to use, without extension
 * @param error the pointer to the error object, if any.
 * @return `YES` if it succeeded, `NO` otherwise.
 */
- (BOOL)openWithKey:(NSString *)key secret:(NSString *)secret filename:(NSString *)filename error:(NSError **)error;

/** Close the scanner and disconnect it from the database file.
 *
 * @param error the pointer to the error object, if any.
 * @return `YES` if it succeeded, `NO` otherwise.
 */
- (BOOL)close:(NSError **)error;

///---------------------------------------------------------------------------------------
/// @name Synchronization Methods
///---------------------------------------------------------------------------------------

/** Synchronize the image signatures from Moodstocks API to the device.
 *
 * This method runs in the background so you can safely call it from the main thread.
 *
 * @param delegate the delegate that will be notified with the synchronization events.
 *
 * @warning **Note:** this method requires an Internet connection.
 */
- (void)syncWithDelegate:(id<MSScannerDelegate>)delegate;

/** Check if a synchronization is pending.
 *
 * @return `YES` if a synchronization is pending, `NO` otherwise.
 */
- (BOOL)isSyncing;

///---------------------------------------------------------------------------------------
/// @name Information Methods
///---------------------------------------------------------------------------------------

/** Get the total number of images recorded within the local database.
 *
 * @param error the pointer to the error object, if any.
 * @return the number of image signatures found within the local database, or -1 if an error occurred.
 */
- (NSInteger)count:(NSError **)error;

/** Get an array made of all images identifiers found into the local database.
 *
 * @param error the pointer to the error object, if any.
 * @return the array of `NSString` identifiers for each image found within the local database.
 */
- (NSArray *)info:(NSError **)error;

///---------------------------------------------------------------------------------------
/// @name Server-side Image Matching Methods
///---------------------------------------------------------------------------------------

/** Perform remote image search on Moodstocks API (a.k.a server-side image matching).
 *
 * This method runs in the background so you can safely call it from the main thread.
 *
 * @param qry the query image.
 * @param delegate the delegate that will be notified with the search events.
 *
 * @warning **Note:** this method requires an Internet connection.
 */
- (void)apiSearch:(MSImage *)qry withDelegate:(id<MSScannerDelegate>)delegate;

/** Cancel any pending API search(es).
 */
- (void)cancelApiSearch;

///---------------------------------------------------------------------------------------
/// @name On-device Image Matching Methods
///---------------------------------------------------------------------------------------

/** Perform on-device image matching over the local database.
 *
 * @param qry the query image.
 * @param error the pointer to the error object, if any.
 * @return the result found, or `nil` if there is none or an error occurred.
 */
- (MSResult *)search:(MSImage *)qry error:(NSError **)error;

/** Perform on-device image matching over the local database, with optional options.
 *
 * There are currently 2 usable options, defined in the `ms_search_flag_t` enum
 * in `moodstocks_sdk.h`:
 *
 * - `MS_SEARCH_NOPARTIAL` disables "partial matching" in order to avoid returning
 * false positive results, for example if several of the indexed images share the
 * exact same logo.
 *
 * - `MS_SEARCH_SMALLTARGET` boosts scale invariance in order to allow smaller or farther
 * images to be recognized. Slightly slower than the default mode.
 *
 * @param qry the query image.
 * @param options a bitwise-OR combination of the `ms_search_flag_t` options.
 * @param error the pointer to the error object, if any.
 * @return the result found, or `nil` if there is none or an error occurred.
 */
- (MSResult *)search2:(MSImage *)qry options:(int)options error:(NSError **)error;

/** Match a query image against a given local reference.
 *
 * @param qry the query image.
 * @param ref the reference (a.k.a identifier) of the local image to match against.
 * @param error the pointer to the error object, if any.
 * @return `YES` if both images match, `NO` otherwise.
 */
- (MSResult *)match:(MSImage *)qry ref:(MSResult *)ref error:(NSError **)error;

/** Match a query image against a given local reference, with additional options.
 *
 * See the documentation for `search2:options:error`.
 *
 * @param qry the query image.
 * @param ref the reference (a.k.a identifier) of the local image to match against.
 * @param options a bitwise-OR combination of the `ms_search_flag_t` options.
 * @param error the pointer to the error object, if any.
 * @return `YES` if both images match, `NO` otherwise.
 */
- (MSResult *)match2:(MSImage *)qry ref:(MSResult *)ref options:(int)options error:(NSError **)error;

///---------------------------------------------------------------------------------------
/// @name On-device Barcode Decoding Methods
///---------------------------------------------------------------------------------------

/** Perform on-device barcode decoding.
 *
 * @param qry the query image.
 * @param formats the flag that represents the formats to be decoded. See `MSResult`for the
 * list of available format. Each format must be added by bitwise-or, e.g
 * `MS_RESULT_TYPE_QRCODE | MS_RESULT_TYPE_DMTX`
 * @param error the pointer to the error object, if any.
 * @return the result found, or `nil` if there is none or an error occurred.
 */
- (MSResult *)decode:(MSImage *)qry formats:(int)formats error:(NSError **)error;

@end

/** Scanner protocol for asynchronous network operations.
 *
 * @warning **Note:** these methods are always called on main thread.
 */
@protocol MSScannerDelegate <NSObject>
@optional
/** Dispatched when a synchronization is about to start.
 *
 * @param scanner the corresponding scanner instance.
 */
- (void)scannerWillSync:(MSScanner *)scanner;

/** Dispatched as soon as the synchronization progresses.
 *
 * @param current specifies how many image signatures have been fetched so far.
 * @param total specifies the total number of images signatures that have to be fetched.
 */
- (void)didSyncWithProgress:(NSNumber *)current total:(NSNumber *)total;

/** Dispatched when a synchronization is completed.
 *
 * @param scanner the corresponding scanner instance.
 */
- (void)scannerDidSync:(MSScanner *)scanner;

/** Dispatched when a synchronization failed.
 *
 * @param scanner the corresponding scanner instance.
 * @param error the error object.
 */
- (void)scanner:(MSScanner *)scanner failedToSyncWithError:(NSError *)error;

/** Dispatched when the scanner is about to start an server-side search.
 *
 * @param scanner the corresponding scanner instance.
 */
- (void)scannerWillSearch:(MSScanner *)scanner;

/** Dispatched when a server-side search is completed.
 *
 * @param scanner the corresponding scanner instance.
 * @param result the result found, if any.
 * @warning **Note:** the returned result is `nil` if there is no match found.
 */
- (void)scanner:(MSScanner *)scanner didSearchWithResult:(MSResult *)result;

/** Dispatched when a server-side search failed.
 *
 * @param scanner the corresponding scanner instance.
 * @param error the error object.
 */
- (void)scanner:(MSScanner *)scanner failedToSearchWithError:(NSError *)error;
@end
