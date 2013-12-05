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

#import "MSImage.h"
#import "MSObjC.h"

#if MS_IPHONE_OS_REQUIREMENTS
/**
 * Creates an image with Moodstocks format from a camera frame buffer
 *
 * The pixel format *must* be 32-bit BGRA (i.e `kCVPixelFormatType_32BGRA`)
 * otherwise the method returns a NULL pointer
 *
 * The caller must manage deletion
 */
ms_img_t *MSCreateImageFromSampleBuffer(CMSampleBufferRef sbuf);

/**
 * Same as above, but with a provided device orientation that determines
 * the "real" orientation of the image (e.g. see EXIF orientation for more details)
 *
 * The caller must manage deletion
 */
ms_img_t *MSCreateImageFromSampleBuffer2(CMSampleBufferRef sbuf, AVCaptureVideoOrientation orientation);
#endif

@implementation MSImage

@synthesize image = _img;

- (id)init {
    self = [super init];
    if (self) {
        _img = NULL;
    }
    return self;
}

#if MS_IPHONE_OS_REQUIREMENTS
- (id)initWithBuffer:(CMSampleBufferRef)buf {
    self = [self init];
    if (self) {
        _img = MSCreateImageFromSampleBuffer(buf);
    }
    return self;
}

- (id)initWithBuffer:(CMSampleBufferRef)buf
         orientation:(AVCaptureVideoOrientation)orientation {
    self = [self init];
    if (self) {
        _img = MSCreateImageFromSampleBuffer2(buf, orientation);
    }
    return self;
}
#endif

- (void)dealloc {
#if MS_SDK_REQUIREMENTS
    if (_img) ms_img_del(_img);
#endif
    _img = NULL;
    
#if ! __has_feature(objc_arc)
    [super dealloc];
#endif
}

#if MS_IPHONE_OS_REQUIREMENTS
+ (CGImageRef)newCGImageFromBuffer:(CMSampleBufferRef)buf {
    
    CGImageRef result = NULL;
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(buf);
    
    if (CVPixelBufferGetPixelFormatType(imageBuffer) != kCVPixelFormatType_32BGRA)
        return NULL;
    
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    
    void *data = CVPixelBufferGetBaseAddress(imageBuffer);
    size_t bpr = CVPixelBufferGetBytesPerRow(imageBuffer);
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data, width, height, 8,
                                                 bpr, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
    result = CGBitmapContextCreateImage(context);
    
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    
    return result;
}
#endif

+ (CGImageRef)newWarpFromImage:(CGImageRef)img data:(ms_warp_data_t *)data {
    return [self newWarpFromImage:img data:data scale:-1];
}

+ (CGImageRef)newWarpFromImage:(CGImageRef)img data:(ms_warp_data_t *)data scale:(float)scale {
    CGImageRef result = NULL;
    
#if MS_SDK_REQUIREMENTS
    // Get input image bytes data.
    ms_color_img_t src;
    src.width = (int) CGImageGetWidth(img);
    src.height = (int) CGImageGetHeight(img);
    src.stride = (int) CGImageGetBytesPerRow(img);
    src.data = malloc(src.stride * src.height);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(src.data, src.width, src.height,
                                                 8, src.stride, colorSpace,
                                                 kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
    CGContextDrawImage(context, CGRectMake(0, 0, src.width, src.height), img);
    CGContextRelease(context);
    
    // Warp
    ms_color_img_t dst;
    if (scale < 0)
        dst = ms_color_img_warp(&src, data);
    else
        dst = ms_color_img_warp2(&src, data, scale);
    
    // Write to CGImage
    if (dst.data) {
        context = CGBitmapContextCreate(dst.data, dst.width, dst.height,
                                        8, dst.stride, colorSpace,
                                        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
        result = CGBitmapContextCreateImage(context);
        CGContextRelease(context);
        free(dst.data);
    }
    CGColorSpaceRelease(colorSpace);
    free(src.data);
#endif
    
    return result;
}
@end

#if MS_IPHONE_OS_REQUIREMENTS
ms_img_t *MSCreateImageFromSampleBuffer(CMSampleBufferRef sbuf) {
    return MSCreateImageFromSampleBuffer2(sbuf, -1);
}

ms_img_t *MSCreateImageFromSampleBuffer2(CMSampleBufferRef sbuf, AVCaptureVideoOrientation orientation) {
#if MS_SDK_REQUIREMENTS
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sbuf);
    
    if (CVPixelBufferGetPixelFormatType(imageBuffer) != kCVPixelFormatType_32BGRA)
        return NULL;
    
    CVPixelBufferLockBaseAddress(imageBuffer, 0); 
    
    void *data = CVPixelBufferGetBaseAddress(imageBuffer); 
    int bpr = (int) CVPixelBufferGetBytesPerRow(imageBuffer);
    int width = (int) CVPixelBufferGetWidth(imageBuffer);
    int height = (int) CVPixelBufferGetHeight(imageBuffer);
    
    ms_pix_fmt_t fmt = MS_PIX_FMT_RGB32;
    ms_ori_t ori = MS_UNDEFINED_ORI;
    switch (orientation) {
        case AVCaptureVideoOrientationPortrait:
            ori = MS_LEFT_BOTTOM_ORI;
            break;
            
        case AVCaptureVideoOrientationLandscapeRight:
            ori = MS_TOP_LEFT_ORI;
            break;
            
        case AVCaptureVideoOrientationLandscapeLeft:
            ori = MS_BOTTOM_RIGHT_ORI;
            break;
            
        case AVCaptureVideoOrientationPortraitUpsideDown:
            ori = MS_RIGHT_TOP_ORI;
            break;
            
        default:
            break;
    }
    
    ms_img_t *img;
    ms_errcode ecode = ms_img_new(data, width, height, bpr, fmt, ori, &img);
    
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    
    return (ecode == MS_SUCCESS) ? img : NULL;
#else
    return NULL;
#endif
}
#endif
