// VZ Image Compare (C) 2020 Stephane Charette <stephanecharette@gmail.com>
// MIT license applies.  See "license.txt" for details.
// $Id: VzImgCmp.hpp 3035 2020-08-28 19:13:52Z stephane $

#pragma once

#include <opencv2/opencv.hpp>
#include <Imagination.hpp>

// method of interpolation
// used in resizing and warping functions
#define INTERP_METHOD cv::INTER_AREA

// limit of image size width
// in image analysis engine
// images bigger than this will
// be scaled down, keeping the aspect ratio
#define PROCESSING_WITH_LIMIT 1500.f

/// CCR's @p Vz (image manipulation) namespace.
namespace vz
{
	/** This class is used to compare images and find/highlight differences.  There is a single @em master image against
	 * which other images are compared.  The output is a combination of a threshold mask, a sequence of contours (the
	 * differences) and/or an annotated image.
	 *
	 * ~~~~
	 * cv::Mat img = ...; // do something here to get the master image
	 * vz::ImgCmp image_compare;
	 * image_compare.set_flag(vz::ImgCmp::Flags::kAnnotateOverGrey);
	 * image_compare.set_flag(vz::ImgCmp::Flags::kAnnotateAddRedBorder);
	 * image_compare.set_flag(vz::ImgCmp::Flags::kAnnotateAddGreenBorder);
	 * image_compare.set_flag(vz::ImgCmp::Flags::kDrawContour);
	 *
	 * image_compare.set_master_image(img);
	 *
	 * cv::Mat test = ...; // do something here to get the test image
	 * image_compare.compare(test);
	 *
	 * cv::Mat output = image_compare.annotate();
	 * ~~~~
	 *
	 * Usual sequence of events:
	 *
	 * @li Instantiate an object of type @ref vz::ImgCmp using the default constructor.  It can safely be instantiated on the stack.
	 * @li Customize the object by calling @ref vz::ImgCmp::set_flag() or manually settings values such as @ref vz::ImgCmp::min_contour_area.
	 * @li Set the master image with @ref vz::ImgCmp::set_master_image().
	 * @li Compare the master image against a new image with @ref vz::ImgCmp::compare().
	 * @li Copy the structures that are needed, such as @ref differences_contours or @ref differences_threshold.
	 * @li Optionally ask the library to annotate the image with a call to @ref vz::ImgCmp::annotate().
	 * @li Then the cycle repeats with a new call to @ref vz::ImgCmp::compare().
	 */
	class ImgCmp
	{
		public:

			/// Optional features flags.  @see @ref set_flag() @see @ref clear_flag() @see @ref verify_flags()
			enum Flags
			{
				kNone					= 0x0000,

				/// Compare the original full-sized image, or the resized copy? @{
				kDiffOriginalSize		= 0x0001,
				kDiffResized			= 0x0002,
				/// @}

				/// Compare the original colour image, or the greyscale+blurred copy? @{
				kDiffColour				= 0x0004,
				kDiffGreyscale			= 0x0008,
				/// @}

				/// Create the difference threshold using the triangle or Otsu method? @{
				kThresholdTriangle		= 0x0010,
				kThresholdOtsu			= 0x0020,
				/// @}

				/// Determine whether the background image in the annotation is colour or greyscale. @{
				kAnnotateOverGrey		= 0x0040,
				kAnnotateOverColour		= 0x0080,
				/// @}

				/// Annotated image will contain a red border if there are differences.
				kAnnotateAddRedBorder	= 0x0100,

				/// Annotated image will contain a green border if there are no differences.
				kAnnotateAddGreenBorder	= 0x0200,

				/// The contour of differences will be drawn on the annotated image.
				kDrawContour			= 0x0400,

				/// The rotated rectangle which contains the differences will be drawn on the annotated image.
				kDrawRectangle			= 0x0800,
			};

			/// Empty constructor.  You must call @ref set_master_image() before it can be used.
			ImgCmp();

			/// Constructor.  Automatically calls @ref set_master_image() with the given @p cv::Mat image.
			ImgCmp(cv::Mat mat);

			/// Destructor.
			virtual ~ImgCmp();

			/** Reset the object to a known state.  This is automatically called by the constructors, but is @em not called by
			 * @ref set_master_image().
			 */
			virtual ImgCmp & reset();

			/// Get the version string.  Similar to @p "1.0.0-1234".
			virtual std::string version() const;

			/// Set a feature flag.  Optionally can be told to call @ref verify_flags().
			virtual ImgCmp & set_flag(const Flags f, const bool verify = false);

			/// Set a feature flag.  Optionally can be told to call @ref verify_flags().
			virtual ImgCmp & clear_flag(const Flags f, const bool verify = false);

			/** This knows about the groupings of the various flags and will ensure that incompatible flags are not set,
			 * and that at least 1 of each feature flag groups has been set.  This is automatically called by some key
			 * API calls, such as @ref compare().
			 */
			virtual ImgCmp & verify_flags();

			/** Save all the images as .png files using semi-descriptive names for debug purposes.
			 * @returns A vector of all the saved image filenames.
			 */
			virtual VStr debug_save();

			/** Set the master image against which all other images will be compared.  The image is cloned prior to
			 * @p set_master_image() returning, so the original @p cv::Mat can be modified, reused, or destroyed without it
			 * causing any problems.
			 *
			 * @note This does @em not call @ref reset(), so previous customizations such as annotation colour and feature flags
			 * will be retained even when setting a new master image.
			 *
			 * Once you've called @p %set_master_image() (either directly or via the class constructor that takes a @p cv::Mat)
			 * then you may call @ref compare().
			 */
			virtual ImgCmp & set_master_image(cv::Mat img);

			/** Compare the given image against the previously set master image.
			 * @returns The number of differences found in the image.  (This is the number of vectors stored in @ref differences_contours.)
			 *
			 * @note This does not automatically call @ref annotate.  If you want the annotated version of the image, you must explicitly call @ref annotate().
			 *
			 * @see @ref set_master_image()
			 * @see @ref differences
			 * @see @ref differences_threshold
			 * @see @ref differences_contours
			 */
			virtual size_t compare(cv::Mat img, int threshold, int filter_size);

			/** Annotate the most recently compared image.  You must call @ref compare() prior to this method.  The annotated
			 * image will also be stored in @ref annotated_candy.
			 *
			 * @li Annotation on full-colour image: @image html 1588545374-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_contours-7-annotated.png width=25%
			 * @li Annotation where only the differences are in colour: @image html 1588545501-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_contours-draw_rectangles-7-annotated.png width=25%
			 *
			 * @see @ref annotated_candy
			 */
			virtual cv::Mat annotate();
			
			virtual void threshold_and_opening(int threshold, int filter_size);


		protected:

			// This is called by @ref diff_images() to align the two compared images
			virtual void align_images(cv::Mat& a, cv::Mat& b);

			/// This is called by @ref compare() to compute @ref differences.
			virtual ImgCmp & diff_images();


			/// This is called by @ref compare() to get the @ref differences_contours.
			virtual ImgCmp & get_contours();

			// *************************************************
			// ******************** MEMBERS ********************
			// *************************************************

		public:

			/// Original master image as set by @ref set_master_image.  @see @ref master_resized
			vz::Imagination master_original;

			/// Master image reduced to 25% of the original size.  @see @ref master_original  @see @ref resized_image_scale
			vz::Imagination master_resized;

			/// The target image which is compared against the original is called @p candy.  @see @ref candy_resized  @see @ref resized_image_scale
			vz::Imagination candy_original;

			/// The target image which is compared against the original, but resized to match @ref master_resized.
			vz::Imagination candy_resized;

			// The target image which is compared against the original, but aligned in position, rotation and scale to match master
			vz::Imagination candy_aligned;

			/** Scale to apply when resizing images.  Default is 0.25, meaning resized image will be 1/4 the size of the original.
			 * If you change this value, you @em must call @ref set_master_image() again to ensure that @ref master_resized has
			 * been set correctly.
			 */
			double resized_image_scale;

			/** The differences between the master image and the most recent image after calling @ref compare().  This image may
			 * be colour or greyscale (single channel) depending on whether @ref kDiffColour or @ref kDiffGreyscale
			 * has been set.
			 *
			 * Starting with the @ref Settings "example image",
			 * @li colour differences  @image html 1588482608-full_size-51_contours-2_minarea-3_dilate_erode-colour_diff-triangle_threshold-draw_contours-draw_rectangles-5-differences.png width=25%
			 * @li greyscale differences  @image html 1588481728-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_contours-draw_rectangles-5-differences.png width=25%
			 *
			 * @see @ref differences_threshold
			 */
			cv::Mat differences;

			/** The @ref differences converted to a binary mask using one of the thresholds.  There are two threshold methods
			 * available, @em triangle (the default) and @em Otsu.  There may be situations or configurations where Otsu gives
			 * better results than the traditional triangle method, though in my testing of the sample images I have on hand
			 * the traditional triangle method seems to work better.
			 *
			 * Starting with the @ref Settings "example image",
			 * @li triangle threshold  @image html 1588542742-full_size-51_contours-2_minarea-3_dilate_erode-colour_diff-triangle_threshold-draw_contours-draw_rectangles-6-diff_threshold.png width=25%
			 * @li Otsu threshold  @image html 1588542754-full_size-36_contours-2_minarea-3_dilate_erode-colour_diff-otsu_threshold-draw_contours-draw_rectangles-6-diff_threshold.png width=25%
			 *
			 * @see @ref kThresholdTriangle
			 * @see @ref dilate_and_erode
			 */
			cv::Mat differences_threshold;

			/** The contours (each of which is a vector of points) from @ref differences_threshold.  Each contour represents an
			 * important difference found when the image was compared against the master in @ref compare().
			 */
			vz::VContours differences_contours;

			/// The minimum area a difference must occupy to be retained.  Differences smaller than this will be ignored.  Units of measurement is "square pixels".
			double min_contour_area;

			/** The amount of dilation and erosion applied to @ref differences_threshold.  Default is 3.  Set to zero to disable.
			 * Dilation and erosion determine how differences which are near to each other may be combined into a single difference.
			 * Without any dilation and erosion, differences are very sharp and stand out easily; each difference is interpreted
			 * independently.  But differences that are very tiny may be ignored because they fall beneath the threshold of
			 * @ref min_contour_area.
			 *
			 * There needs to be a balance between having too much, and not enough.
			 *
			 * Starting with the @ref Settings "example image",
			 * @li @p dilate_and_erode=0  @image html 1588543669-full_size-153_contours-2_minarea-0_dilate_erode-colour_diff-triangle_threshold-draw_contours-draw_rectangles-6-diff_threshold.png width=25%
			 * @li @p dilate_and_erode=5  @image html 1588543687-full_size-35_contours-2_minarea-5_dilate_erode-colour_diff-triangle_threshold-draw_contours-draw_rectangles-6-diff_threshold.png width=25%
			 * @li @p dilate_and_erode=10  @image html 1588543699-full_size-26_contours-2_minarea-10_dilate_erode-colour_diff-triangle_threshold-draw_contours-draw_rectangles-6-diff_threshold.png width=25%
			 */
			int dilate_and_erode;

			/** The target image which is compared against the original is called @p candy.  This is the annotated version,
			 * highlighting all of the differences.  This image is only generated when @ref annotate() is called.
			 *
			 * Starting with the @ref Settings "example image",
			 * @li With @ref kAnnotateOverGrey, this means the background image is greyscale and only the parts that are different are shown in colour: @image html 1588545296-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_contours-7-annotated.png width=25%
			 * @li With @ref kAnnotateOverColour, the entire image is shown in colour: @image html 1588545374-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_contours-7-annotated.png width=25%
			 * @li With @ref kAnnotateAddRedBorder, a red border is added to the image if any differences have been detected: @image html 1588545415-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_contours-7-annotated.png width=25%
			 * @li With @ref kDrawRectangle and without @ref kDrawContour, rotated rectangles are displayed around each difference: @image html 1588545461-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_rectangles-7-annotated.png width=25%
			 * @li With both @ref kDrawContour and @ref kDrawRectangle, both the contour and rotated rectangles are displayed: @image html 1588545501-full_size-43_contours-2_minarea-3_dilate_erode-greyscale_diff-triangle_threshold-draw_contours-draw_rectangles-7-annotated.png width=25%
			 */
			cv::Mat annotated_candy;

			/** The colour that will be used to annotate the images and highlight the differences.  The default colour is pure red.
			 * @note Remember that OpenCV uses BGR, not RGB.  For example, red is @p "{0, 0, 255}" while blue is @p "{255, 0, 0}".
			 */
			cv::Scalar annotation_colour;

			/// The thickness of the lines used to annotate the images and highlight the differences.  The default is @p 2.
			int annotation_thickness;

			/** Optional flags.  Instead of accessing this member directly, please use @ref set_flag() and @ref clear_flag()
			 * which has additional logic to ensure conflicting features aren't set.
			 */
			Flags flags;

			// the feature detector used for rough alignment procedure
			cv::Ptr<cv::FeatureDetector> detector = cv::ORB::create(2000, 1.2, 8, 91, 0, 2, cv::ORB::ScoreType::HARRIS_SCORE, 91, 10);
			// the feature matcher used to match features detected by @ref detector
			cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMINGLUT);
			// the keypoints of the features detected for the current master image
			std::vector<cv::KeyPoint> keypoints_master;
			// the descriptors of the features detected for the current master image
			cv::Mat descriptors_master;
			
	};
}
