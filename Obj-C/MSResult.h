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
#if MS_IPHONE_OS_REQUIREMENTS
    #import <AVFoundation/AVFoundation.h>
#endif

/** Scanning types.
 */
typedef ms_result_type MSResultType;

/** Extra information to attach to the results of a scan.
 */
typedef enum {
    MS_RESULT_EXTRA_NONE = 0,
    MS_RESULT_EXTRA_IMAGE
} MSResultExtra;

/** Result of a scan.
 *
 * A result is composed of:
 *
 * - its type among those listed in the enum below,
 * - its value as a string or raw data that may represent.
 *
 * The value may be:
 *
 * - an image ID when the type is `MS_RESULT_TYPE_IMAGE`,
 * - a barcode numbers when the type is `MS_RESULT_TYPE_EAN8` or `MS_RESULT_TYPE_EAN13`,
 * - raw QR Code data, i.e. *unparsed*, when type is `MS_RESULT_TYPE_QRCODE` or `MS_RESULT_TYPE_DMTX`.
 *
 * The result types (a.k.a scanning types) are defined by `moodstocks_sdk.h`:
 *
 * - `MS_RESULT_TYPE_EAN8`     -> EAN8 linear barcode
 * - `MS_RESULT_TYPE_EAN13`    -> EAN13 linear barcode
 * - `MS_RESULT_TYPE_QRCODE`   -> QR Code 2D barcode
 * - `MS_RESULT_TYPE_DMTX`     -> Datamatrix 2D barcode
 * - `MS_RESULT_TYPE_IMAGE`    -> Image match
 *
 * These are to be used either as scan options (see `MSScannerSession`)
 * by combining them with bitwise-or, or to hold a kind of result.
 */
@interface MSResult : NSObject <NSCopying> {
    ms_result_t *_result;
#if MS_IPHONE_OS_REQUIREMENTS
    CGImageRef _image;
    AVCaptureVideoOrientation _orientation;
#endif
}

/** The underlying `ms_result_t` structure.
 */
@property (nonatomic, readonly) ms_result_t *handle;

///---------------------------------------------------------------------------------------
/// @name Initialization Methods
///---------------------------------------------------------------------------------------

/** Initialize a blank result.
 *
 * @return a result instance.
 */
- (id)init;

/** Initialize a result by copying result.
 *
 * @param result the result to be initialized with. It is deep duplicated.
 * @return a result instance.
 */
- (id)initWithResult:(const ms_result_t *)result;

///---------------------------------------------------------------------------------------
/// @name Getter Methods
///---------------------------------------------------------------------------------------

/** Get the string scan result.
 *
 * Use `getData` if you intend to create a string with another encoding or just want to interact
 * with the raw bytes
 *
 * @return the result as a string with UTF-8 encoding.
 */
- (NSString *)getValue;

/** Get the raw scan result.
 *
 * @return the result as a byte array.
 */
- (NSData *)getData;

/** Get the decoded scan result by applying base64url without padding decoding on it.
 *
 * @return the decoded result a byte array.
 */
- (NSData *)getDataFromBase64URL;

/** Perform base64url without padding decoding.
 *
 * @param string the string to be decoded.
 * @return the decoded data as a byte array.
 */
+ (NSData *)dataFromBase64URLString:(NSString *)string;

/** Get the kind of scan result.
 *
 * @return the result type.
 */
- (MSResultType)getType;

///---------------------------------------------------------------------------------------
/// @name Getter Methods for Geometrical Data
///---------------------------------------------------------------------------------------

/** Compute the homography between the matched reference image and the query frame.
 *
 * The query frame is considered in its initial orientation, i.e. as the frame is **physically**
 * provided by the camera.
 *
 * In order to make abstraction of the frame and reference image dimensions,
 * this homography is computed with the assumption that both the frame and the
 * reference image coordinates system are in the [-1..1] range.
 *
 * This homography can be used to project reference image points into the query
 * frame domain by using homogeneous coordinates, i.e.:
 *
 *     P' = H x P
 *     with: P = [x, y, 1]t and P' = [u, v, w]t
 *
 * @param homog the 3x3 homography matrix in row-major order, if any.
 * @return `YES` if the homography could be computed, `NO` otherwise.
 */
- (BOOL)getHomography:(float[9])homog;

/** Get the (x, y) coordinates of the corner points that delimit the area of the
 * recognized content within the query frame domain in its initial orientation,
 * i.e. as the frame is **physically** provided by the camera.
 *
 * The coordinates are provided as a ratio computed with the query frame dimensions,
 * i.e. they are within the range [-1..1] when the point is found inside the query frame.
 *
 * @param corners the array of the 4 corners in clockwise order starting from top-left, if any.
 * @return `YES`  if the corners could be computed, `NO` otherwise.
 *
 * @warning **Note:** the corners are **NOT** clamped with the query frame boundaries and could thus take
 * values outside the range [-1..1].
 */
- (BOOL)getCorners:(CGPoint[4])corners;

/** Same as `getCorners` but re-orients the coordinates to fit the current screen orientation.
 *
 * @param corners the array of the 4 corners in clockwise order starting from top-left, if any.
 * @param ori the current screen orientation.
 * @return `YES`  if the corners could be computed, `NO` otherwise.
 */
- (BOOL)getCorners:(CGPoint[4])corners forOrientation:(UIInterfaceOrientation)ori;

/** Get the dimensions of the reference frame that has been matched.
 *
 * @param dims the dimensions of the reference frame in pixels, if any.
 * @return `YES` if there are such dimensions, `NO` otherwise.
 *
 * @warning **Note:** this method returns `NO` if the result refers to a barcode.
 */
- (BOOL)getDimensions:(CGSize *)dims;

#if MS_IPHONE_OS_REQUIREMENTS
/** Sets the CGImage corresponding to the query frame associated with this result.
 * 
 * @param img the query frame as a CGImage. The resulting `MSResult` object retains a
 * reference on this image, which means you may release the original image after calling
 * this function.
 * @param ori the `AVCaptureVideoOrientation` flag corresponding to the orientation
 * used to create the `MSImage` that led to this result.
 */
- (void)setImage:(CGImageRef)img withOrientation:(AVCaptureVideoOrientation)ori;

/**
 * Get the query image corresponding to this result, as physically provided by the camera,
 * i.e not re-oriented.
 *
 * WARNING: This method will always return `NO` if the `MS_RESULT_EXTRA_IMAGE` flag
 * has not been added to the `MSScannerSession` using the `setExtras` method!
 * @param img the pointer to the `UIImage` where to store the result.
 * @return `YES` if successful, `NO` otherwise.
 */
- (BOOL)getImage:(UIImage **)img;

/**
 * Get the query image, re-oriented according to the physical orientation of the device,
 * as allowed using `useDeviceOrientation`.
 *
 * WARNING: This method will always return `NO` if the `MS_RESULT_EXTRA_IMAGE` flag
 * has not been added to the `MSScannerSession` using the `setExtras` method!
 * @param img the pointer to the `UIImage` where to store the result.
 * @return `YES` if successful, `NO` otherwise.
 */
- (BOOL)getOrientedImage:(UIImage **)img;

/**
 * Get the warped image recognized object from the query frame, at the maximum possible resolution.
 *
 * This methods crops and straightens up the detected region of interest from the query image, and
 * gives it the aspect ratio of the original reference image, while keeping the best possible
 * resolution.
 *
 * This method can be used to pipe the results of the scanner to any third-party
 * library requesting high quality frames, such as an optical character recognition
 * (OCR) library.
 *
 * Given the fact that this method tries to retrieve the best possible quality, it
 * can be quite time-consuming and should be run asynchronously.
 *
 * WARNING: This method will always return `NO` if the `MS_RESULT_EXTRA_IMAGE` flag
 * has not been added to the `MSScannerSession` using the `setExtras` method!
 * @param warped the pointer to the `UIImage` where to store the result.
 * @return `YES` if successful, `NO` otherwise.
 */
- (BOOL)getWarped:(UIImage **)warped;

/**
 * Similar to the above method, but with a specified result size.
 * 
 * Unlike the above method, the returned UIImage is guaranteed to
 * be exactly of the matched reference image, rescaled according to
 * the `scale` scaling factor.
 * @param warped the pointer to the `UIImage` where to store the result.
 * @param scale the scaling factor, in the [0..1] range. If not in this range,
 * it is clamped to fit into it.
 * @return the warped query frame, at the specified size.
 * @return `YES` if successful, `NO` otherwise.
 */
- (BOOL)getWarped:(UIImage **)warped scale:(float)scale;
#endif

///---------------------------------------------------------------------------------------
/// @name Misc Methods
///---------------------------------------------------------------------------------------

/** Compare two results.
 *
 * @param result the result to compare with.
 * @return `YES` if the current result is strictly the same than the input one, `NO` otherwise.
 */
- (BOOL)isEqualToResult:(MSResult *)result;

/** Clone a result.
 *
 * @param zone the zone object.
 * @return the clone result instance.
 */
- (id)copyWithZone:(NSZone *)zone;
@end
