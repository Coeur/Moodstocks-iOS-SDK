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

#if MS_IPHONE_OS_REQUIREMENTS
  #import <AVFoundation/AVFoundation.h>
#endif

#include "moodstocks_sdk.h"

/** Wrapper around the Moodstocks SDK image data structure.
 */
@interface MSImage : NSObject {
    ms_img_t *_img;
}

/** The internal image handle.
 */
@property (readonly, nonatomic) ms_img_t *image;

/** Initialize an image.
 *
 * @return the image instance.
 */
- (id)init;

#if MS_IPHONE_OS_REQUIREMENTS
/** Initialize an image with a camera buffer.
 *
 * @param buf the camera raw image buffer.
 * @return the image instance.
 */
- (id)initWithBuffer:(CMSampleBufferRef)buf;

/** Initialize an image with a camera buffer re-oriented with input orientation.
 *
 * @param buf the camera raw image buffer.
 * @param orientation the orientation used to rotate the input buffer.
 * @return the image instance.
 */
- (id)initWithBuffer:(CMSampleBufferRef)buf
         orientation:(AVCaptureVideoOrientation)orientation;

/** Converts a camera sample buffer of type `kCVPixelFormatType_32BGRA` to a CGImage.
 *
 * @param buf the sample buffer to convert.
 * @return the created CGImage. The caller must manage its deletion.
 */
+ (CGImageRef)newCGImageFromBuffer:(CMSampleBufferRef)buf;

#endif

/**
 * Warps at the maximum possible resolution a CGImage using a perspective transform.
 *
 * This function can be used to pipe the results of the scanner to any third-party
 * library requesting high quality frames, such as an optical character recognition
 * (OCR) library.
 * Given the fact that this function tries to retrieve the best possible quality, it
 * can be quite time-consuming and should be run asynchronously.
 * @param img the CGImage to transform
 * @param data the `ms_warp_data_t` object specifying the perspective transform and
 * desired result dimensions.
 * @return the warped CGImage, at the maximum possible resolution given the inputs,
 * and preserving the aspect ratio specified by the dimensions in `data`, or `nil`
 * if it could not be computed. The caller must manage its deletion.
 */
+ (CGImageRef)newWarpFromImage:(CGImageRef)img data:(ms_warp_data_t *)data;

/**
 * Similar to the above function, but with a specified result size.
 * Unlike the above function, the returned bitmap is guaranteed to
 * be of the exact size specified in `data`, rescaled using the `scale`
 * factor.
 * @param img the source CGImage to transform
 * @param data the `ms_warp_data_t` object specifying the perspective transform and
 * desired result dimensions.
 * @param scale the scale factor to apply to the dimensions specified in `data`, in
 * the [0..1] range. If not in this range, it is clamped to fit into it.
 * @return the warped CGImage, at the exact size specified by `data`, rescaled using the
 * `scale` factor, or `nil` if it could not be computed. The caller must manage its
 * deletion.
 */
+ (CGImageRef)newWarpFromImage:(CGImageRef)img data:(ms_warp_data_t *)data scale:(float)scale;
@end
