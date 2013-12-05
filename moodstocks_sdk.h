/**
 * Copyright (C) 2013 Moodstocks. All rights reserved.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _MOODSTOCKS_SDK_H
#define _MOODSTOCKS_SDK_H

#ifdef  __cplusplus
extern "C" {
#endif

/** Library version string */
extern const char *ms_version;

/*************************************************
 * Error Codes
 *************************************************/

/**
 * Type of a library error code
 */
typedef int ms_errcode;

/**
 * Enumeration for error codes
 */
enum {                                /* enumeration for error codes */
  MS_SUCCESS = 0,                     /* (0)  success */
  MS_ERROR,                           /* (1)  unspecified error */
  MS_MISUSE,                          /* (2)  invalid use of the library */
  MS_NOPERM,                          /* (3)  access permission denied */
  MS_NOFILE,                          /* (4)  file not found */
  MS_BUSY,                            /* (5)  database file locked */
  MS_CORRUPT,                         /* (6)  database file corrupted */
  MS_EMPTY,                           /* (7)  empty database */
  MS_AUTH,                            /* (8)  authorization denied */
  MS_NOCONN,                          /* (9)  no internet connection */
  MS_TIMEOUT,                         /* (10) operation timeout */
  MS_THREAD,                          /* (11) threading error */
  MS_CREDMISMATCH,                    /* (12) credentials mismatch */
  MS_SLOWCONN,                        /* (13) internet connection too slow */
  MS_NOREC,                           /* (14) record not found */
  MS_ABORT,                           /* (15) operation aborted */
  MS_UNAVAIL,                         /* (16) resource temporarily unavailable */
  MS_IMG,                             /* (17) image size or format not supported */
  MS_APIKEY                           /* (18) wrong API key or no offline image */
};

/**
 * Get the character string corresponding to an error code.
 * @param ecode the error code.
 * @return the character string associated to the error code.
 */
const char *ms_errmsg(ms_errcode ecode);

/*************************************************
 * Image Data Types
 *************************************************/

/**
 * Pixel Format
 *
 * Specifies the color format and encoding for each pixel in the image.
 *
 * MS_PIX_FMT_RGB32
 * This is a packed-pixel format handled in an endian-specific manner.
 * An RGBA color is packed in one 32-bit integer as follow:
 *   (A << 24) | (R << 16) | (G << 8) | B
 * This is stored as BGRA on little-endian CPU architectures (e.g. iPhone)
 * and ARGB on big-endian CPUs.
 *
 * MS_PIX_FMT_GRAY8
 * This specifies a 8-bit per pixel grayscale pixel format.
 *
 * MS_PIX_FMT_NV21
 * This specifies the YUV pixel format with 1 plane for Y and 1 plane for the
 * UV components, which are interleaved: first byte V and the following byte U
 */
typedef enum {          /* enumeration of pixel formats */
  MS_PIX_FMT_RGB32 = 0, /* 32bpp BGRA on little-endian CPU arch */
  MS_PIX_FMT_GRAY8,     /* 8bpp grey */
  MS_PIX_FMT_NV21,      /* planar YUV 4:2:0, 12bpp, one plane for Y and one for UV */
  MS_PIX_FMT_NB         /* number of pixel formats - do not use! */
} ms_pix_fmt_t;

/**
 * Image Orientation
 *
 * Flags defining the real orientation of the image as found within
 * the EXIF specification
 *
 * Each flag specifies where the origin (0,0) of the image is located.
 * Use 0 (undefined) to ignore or 1 (the default) to keep the
 * image unchanged.
 */
typedef enum {
  /** undefined orientation (i.e image is kept unchanged) */
  MS_UNDEFINED_ORI    = 0,
  /** 0th row is at the top, and 0th column is on the left (the default) */
  MS_TOP_LEFT_ORI     = 1,
  /** 0th row is at the bottom, and 0th column is on the right */
  MS_BOTTOM_RIGHT_ORI = 3,
  /** 0th row is on the right, and 0th column is at the top */
  MS_RIGHT_TOP_ORI    = 6,
  /** 0th row is on the left, and 0th column is at the bottom */
  MS_LEFT_BOTTOM_ORI  = 8
} ms_ori_t;

/**
 * Type of a 8-bit grayscaled image object
 */
typedef struct ms_img_t_ ms_img_t;

/**
 * Create an image object from input image data.

 * @param data the pointer to aligned image data.
 * @param w the image width in pixels (see below for more details).
 * @param h the image height in pixels (see below for more details).
 * @param bpr the size of aligned image row in bytes.
 * @param fmt the image pixel format.
 * @param ori the image orientation.
 * @param img the pointer to a variable into which the pointer to the created
 * image will be assigned.
 * @return the appropriate error code if something went wrong, MS_SUCCESS otherwise.
 *
 * Because the returned image is allocated by this function, it should be released
 * with the `ms_img_del' call when it is no longer useful.
 *
 * IMPORTANT: - the largest input dimension *MUST* be higher than or equal to
 *              480 pixels.
 *            - Image size should not exceed 1280x720 pixels.
 * Any image that does not respect *all* these conditions will be rejected
 * and an `MS_MISUSE' error code will be returned.
 *
 * Whenever possible we recommend you to provide a 1280x720 pixels image.
 */
ms_errcode ms_img_new(const void *data,
                      int w,
                      int h,
                      int bpr,
                      ms_pix_fmt_t fmt,
                      ms_ori_t ori,
                      ms_img_t** img);

/**
 * Delete an image object.
 * @param img the image object to delete.
 */
void ms_img_del(ms_img_t *img);

/**
 * Type of a color image.
 * Represents a 8 bit-per-component color image with 4 interleaved channels,
 * such as RGBA, ARGB, BGRA...
 */
typedef struct {
  unsigned char *data;    /** The image pixels */
  int width;              /** The image width */
  int height;             /** The image height */
  int stride;             /** The number of bytes per row */
} ms_color_img_t;

/** Represents the data used to warp a color image. */
typedef struct {
  /**
   * The homography to apply, using the conventions
   * specified in `ms_result_get_homography'.
   */
  float *homography;
  /** The requested transformed image width  */
  int width;
  /** The requested transformed image height */
  int height;
} ms_warp_data_t;

/**
 * Applies a perspective transform to a color image.
 *
 * The resulting transformed image will be of the higher
 * possible resolution given the inputs, preserving the
 * aspect ratio of the dimensions set in `data'.
 * Use this function if you want, for example, to pipe the
 * results of the scanner into a third-party library such
 * as an Optical Character Recognition library.
 *
 * @param src the color image to warp
 * @param data the `ms_warp_data_t' to use for warping.
 * @return the warped image.
 *
 * The caller must manage deletion of the resulting image
 * data.
 * If the warped image could not be computed, the returned
 * structure will be initialized to all zeroes.
 *
 * WARNING: this function aims at providing the best possible
 * quality, but can be time-consuming, so you should run it
 * asynchronously.
 */
ms_color_img_t ms_color_img_warp(const ms_color_img_t *src,
                                 ms_warp_data_t *data);

/**
 * Similar to `ms_color_img_warp', but the resulting image
 * will be of the *exact* size specified in `data', scaled
 * by the `scale' factor.
 *
 * @param src the color image to warp
 * @param data the `ms_warp_data_t' to use for warping.
 * @param scale the scale factor.
 * @return the warped image.
 */
ms_color_img_t ms_color_img_warp2(const ms_color_img_t *src,
                                  ms_warp_data_t *data,
                                  float scale);

/*************************************************
 * Scan Result Type
 *************************************************/

/**
 * Type of a scan result
 */
typedef int ms_result_type;

/**
 * Enumeration for scan result types.
 */
enum {
  MS_RESULT_TYPE_NONE      = 0,
  MS_RESULT_TYPE_EAN8      = 1 << 0,      /* EAN8 linear barcode */
  MS_RESULT_TYPE_EAN13     = 1 << 1,      /* EAN13 linear barcode */
  MS_RESULT_TYPE_QRCODE    = 1 << 2,      /* QR Code 2D barcode */
  MS_RESULT_TYPE_DMTX      = 1 << 3,      /* Datamatrix 2D barcode */
  MS_RESULT_TYPE_IMAGE     = 1 << 31      /* Image match */
};

/**
 * Type of a scan result object
 */
typedef struct ms_result_t_ ms_result_t;

/**
 * Get the pointer to the result data.
 *
 * @param r the result object.
 * @param siz the pointer to the variable into which the size of the result data
 * is assigned.
 * @param data the pointer to the region containing the result data. It should
 * *not* be freed after use. The memory will be freed when ms_result_del() is
 * called on `r';
 */
void ms_result_get_data(const ms_result_t *r,
                        const char **data,
                        int *siz);

/**
 * Get the pointer to the result data, assuming it's a NULL terminated string.
 * It's safe to use this method on results coming from image recognition but
 * not on barcode results, because QR-Codes and Datamatrices binary data may
 * contain '\0' bytes.
 *
 * @param r the result object.
 * @param string the pointer to the region containing the result string. It should
 * *not* be freed after use. The memory will be freed when ms_result_del() is
 * called on `r';
 */
void ms_result_get_data2(const ms_result_t *r,
                         const char **string);

/**
 * Get the base64url without padding decoded version of the data.
 *
 * @param r the result object.
 * @param siz the pointer to the variable into which the size of the
 * decoded data is assigned.
 * @return the decoded data. You are responsible of releasing it by calling
 * `free' on it.
 */
char *ms_result_get_data_b64(const ms_result_t *r,
                             int *siz);

/**
 * Decodes the given null-terminated, base64url without padding encoded string.
 *
 * @param buf the null-terminated string to convert from.
 * @param siz the pointer to the variable into which the size of the
 * decoded data is assigned.
 * @return the decoded data. You are responsible of releasing it by calling
 * `free' on it.
 */
char *ms_result_decode_b64(const char *buf,
                           int *siz);

/**
 * Get the type of the result.
 *
 * @param r the result object.
 * @return the type of the result, among `ms_result_type'.
 */
ms_result_type ms_result_get_type(const ms_result_t *r);

/**
 * Get the (x, y) coordinates of the corner points that delimit the area of the
 * recognized content within the query frame domain in its initial orientation,
 * i.e. as the frame is physically provided by the camera.
 *
 * The coordinates are provided as a ratio computed with the query frame
 * dimensions, i.e. they are within the range [-1, 1] when the point is found
 * inside the query frame. Note that they are not clamped with the query frame
 * boundaries and could thus take values outside the range [-1, 1].
 *
 * @param r the result from which to get corners.
 * @param corners the array of size 8 that will be filled with the 4 corners
 * coordinates as [x1, y1, x2, y2, ...], returned in clockwise order.
 * @return 0 if the corners could be computed, 1 otherwise.
 */
int ms_result_get_corners(const ms_result_t *r,
                          float corners[8]);

/**
 * For image recognition results only!
 * Compute the homography between the reference image and the query frame in its
 * initial orientation, (i.e. as the frame is physically provided by the camera)
 * to retrieve the related perspective transform.
 *
 * In order to make abstraction of the frame and reference image dimensions,
 * this homography is computed with the assumption that both the frame and the
 * reference image coordinates system are in the [-1, 1] range.
 * This homography can be used to project reference image points into the query
 * frame domain by using homegenous coordinates, i.e.:
 *   P' = H x P
 *   with: P = [x, y, 1]t and P' = [u, v, w]t
 *
 * @param r the result fron which to extract homography.
 * @param homog the array of siez 9 to fill with the homography, as
 * [H11, H12, H13, H21, ...] representing the 3x3 homography matrix in row-major
 * order.
 * @return 0 if the homography could be computed, 1 otherwise.
 */
int ms_result_get_homography(const ms_result_t *r,
                             float homog[9]);

/**
 * For image recognition results only!
 * Get the size in pixels of the matched image.
 *
 * @param r the result from which to get the reference frame dimensions.
 * @param ims the array of size 2 to fill with the reference frame dimensions,
 * as [width, height].
 * @return 0 if successful, 1 otherwise (for example if the result is a barcode).
 */
int ms_result_get_dimensions(const ms_result_t *r,
                             int dims[2]);

/**
 * Duplicate a result object
 *
 * @param r the result object to be duplicated.
 * @param rdup the pointer to a variable into which the pointer to the duplicated
 *  result will be assigned. Because the duplicated result is allocated by this
 * function it should be released with the `ms_result_del' call when it is no
 * longer used.
 * @return the appropriate error code if something went wrong, MS_SUCCESS otherwise.
 */
ms_errcode ms_result_dup(const ms_result_t *r,
                         ms_result_t **rdup);

/**
 * Compare two result objects
 *
 * @param ra the pointer to the first result object.
 * @param rb the pointer to the second result object.
 * @return 0 if the results are the same, a non-zero value if they are different
 * or if an error occurred.
 */
int ms_result_cmp(const ms_result_t *ra,
                  const ms_result_t *rb);

/**
 * Delete a result object.
 *
 * @param r the result object to delete.
 */
void ms_result_del(ms_result_t *r);

/*************************************************
 * Image Scanner
 *************************************************/

/**
 * Type of a scanner object
 */
typedef struct ms_scanner_t_ ms_scanner_t;

/**
 * Create a scanner object.
 *
 * @param s the pointer to a variable into which the pointer to the created
 * scanner will be assigned. Because the returned scanner is allocated by this
 * function it should be released with the `ms_scanner_del' call when it is no
 * longer used
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 */
ms_errcode ms_scanner_new(ms_scanner_t **s);

/**
 * Delete a scanner object.
 * If the scanner object was not closed, it is done implicitly.
 *
 * @param s the scanner object to delete.
 */
void ms_scanner_del(ms_scanner_t *s);

/**
 * Open a scanner object.
 *
 * @param s the scanner object to open.
 * @param path the path of the database file to be used by this scanner.
 * @param key the character string containing a valid Moodstocks API key
 * to be used with this scanner.
 * @param secret the character string containing a valid Moodstocks API secret
 * to be used with this scanner.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If this operation failed it is not possible to use the scanner for any other
 * operation.
 * If you try to open a scanner object that is already opened this function
 * returns an `MS_MISUSE` error code.
 */
ms_errcode ms_scanner_open(ms_scanner_t *s,
                           const char *path,
                           const char *key,
                           const char *secret);

/**
 * Close a scanner object.
 * Closing a scanner releases its database file.
 *
 * @param s the scanner object to close.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If you try to close a scanner that is not opened this function returns an
 * `MS_MISUSE` error code.
 */
ms_errcode ms_scanner_close(ms_scanner_t *s);

/**
 * Remove the database file related to a scanner.
 *
 * This is a convenient utility provided for *extraordinary* situations.
 * In practice there are no more than two situtations that legitimate calling clean:
 * - when the `ms_scanner_open` operation failed because of a corrupt database,
 *   i.e. when the `MS_CORRUPT` error code is returned;
 * - when you decide intentionally to clean up the database file, e.g. because
 *   of not enough disk space, etc.
 *
 * @param path the path to the database file to remove.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * This function fails with a locking error (error code `MS_BUSY`) if an open
 * scanner is associated to the database file. Make sure to close the scanner
 * object before calling it.
 */
ms_errcode ms_scanner_clean(const char *path);

/*************************************************
 * On-device Cache Synchronization
 *************************************************/

/** Type of a synchronization progress callback function
 *
 * @param opq the pointer to the opaque object that will be passed as input.
 * @param total the total number of image signatures to be synchronized,
 * or -1 if this number is not known yet.
 * @param current how many image signatures have been synchronized so far,
 * or -1 if this number is not known yet.
 * @return a non-zero value to cancel the synchronization as soon as possible.
 */
typedef int (*ms_scanner_sync_cb)(void *opq,
                                  int total,
                                  int current);

/**
 * Synchronize a scanner object via HTTP requests to Moodstocks API.
 * This requires a working Internet connection.
 *
 * @param s the scanner object.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If no connection is available, a MS_NOCONN error code will be returned.
 *
 * It is important to note that this function is blocking. It must be run
 * asynchronously to avoid blocking the main thread of the application.
 */
ms_errcode ms_scanner_sync(ms_scanner_t *s);

/** Same as `ms_scanner_sync` with the ability to provide a progress callback
 * This requires a working Internet connection.
 *
 * @param s the scanner object.
 * @param cb the progress callback function.
 * @param opq the pointer to the opaque object that will be passed as
 * input to the progress callback function.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If no connection is available, a MS_NOCONN error code will be returned.
 *
 * It is important to note that this function is blocking. It must be run
 * asynchronously to avoid blocking the main thread of the application.
 */
ms_errcode ms_scanner_sync2(ms_scanner_t *s,
                            ms_scanner_sync_cb cb,
                            void *opq);

/**
 * Get information about the synchronized database.
 * This does not require any Internet connection.
 *
 * @param s the scanner object.
 * @param count the pointer to a variable into which the size of the
 * database (i.e. the number of image records) is assigned.
 * @param ids the pointer to a variable into which the pointer to the array
 * of unique image identifier strings are stored. If the array of identifiers is
 * not needed, `NULL' can be provided. If the array is provided, it will contain
 * `count' strings if the database is not empty.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If the database contains no image record, this function returns an `MS_EMPTY'
 * error code and does not modify `count' and `ids'.
 *
 * Because the returned array and each array element are allocated by this function,
 * they should be released with the `free' call when they are no longer useful.
 */
ms_errcode ms_scanner_info(ms_scanner_t *s,
                           int *count,
                           char ***ids);

/*************************************************
 * On-device Search
 *************************************************/

/**
 * Optional flags to be used with ms_scanner_search2 and ms_scanner_match2.
 */
typedef enum {
  /**
   * Default mode.
   */
  MS_SEARCH_DEFAULT     = 0,
  /**
   * Disables "partial matching".
   * Use this flag to avoid returning false positive results due to partial
   * matching, for example if several of the indexed images share the exact same
   * logo.
   */
  MS_SEARCH_NOPARTIAL   = 1 << 0,
  /**
   * Allows small image recognition.
   * Use this flag to boost the scale invariance so that smaller or farther images
   * can be recognized. Slightly slower that default mode.
   */
  MS_SEARCH_SMALLTARGET = 1 << 1
} ms_search_flag_t;

/**
 * Perform a search on the synchronized database (aka offline search).
 * This does not require any Internet connection.
 *
 * @param s the scanner object.
 * @param qry the query image object.
 * @param result the pointer to a variable into which the pointer to the
 * result containing the character string representing the image unique
 * identifier will be assigned if a match is found. If there is no match, it is
 * set to `NULL'.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * Because the returned result is allocated by this function when a match is found
 * it should be released with the `ms_result_del' call when it is no longer useful.
 */
ms_errcode ms_scanner_search(ms_scanner_t *s,
                             const ms_img_t *qry,
                             ms_result_t **result);

/**
 * Similar to the above function, but with additional options.
 *
 * @param s the scanner object.
 * @param qry the query image object.
 * @param result the pointer to a variable into which the pointer to the
 * result containing the character string representing the image unique
 * identifier will be assigned if a match is found. If there is no match, it is
 * set to `NULL'.
 * @param options the options specified as a bitwise-OR combination of the
 * requested `ms_search_flag_t' options.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * Because the returned result is allocated by this function when a match is found
 * it should be released with the `ms_result_del' call when it is no longer useful.
 */
ms_errcode ms_scanner_search2(ms_scanner_t *s,
                              const ms_img_t *qry,
                              ms_result_t **result,
                              ms_search_flag_t options);

/**
 * Match a query image against a reference image from the synchronized database.
 * This does not require any Internet connection.
 *
 * @param s the scanner object.
 * @param qry the query image object.
 * @param id the unique identifier of the reference image to be matched against.
 * It can be easily obtained from a previously returned `ms_result_t' image result
 * using `ms_result_get_data2'.
 * @param match the pointer to a variable into which the matching result will
 * be assigned. If query matches against the reference image this value is set,
 * otherwise it is set to `NULL'.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If there is no reference image with this unique identifier into the database,
 * the MS_NOREC error code is returned.
 * If the database is empty then MS_EMPTY is returned.
 *
 * Because the returned match is allocated by this function, it should be released
 * with the `ms_result_del` call when it is no longer used.
 */
ms_errcode ms_scanner_match(ms_scanner_t *s,
                            const ms_img_t *qry,
                            const char *id,
                            ms_result_t **match);

/**
 * Similar to the above function, but with additional options.
 *
 * @param s the scanner object.
 * @param qry the query image object.
 * @param id the unique identifier of the reference image to be matched against.
 * It can be easily obtained from a previously returned `ms_result_t' image result
 * using `ms_result_get_data2'.
 * @param match the pointer to a variable into which the matching result will
 * be assigned. If query matches against the reference image this value is set,
 * otherwise it is set to `NULL'.
 * @param options the options specified as a bitwise-OR combination of the
 * requested `ms_search_flag_t' options.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If there is no reference image with this unique identifier into the database,
 * the MS_NOREC error code is returned.
 * If the database is empty then MS_EMPTY is returned.
 *
 * Because the returned match is allocated by this function, it should be released
 * with the `ms_result_del` call when it is no longer used.
 */
ms_errcode ms_scanner_match2(ms_scanner_t *s,
                             const ms_img_t *qry,
                             const char *id,
                             ms_result_t **match,
                             ms_search_flag_t options);

/*************************************************
 * Online Search
 *************************************************/

/**
 * Type of a Moodstocks API handle
 */
typedef struct ms_api_handle_t_ ms_api_handle_t;

/**
 * Obtain a Moodstocks API handle.
 *
 * @param s the scanner object.
 * @param h the pointer to a variable into which the pointer to the handle
 * object will be assigned.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If MS_UNAVAIL is returned, the call to this function might work if you try again
 * later.
 *
 * Because the returned result is allocated by this function it should be released
 * with the `ms_api_handle_del' call when it is no longer used.
 */
ms_errcode ms_scanner_api_handle(ms_scanner_t *s,
                                 ms_api_handle_t **h);

/**
 * Perform a search via an HTTP request to Moodstocks API.
 * This requires a working Internet connection.
 *
 * @param h the handle object.
 * @param qry the query image object.
 * @param result the pointer to a variable into which the pointer to the
 * result containing the character string representing the image unique
 * identifier will be assigned if a match is found. If there is no match, it is
 * set to `NULL'.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * If no connection is available, a MS_NOCONN error code will be returned.
 *
 * Because the returned result is allocated by this function it should be released
 * with the `ms_result_del' call when it is no longer used.
 *
 * It is important to note that this function is blocking. It must be run
 * asynchronously to avoid blocking the main thread of the application.
 */
ms_errcode ms_api_handle_search(const ms_api_handle_t *h,
                                const ms_img_t *qry,
                                ms_result_t **result);

/**
 * Cancel the pending request related to a API handle.
 *
 * This function has for effect to abort any pending request at its earliest
 * opportunity. The caller must call this function from a thread different from
 * the thread that is currently running the request.
 *
 * @param h the handle object.
 *
 * If successful the return value of the pending request will be `MS_ABORT'.
 */
void ms_api_handle_cancel(ms_api_handle_t *h);

/**
 * Release a Moodstocks API handle object after use.
 *
 * @param h the handle object.
 */
void ms_api_handle_release(ms_api_handle_t *h);

/*************************************************
 * Barcode Decoding
 *************************************************/

/**
 * Perform a barcode decoding.
 *
 * @param s the scanner object.
 * @param qry the query image object.
 * @param formats the formats to be decoded, as a bitwise-OR combination of the
 * barcode-related `ms_result_type' enum.
 * @param result the pointer to a variable into which the pointer to the result
 * barcode will be assigned if a barcode is successfully decoded. If no barcde
 * is decoded it is set to `NULL'.
 * @return an appropriate error code is something went wrong, MS_SUCCESS otherwise.
 *
 * Because the returned result is allocated by this function when a barcode is decoded
 * it should be released with the `ms_result_del' call when it is no longer used.
 */
ms_errcode ms_scanner_decode(ms_scanner_t *s,
                             const ms_img_t *qry,
                             int formats,
                             ms_result_t **result);

#ifdef  __cplusplus
}
#endif

#endif
