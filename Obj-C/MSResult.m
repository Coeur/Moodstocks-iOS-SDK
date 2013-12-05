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

#import "MSResult.h"
#import "MSAvailability.h"
#import "MSObjC.h"
#import "MSImage.h"

@implementation MSResult

@synthesize handle = _result;

- (id)init {
    self = [super init];
    if (self) {
        _result = NULL;
#if MS_IPHONE_OS_REQUIREMENTS
        _image = NULL;
        _orientation = AVCaptureVideoOrientationPortrait;
#endif
    }
    return self;
}

- (id)initWithResult:(const ms_result_t *)result {
    self = [self init];
#if MS_SDK_REQUIREMENTS
    if (self) {
        ms_result_dup(result, &_result);
    }
#endif
    return self;
}

- (NSString *)getValue {
    NSString *str = nil;
#if MS_SDK_REQUIREMENTS
    if (_result) {
        const char *bytes = NULL;
        int length;
        ms_result_get_data(_result, &bytes, &length);
        str = [[[NSString alloc] initWithBytes:bytes
                                        length:length
                                      encoding:NSUTF8StringEncoding] autorelease_stub];
    }
#endif
    return str;
}

- (NSData *)getData {
    NSData *data = nil;
#if MS_SDK_REQUIREMENTS
    if (_result) {
        const char *bytes = NULL;
        int length;
        ms_result_get_data(_result, &bytes, &length);
        data = [NSData dataWithBytes:bytes length:length];
    }
#endif
    return data;
}

- (NSData *)getDataFromBase64URL {
    NSData *data = nil;
#if MS_SDK_REQUIREMENTS
    if (_result) {
        int length;
        char *bytes = ms_result_get_data_b64(_result, &length);
        data = [NSData dataWithBytesNoCopy:bytes length:length freeWhenDone:YES];
    }
#endif
    return data;
}


+ (NSData *)dataFromBase64URLString:(NSString *)string {
    NSData *data = nil;
#if MS_SDK_REQUIREMENTS
    const char *str = [string cStringUsingEncoding:NSASCIIStringEncoding];
    int length;
    char *bytes = ms_result_decode_b64(str, &length);
    data = [NSData dataWithBytesNoCopy:bytes length:length freeWhenDone:YES];
#endif
    return data;
}

- (MSResultType)getType {
    ms_result_type type = MS_RESULT_TYPE_NONE;
#if MS_SDK_REQUIREMENTS
    if (_result)
        type = ms_result_get_type(_result);
#endif
    return type;
}

- (BOOL)getHomography:(float[9])homog {
#if MS_SDK_REQUIREMENTS
    if (_result) {
        if (!ms_result_get_homography(_result, homog)) {
            return YES;
        };
    }
#endif
    return NO;
}

- (BOOL)getCorners:(CGPoint[4])corners {
#if MS_SDK_REQUIREMENTS
    if (_result) {
        float c[8];
        if (!ms_result_get_corners(_result, c)) {
            for (int i = 0; i < 4; ++i) {
                corners[i] = CGPointMake(c[2*i], c[2*i+1]);
            }
            return YES;
        }
    }
#endif
    return NO;
}

- (BOOL)getCorners:(CGPoint[4])corners forOrientation:(UIInterfaceOrientation)ori {
#if MS_SDK_REQUIREMENTS
    if (_result) {
        float c[8];
        if (!ms_result_get_corners(_result, c)) {
            switch (ori) {
                case UIInterfaceOrientationPortrait:
                    for (int i = 0; i < 4; ++i) {
                        corners[i] = CGPointMake(-c[2*i+1], c[2*i]);
                    }
                    break;

                case UIInterfaceOrientationPortraitUpsideDown:
                    for (int i = 0; i < 4; ++i) {
                        corners[i] = CGPointMake(c[2*i+1], -c[2*i]);
                    }
                    break;

                case UIInterfaceOrientationLandscapeLeft:
                    for (int i = 0; i < 4; ++i) {
                        corners[i] = CGPointMake(-c[2*i], -c[2*i+1]);
                    }
                    break;

                case UIInterfaceOrientationLandscapeRight:
                default:
                    for (int i = 0; i < 4; ++i) {
                        corners[i] = CGPointMake(c[2*i], c[2*i+1]);
                    }
                    break;
            }
            return YES;
        }
    }
#endif
    return NO;
}

- (BOOL)getDimensions:(CGSize *)dims {
#if MS_SDK_REQUIREMENTS
    if (_result) {
        int d[2];
        if (!ms_result_get_dimensions(_result, d)) {
            *dims = CGSizeMake(d[0], d[1]);
            return YES;
        }
    }
#endif
    return NO;
}

#if MS_IPHONE_OS_REQUIREMENTS
- (void)setImage:(CGImageRef)img withOrientation:(AVCaptureVideoOrientation)ori {
    _image = img;
    CGImageRetain(_image);
    _orientation = ori;
}

- (BOOL)getImage:(UIImage **)img {
    if (!_image)
        return NO;
    *img = [UIImage imageWithCGImage:_image];
    return YES;
}

- (BOOL)getOrientedImage:(UIImage **)img {
    if (!_image)
        return NO;
    UIImageOrientation ori = UIImageOrientationUp;
    switch (_orientation) {
        case AVCaptureVideoOrientationPortrait:
            ori = UIImageOrientationRight;
            break;
        case AVCaptureVideoOrientationPortraitUpsideDown:
            ori = UIImageOrientationLeft;
            break;
        case AVCaptureVideoOrientationLandscapeLeft:
            ori = UIImageOrientationDown;
        case AVCaptureVideoOrientationLandscapeRight:
        default:
            break;
    }
    *img = [UIImage imageWithCGImage:_image scale:1.0 orientation:ori];
    return YES;
}

- (BOOL)getWarped:(UIImage **)warped {
    return [self getWarped:warped scale:-1];
}

- (BOOL)getWarped:(UIImage **)warped scale:(float)scale {
#if MS_SDK_REQUIREMENTS
    if (!_image)
        return NO;
    float homog[9];
    CGSize s;
    [self getHomography:homog];
    [self getDimensions:&s];
    ms_warp_data_t data = { .homography = homog,
                            .width = s.width,
                            .height = s.height };
    CGImageRef warp = [MSImage newWarpFromImage:_image data:&data scale:scale];
    *warped = [UIImage imageWithCGImage:warp];
    CGImageRelease(warp);
    return YES;
#else
    return NO;
#endif
}
#endif


- (BOOL)isEqualToResult:(MSResult *)result {
#if MS_SDK_REQUIREMENTS
    return ms_result_cmp(_result, [result handle]) == 0 ? YES : NO;
#else
    return NO;
#endif
}

- (void)dealloc {
#if MS_SDK_REQUIREMENTS
    if (_result)
        ms_result_del(_result);
#endif
    _result = NULL;
#if MS_IPHONE_OS_REQUIREMENTS
    if (_image)
        CGImageRelease(_image);
    _image = NULL;
#endif

#if ! __has_feature(objc_arc)
    [super dealloc];
#endif
}

- (id)copyWithZone:(NSZone *)zone {
#if MS_SDK_REQUIREMENTS
    MSResult *copy = [[MSResult allocWithZone:zone] initWithResult:_result];
    if (_image)
        [copy setImage:CGImageCreateCopy(_image) withOrientation:_orientation];
    return copy;
#else
    return nil;
#endif
}

@end
