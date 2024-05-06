/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: MatManip.hpp 2969 2020-05-04 16:23:18Z stephane $
 */

#pragma once

#include "vzTypes.hpp"

/** @file
 * MatManip is a collection of C++ functions in C Code Run's @p %vz namespace which act on OpenCV @p cv::Mat images.
 * They can be called independently, or via @ref vz::Imagination::get().
 */


namespace vz
{
	/// Compare two images.  This will only return @p true if the images are @b exactly the same.
	bool compare_exact(const cv::Mat & lhs, const cv::Mat & rhs);


	/** Convert the source @p cv::Mat image to single-channel grey.  If the source is already greyscale, then returns a
	 * clone of the source image.
	 *
	 * Examples:
	 * original image				| result image
	 * -----------------------------|-------------
	 * @image html capture.jpg ""	| @image html capture_grey.jpg ""
	 * @image html coins.jpg ""		| @image html coins_grey.jpg ""
	 *
	 *  @see @ref vz::Imagination::EMatType::kO2Greyscale
	 */
	cv::Mat to_greyscale(const cv::Mat & src);


	/** Apply a Gaussian blur to the image.
	 *
	 * @param [in] src The input image.  This can be a colour, greyscale, or binary image.
	 * @param [in] kernel_size The kernel size must be positive, and an odd number.  The larger the kernel size, the larger
	 * the blur area.  A kernel of size 1 does not blur.
	 *
	 * Examples:
	 * original image								| kernel_size=3							| kernel_size=9							| kernel_size=15
	 * ---------------------------------------------|---------------------------------------|---------------------------------------|---------------
	 * @image html gaussian_blur_original.jpg ""	| @image html gaussian_blur_3x3.jpg ""	| @image html gaussian_blur_9x9.jpg ""	| @image html gaussian_blur_15x15.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kO2GaussianBlur
	 */
	cv::Mat apply_gaussian_blur(const cv::Mat & src, const int kernel_size = 3);


	/** Apply a Gaussian blur to the image and then convert the result to single-channel greyscale.  This is equivalent to
	 * calling both @ref apply_gaussian_blur() and @ref to_greyscale().
	 *
	 * @param [in] src The input image.  This can be a colour, greyscale, or binary image.
	 * @param [in] kernel_size The kernel size must be positive, and an odd number.  The larger the kernel size, the larger
	 * the blur area.  A kernel of size 1 does not blur.
	 * @returns The output image will be single-channel greyscale, regardless of the input image.
	 *
	 * Examples:
	 * original image								| kernel_size=3								| kernel_size=9								| kernel_size=15
	 * ---------------------------------------------|-------------------------------------------|-------------------------------------------|---------------
	 * @image html gaussian_blur_original.jpg ""	| @image html gaussian_blur_3x3_grey.jpg ""	| @image html gaussian_blur_9x9_grey.jpg ""	| @image html gaussian_blur_15x15_grey.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kO2BlurGreyscale
	 */
	cv::Mat apply_blur_and_convert_to_greyscale(const cv::Mat & src, const int kernel_size = 3);


	/** Apply a simple colour balance to the image.
	 *
	 * > This is the color balancing technique used in Adobe Photoshop's "auto levels" command. The idea is that in a well
	 * > balanced photo, the brightest color should be white and the darkest black. Thus, we can remove the color cast from
	 * > an image by scaling the histograms of each of the R, G, and B channels so that they span the complete 0-255 scale.
	 *
	 * @note Source:  http://www.morethantechnical.com/2015/01/14/simplest-color-balance-with-opencv-wcode/
	 * @note Source:  http://web.stanford.edu/~sujason/ColorBalancing/simplestcb.html
	 *
	 * Examples:
	 * original image				| result image
	 * -----------------------------|-------------
	 * @image html capture.jpg ""	| @image html simple_colour_balance.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kO2SimpleColourBalance
	 */
	cv::Mat apply_simple_colour_balance(const cv::Mat & src);


	/** Erode the image.  Dark area in the image are "grown" to erode lighter area.
	 * Reverses the changes done by @ref vz::dilate().
	 *
	 * @param [in] src The input image.
	 * @param [in] iterations The number of erosion iterations to run.  Must be a positive number.
	 *
	 * Examples:
	 * original image					| iterations=1						| iterations=2						| iterations=3
	 * ---------------------------------------------|-----------------------|-----------------------------------|-------------
	 * @image html coins_grey.jpg ""	| @image html coins_erode_1.jpg ""	| @image html coins_erode_2.jpg ""	| @image html coins_erode_3.jpg ""
	 *
	 * The results are typically better when the source is a binary image:
	 * original image									| iterations=1									| iterations=2									| iterations=3
	 * -------------------------------------------------|-----------------------------------------------|-----------------------------------------------|-------------
	 * @image html coins_threshold_with_blur_3.jpg ""	| @image html coins_threshold_erode_1.jpg ""	| @image html coins_threshold_erode_2.jpg ""	| @image html coins_threshold_erode_3.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kErode
	 * @see @ref vz::Imagination::EMatType::kThreshold
	 * @see @ref vz::dilate()
	 */
	cv::Mat erode(const cv::Mat & src, const int iterations = 1);


	/** Dilate the image.  This works best on black-and-white threshold images.
	 * Does the opposite as @ref vz::erode(), meaning that white areas of the image are "grown".
	 *
	 * @param [in] src The input image.
	 * @param [in] iterations The number of dilation iterations to run.  Must be a positive number.
	 *
	 * Examples:
	 * original image			| erode iterations=3							| erode + dilate iterations=3
	 * -------------------------|-----------------------------------------------|----------------------------
	 * @image html coins.jpg ""	| @image html coins_threshold_erode_3.jpg ""	| @image html coins_erode_and_dilate_3.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kErodeAndDilate
	 * @see @ref vz::to_greyscale()
	 * @see @ref vz::erode()
	 */
	cv::Mat dilate(const cv::Mat & src, const int iterations = 1);


	/** Invert the image.
	 *
	 * Examples:
	 * original image				| result image
	 * -----------------------------|-------------
	 * @image html capture.jpg ""	| @image html capture_invert.jpg ""
	 * @image html coins.jpg ""		| @image html coins_invert.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kO2Invert
	 */
	cv::Mat invert(const cv::Mat & src);


	/** Apply canny edge detection.  The result of canny edge detection is a black-and-white image, and the output can
	 * differ significantly depending on the input source.
	 *
	 * @param [in] src The input image.
	 * @param [in] canny_threshold A threshold to apply when converting the input image to black-and-white.  This must be a positive number.
	 *
	 * Examples:
	 * input image description																											| input image							| output image
	 * ---------------------------------------------------------------------------------------------------------------------------------|---------------------------------------|-------------
	 * original colour image																											| @image html coins.jpg ""				| @image html coins_canny_edge_detect.jpg ""
	 * gaussian blur image (3x3 kernel size) @see @ref vz::apply_gaussian_blur() @see @ref vz::Imagination::EMatType::kO2GaussianBlur	| @image html coins_blur_3.jpg ""		| @image html coins_blur_3_canny_edge_detect.jpg ""
	 * gaussian blur image (15x15 kernel size) @see @ref vz::apply_gaussian_blur() @see @ref vz::Imagination::EMatType::kO2GaussianBlur	| @image html coins_blur_15.jpg ""		| @image html coins_blur_15_canny_edge_detect.jpg ""
	 * threshold image @see @ref vz::Imagination::EMatType::kThreshold																	| @image html coins_threshold.jpg ""	| @image html coins_threshold_canny_edge_detect.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kCannyEdgeDetection
	 */
	cv::Mat apply_canny_edge_detection(const cv::Mat & src, const double canny_threshold);


	/** Find the contours in the given source image.  Unlike most other functions in the @p %vz namespace, the output of
	 * this function is not a visual image.  Instead, the output is a vector of contours, each of which is a vector of
	 * points.
	 *
	 * To get a visual representation of the contours, call @ref vz::add_contours_to_image().
	 *
	 * @param [in] src The input image should be a black-and-white image, such as those produced by @ref apply_canny_edge_detection().
	 *
	 * @note For a visual representation of what @p %find_contours() detects, see the output of @ref add_contours_to_image().
	 *
	 * @see @ref vz::Imagination::EMatType::kFindContours
	 * @see @ref vz::add_contours_to_image().
	 */
	VContours find_contours(const cv::Mat & src);


	/** Add the given contours (if any) to the image.
	 * @param [in] src Input image which is cloned prior to drawing the contours.
	 * @param [in] contours The vector of contours to draw.  This is obtained via @ref vz::find_contours().
	 * @param [in] colour The RGB colour to use when drawing the contours.  The default colour is yellow.
	 *
	 * Example:
	 * input image				| output image
	 * -------------------------|-------------
	 * @image html coins.jpg ""	| @image html coins_contours.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kFindContours
	 * @see @ref vz::find_contours().
	 */
	cv::Mat add_contours_to_image(const cv::Mat & src, const VContours & contours, const cv::Scalar & colour=cv::Scalar(0, 255, 255));


	/** Very similar to @ref add_contours_to_image(), but colours the contours based on groupings/limits of the contour areas.
	 */
	cv::Mat add_contours_and_limits_to_image(const cv::Mat & src, const VContours & contours);


	/** Plot the image's histogram.
	 * @param [in] src Input must be either 3-channel (BGR) or 1-channel (greyscale) image.
	 * @param [in] width Width of output histogram plot.
	 * @param [in] height Height of output histogram plot.
	 * @param [in] background_colour Defaults to black, but also common would be @p cv::Scalar(255,255,255) to get a white background.
	 *
	 * Examples of histograms on colour images:
	 * original image					| histogram
	 * ---------------------------------|----------
	 * @image html capture.jpg ""		| @image html histogram_capture_colour.png ""
	 * @image html coins.jpg ""			| @image html histogram_coins_colour.png ""
	 * @image html sudoku.jpg ""		| @image html histogram_sudoku_colour.png ""
	 *
	 * Examples of histograms on greyscale images:
	 * original image					| histogram
	 * ---------------------------------|----------
	 * @image html capture_grey.jpg ""	| @image html histogram_capture_grey.png ""
	 * @image html coins_grey.jpg ""	| @image html histogram_coins_grey.png ""
	 * @image html sudoku_grey.jpg ""	| @image html histogram_sudoku_grey.png ""
	 */
	cv::Mat plot_histogram(const cv::Mat & src, const int width=640, const int height=480, const cv::Scalar background_colour=cv::Scalar(0, 0, 0));


	/** Apply the Hough Lines transform.
	 * @param [in] src Input image must be canny edge detection.
	 * @param [out] lines Vector of 4 integer endpoints, where: @li `lines[i][0]` is @p x1 @li `lines[i][1]` is @p y1 @li `lines[i][2]` is @p x2 @li `lines[i][3]` is @p y2.
	 *
	 * &nbsp;
	 *
	 * Example:
	 * original image				| input image (canny edge)						| output image
	 * -----------------------------|-----------------------------------------------|-------------
	 * @image html sudoku.jpg ""	| @image html threshold_sudoku_result.png ""	| @image html hough_lines.jpg ""
	 *
	 * @see @ref vz::Imagination::EMatType::kHoughLines
	 */
	cv::Mat find_hough_lines(const cv::Mat & src, VV4i & lines);


	/** Rotate an image an arbitrary number of degrees around the exact center of the image.  The image may be extended
	 * to ensure it isn't truncated during the rotation.  New areas will be filled in as black.
	 *
	 * Mathematical degrees are used, meaning the rotation is performed counter-clockwise.  Specifying negative degrees
	 * will result in a clockwise rotation.  Angles greater than 360.0 and lesser than -360.0 are valid, and are
	 * automatically normalized to 0-360.
	 *
	 * @note This function has optimizations to deal with rotations for exact multiples of 90.0 degrees (..., -180, -90, 0, 90, 180, ...).
	 *
	 * Example:
	 * ~~~~
	 * cv::Mat dst = vz::rotate(src, 45.0);
	 * // which is equivalent to:
	 * dst = vz::rotate(src, -315.0);
	 * ~~~~
	 *
	 * &nbsp;
	 *
	 * input image					| output image
	 * -----------------------------|-------------
	 * @image html capture.jpg ""	| @image html capture_rotate_45_degrees.jpg
	 */
	cv::Mat rotate(const cv::Mat & src, double degrees);


	/** Resize the given image, while maintaining the original aspect ratio.  Either the width or the height must be
	 * specified as non-zero to indicate the desired image size.  The other measurement will be calculated to keep the
	 * correct aspect ratio.  If both the width and height have been specified, then it is up to the caller to maintain
	 * the correct aspect ratio.
	 *
	 * @see @ref letterbox()
	 */
	cv::Mat resize(const cv::Mat & src, int width, int height = 0);

	/** Resize the given image, keeping the same aspect ratio.  A scale of 1.0 will return an image of the same size,
	 * while 0.5 will be 1/2 the size, and 2.0 will be twice as large as the original.
	 * @see @ref letterbox()
	 */
	cv::Mat resize(const cv::Mat & src, const double scale);

	/** Resize the image to fit within the given dimensions and apply a border to pad the size until the desired size
	 * is achieved.  The aspect ratio will be maintained.  If the image does not fit perfectly within the given dimensions,
	 * apply letterboxing (black bars on top and bottom or left and right sides) to achieve the desired final dimensions.
	 * Default colour for the letterboxing is black.
	 *
	 * Example:
	 * ~~~~
	 * cv::Mat dst = vz::letterbox(src, 320, 200);
	 * ~~~~
	 *
	 * &nbsp;
	 *
	 * input image					| output image
	 * -----------------------------|-------------
	 * @image html capture.jpg ""	| @image html letterbox_320x200.jpg ""
	 *
	 * @see @ref resize()
	 */
	cv::Mat letterbox(const cv::Mat & src, int width, int height, const cv::Scalar & colour = cv::Scalar(0, 0, 0));

	/// Perform 2 Gaussian blurs of different size, and subtract the 2nd from the 1st.  @see @ref apply_DoG_threshold()
	cv::Mat apply_DoG(const cv::Mat & src, const double k=1.6, const double sigma=0.5, const double gamma=1.0);

	/// Obtain the DoG (Difference of Gaussian) by calling @ref apply_DoG(), then find a threshold image from the result.
	cv::Mat apply_DoG_threshold(const cv::Mat & src, const double k=200.0, const double sigma=0.5, const double gamma=1.0);

	/// Convert all of the contours to rotated rectangles.
	VRotatedRects convert_contours_to_rotated_rects(const VContours & contours);
}
