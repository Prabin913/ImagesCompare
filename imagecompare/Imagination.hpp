/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: Imagination.hpp 2969 2020-05-04 16:23:18Z stephane $
 */

#pragma once

#include "MatManip.hpp"

/** @file
 * @p vz::Imagination objects are used to perform several simple image manipulations for the purpose of object detection.
 */


/// @p %vz namespace for C Code Run's "Vision" tools and libraries.
namespace vz
{
	/** @p %Imagination objects are used to perform simple and common image manipulations, as well as act as an image cache
	 * to store variations on an image.  This way, if a variation is requested multiple times, it is retrieved from cache
	 * instead of performing the same calculations multiple times.
	 *
	 * Methods to note include the multiple constructors, @ref get(), and @ref save().
	 */
	class Imagination
	{
		public:

			/// The @p %EMatType represents the numerous variations of an image that can be created using @p %Imagination.  @see @ref get()
			enum class EMatType
			{
				kInvalid				= 0,	///< Empty @p cv::Mat.  Should never come up.
				kOriginalImage			,		///< The original unmodified image.													@image html capture.jpg ""
				kO2Greyscale			,		///< The original image converted to greyscale.										@image html capture_grey.jpg ""							@see @ref vz::to_greyscale()
				kO2GaussianBlur			,		///< The original image with a Gaussian blur (default kernel size is 3x3).			@image html gaussian_blur_3x3.jpg ""					@see @ref vz::apply_gaussian_blur()
				kO2BlurGreyscale		,		///< The original image with a Gaussian blur and then converted to greyscale.		@image html gaussian_blur_3x3_grey.jpg ""				@see @ref vz::apply_blur_and_convert_to_greyscale()
				kO2SimpleColourBalance	,		///< The original image with a simple colour balance applied.						@image html simple_colour_balance.jpg ""				@see @ref vz::apply_simple_colour_balance()
				kO2Invert				,		///< The original image inverted.													@image html coins_invert.jpg ""							@see @ref vz::invert()
				kThreshold				,		///< The @ref EMatType::kO2SimpleColourBalance image converted to black-and-white.	@image html threshold_coins_triangle.png ""				@see @ref vz::Imagination::threshold @see @ref vz::Imagination::create_threshold()
				kErode					,		///< The @ref EMatType::kThreshold image eroded										@image html threshold_coins_result.png ""				@see @ref vz::erode()
				kErodeAndDilate			,		///< The @ref EMatType::kThreshold image both eroded then dilated.					@image html coins_erode_and_dilate_3.jpg ""				@see @ref vz::erode() @see @ref vz::dilate()
				kCannyEdgeDetection		,		///< The @ref EMatType::kThreshold image with edge detection.						@image html coins_threshold_canny_edge_detect.jpg ""	@see @ref vz::apply_canny_edge_detection()
				kFindContours			,		///< Contour detection with contours drawn over @ref EMatType::kOriginalImage.		@image html coins_contours.jpg ""						@see @ref vz::Imagination::contours @see @ref vz::find_contours() @see @ref vz::add_contours_to_image()
				kFindContoursIntervals	,		///< Similar to @ref EMatType::kFindContours but includes intervals.				@image html coins_contours_intervals.jpg ""				@see @ref vz::Imagination::contours_intervals @see @ref vz::intervals()
				kO2Histogram			,		///< Histogram of the original image												@image html histogram.png ""							@see @ref vz::plot_histogram()
				kHoughLines				,		///< Hough lines transform.															@image html hough_lines.jpg ""							@see @ref vz::Imagination::hough_lines @see @ref vz::find_hough_lines()
				kO2AngleCorrected		,		///< The original image with an angle correction.									@image html hough_lines.jpg ""							@see @ref vz::Imagination::hough_lines_correction_angle
				kMosaic					,		///< Combines thumbnails of the other images into a single large mosaic.			@image html mosaic.jpg									@see @ref vz::Imagination::create_mosaic()
				kO2DoG					,		///< Difference of Gaussian.
				kThresholdOtsu			,
				kThresholdTriangle		,
				kThresholdGaussian		,
				kThresholdMeanC			,
				kThresholdDoG			,		///< Threshold based on @ref EMatType::kO2DoG.

#if 0
				kBlurGraySobel			,		///< Blur + greyscale + sobel.
				kMorphologicalSkeleton	,		///< Skeleton (http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/)
#endif
				kUserDefined			,		///< Any custom image added to the map will come back as "user defined".  This may apply to multiple images, while the other enums apply to a single image.

				// remember to update to_string()
				kMax
			};

			enum class EThresholdType
			{
				kInvalid				= 0,
				kOther					,
				kOtsu					,
				kTriangle				,
				kAutoGaussian			,
				kAutoMeanC				,
				kDifferenceOfGaussian	,	///< aka DoG
				kMax
			};

			/** Map where the key is an image type description, and the value is a OpenCV @p cv::Mat object.  While
			 * the map may be referenced directly, is is preferable to use @ref get() or @ref operator[] to ensure
			 * the requested image is built and cached correctly.
			 *
			 * Example:
			 * ~~~~
			 * vz::Imagination img("capture.jpg");
			 * cv::Mat mat = img.get(vz::Imagination::EMatType::kO2Greyscale);
			 * process(mat);
			 * ~~~~
			 *
			 * @since 2019-04 Even though the key is now a @p std::string instead of @p EMatType, the enum is still
			 * the preferred way to access the images stored within an @p %Imagination object.
			 * @{
			 */
			typedef std::map<std::string, cv::Mat> Mats;
			Mats mats;
			/// @}

			/// Auto-detected threshold value.  This is only set once the @ref EMatType::kThreshold image is created.
			double threshold_value;

			/// The threshold type that was used.  This is only set once the @ref EMatType::kThreshold image is created.
			EThresholdType threshold_type;

			/** A vector of contours.  This is only set once the @ref EMatType::kFindContours image is created.  This
			 * can then be used with functions such as @p cv::contourArea() and @ref vz::add_contours_to_image().
			 */
			vz::VContours contours;

			/// Contour intervals.  This is only set once the @ref EMatType::kFindContoursIntervals image is created.
			vz::VVDouble contours_intervals;

			/** The Hough lines, angles, angle intervals, and the correction angle are automatically calculated by
			 * @ref create_hough_lines_and_angles() once @ref get() is called with a parameter of @ref EMatType::kHoughLines.
			 * @{
			 */
			vz::VV4i		hough_lines;					///< All detected Hough lines.  @see @ref vz::find_hough_lines()
			vz::VDouble		hough_lines_angles;				///< A vector of all angles for all lines in @ref hough_lines.
			vz::VVDouble	hough_lines_angle_intervals;	///< The interval composed of all angles in @ref hough_lines_angles.  @see @ref vz::intervals()
			double			hough_lines_correction_angle;	///< The best angle according to @ref hough_lines_angle_intervals.
			/// @}

			/// Empty constructor.  Use @ref set_original(), an assignment, or one of the other explicit constructors to set the original image.
			Imagination();

			/// Constructor with an existing image file.
			Imagination(const std::string & original_image_filename);

			/// Constructor with an existing OpenCV image.
			Imagination(const cv::Mat & original_image);

			/// Destructor.
			virtual ~Imagination();

			/// Convert the given name to @ref EMatType.  If the name is not an exact match, will return @ref EMatType::kInvalid.
			virtual EMatType to_type(const std::string & name) const;

			/// Provide a simple text name for the given mat image type.  This is also used as the key to store images in @ref mats.
			virtual std::string to_string(const EMatType type) const;

			/// Provide a simple text name for the given threshold type.
			virtual std::string to_string(const EThresholdType type) const;

			/** Deep comparison of the @b original image only.  The rest of the images can always be derived from the original which
			 * is why we only need to verify that the original images are the same.  The comparison is pixel-by-pixel, so any image
			 * difference will be detected.
			 */
			virtual bool operator==(const Imagination & rhs) const;

			/// Similar to @ref operator==().
			inline bool operator!=(const Imagination & rhs) const { return !(operator==(rhs)); }

			/// Clear all of the image @p cv::Mat objects cached in @ref mats.
			virtual Imagination & clear();

			/// Clear a specific image type from @ref mats.  The image will be re-created the next time it is requested.  @{
			virtual Imagination & clear(const EMatType type);
			virtual Imagination & clear(const std::string & name);
			/// @}

			/// Determine if the given image has been created.  @{
			inline bool exists(const EMatType type) const { return exists(to_string(type)); }
			inline bool exists(const std::string & name) const { return mats.count(name) == 1; }
			/// @}

			/// Determine if an image has been set.  @see @ref set_original() @see @ref clear()
			inline bool empty() const { return !(exists(EMatType::kOriginalImage)); }

			/** Determine the number of images that currently exist in the @ref mats image cache.  This includes the original image.
			 * @note This is @b not the pixel dimensions of the image!
			 */
			inline size_t size() const { return mats.size(); }

			/// Reset this object and use the given image.  All previous images in @ref mats are cleared prior to the new image being set. @{
			virtual Imagination & set_original(const std::string & original_image_filename);
			virtual Imagination & set_original(const cv::Mat & original_image);
			virtual Imagination & set_original(const EMatType type);
			/// @}

			/** Get the specified image from cache.  If the requested image does not exist, it is created from the original image and cached.
			 * Example:
			 * ~~~~
			 * vz::Imagination img("capture.jpg");
			 * process(img.get(vz::Imagination::EMatType::kO2Greyscale));
			 * ~~~~
			 * @{
			 */
			virtual cv::Mat & get(const EMatType type = EMatType::kOriginalImage);
			virtual cv::Mat & get(const std::string & name);
			/// @}

			/** Similar to the other @ref get() method, but with the ability to pass in a single integer parameter.  This is used
			 * by several specific types to alter the default behaviour.  Other types that don't use a parameter will fall through
			 * to the original @ref get() method.
			 *
			 * @note For the types named in table below, the image will be cleared from cache (if it exists) and re-created with
			 * the specified parameter.
			 *
			 * Types that support a parameter:
			 * type									| parameter
			 * -------------------------------------|----------
			 * @ref EMatType::kO2GaussianBlur		| kernel_size for @ref vz::apply_gaussian_blur()
			 * @ref EMatType::kO2BlurGreyscale		| kernel_size for @ref vz::apply_gaussian_blur()
			 * @ref EMatType::kErode				| number of iterations for @ref vz::erode()
			 * @ref EMatType::kErodeAndDilate		| number of iterations for both @ref vz::erode() and @ref vz::dilate()
			 * @ref EMatType::kCannyEdgeDetection	| canny threshold for @ref vz::apply_canny_edge_detection()
			 */
			virtual cv::Mat & get(const EMatType type, const int parm);

			/** Alias for @ref get().
			 * Example:
			 * ~~~~
			 * vz::Imagination img("capture.jpg");
			 * cv::Mat mat = img[vz::Imagination::EMatType::kO2Greyscale];
			 * ~~~~
			 * @{
			 */
			inline cv::Mat & operator[](const EMatType type) { return get(type); }
			inline cv::Mat & operator[](const std::string & name) { return get(name); }
			/// @}

			/** Replace the given image type.  If an image already exists for this type, then it is replaced.  If an image type
			 * does not yet exist, then it is inserted into the map.  If the image is empty, then the type is completely removed
			 * from @ref mats.  @see @ref set_original()
			 * @{
			 */
			virtual Imagination & replace(const EMatType type, cv::Mat & image);
			virtual Imagination & replace(const std::string & name, cv::Mat & image);
			/// @}

			/** Clears the object and resets the original image.  All of the image types (except for @ref EMatType::kOriginalImage)
			 * will need to be re-calculated.  This used to be a separate function but is now an alias for @ref set_original().
			 */
			virtual Imagination & reset() { return set_original(EMatType::kOriginalImage); }

			/** Clears the object and resets the original image to the given image type.
			 * This used to be a separate function but is now an alias for @ref set_original().
			 * @{
			 */
			virtual Imagination & reset_using(const EMatType type) { return set_original(type); }
			virtual Imagination & reset_using(const std::string & name) { return set_original(name); }
			/// @}

			/** Resize (scale) the given image.  Either the width or the height (but not both) should be specified as zero,
			 * in which case it will be calculated using the other dimension to maintain the original aspect ratio.
			 * @{
			 */
			inline cv::Mat resize(const EMatType type, int width, int height = 0) { return vz::resize(get(type), width, height); }
			inline cv::Mat resize(const std::string & name, int width, int height = 0) { return vz::resize(get(name), width, height); }
			/// @}

			/** Resize (scale) the given image.  A scale less than 1.0 makes the image smaller, while a scale greater
			 * than 1.0 makes the image larger.
			 * @{
			 */
			inline cv::Mat resize(const EMatType type, const double scale) { return vz::resize(get(type), scale); }
			inline cv::Mat resize(const std::string & name, const double scale) { return vz::resize(get(name), scale); }
			/// @}

			/** Save the given image to disk.  Returns the filename used, which is based on @ref to_string().  The image filename
			 * will be appended to the prefix (if any is specified), and unless the prefix includes an absolute or relative path,
			 * will be saved into the current working directory.
			 * @{
			 */
			virtual std::string save(const EMatType type, const std::string & prefix="");
			virtual std::string save(const std::string & name, const std::string & prefix="");
			/// @}

			/** Save all of the image files to disk by repeatedly calling @ref save() with all the image types.  Returns a
			 * vector of all the files that were saved.  @see @ref save()
			 */
			virtual VStr save_all(const std::string & prefix="");

			/** Find the contours of an image using @ref EMatType::kCannyEdgeDetection and @ref vz::find_contours().
			 * The results are cached as @ref Imagination::contours for later use.  This method is called when @ref get()
			 * is invoked with a parameter of @ref EMatType::kFindContours or @ref EMatType::kFindContoursIntervals.
			 */
			virtual vz::VContours find_contours();

			/** Get a decent binary image.  This will try 2 different methods of creating a binary image, automatically return
			 * the best image, and discard the other.
			 *
			 * This method is called when @ref get() is invoked with a parameter of @ref EMatType::kThreshold.
			 *
			 * @note If the resulting image seems to have a white background, the binary image will be inverted so
			 * that objects appear as white-on-black.
			 *
			 * Examples of binary threshold images created by this method:
			 *
			 * original image									| triangle threshold							| gaussian threshold							| output
			 * -------------------------------------------------|-----------------------------------------------|-----------------------------------------------|-------
			 * @image html threshold_capture_balanced.jpg ""	| @image html threshold_capture_triangle.png ""	| @image html threshold_capture_gaussian.png ""	| @image html threshold_capture_result.png ""
			 * @image html threshold_coins_balanced.jpg ""		| @image html threshold_coins_triangle.png ""	| @image html threshold_coins_gaussian.png ""	| @image html threshold_coins_result.png ""
			 * @image html threshold_sudoku_balanced.jpg ""		| @image html threshold_sudoku_triangle.png ""	| @image html threshold_sudoku_gaussian.png ""	| @image html threshold_sudoku_result.png ""
			 */
			virtual cv::Mat create_threshold();

			/** Find the Hough lines, and make the necessary calculations to determine the angle of each line.
			 *
			 * This method is called when @ref get() is invoked with a parameter of @ref EMatType::kHoughLines.
			 *
			 * The lines, angles, intervals, and best angle are stored in @ref hough_lines, @ref hough_lines_angles,
			 * @ref hough_lines_angle_intervals, and @ref hough_lines_correction_angle.
			 */
			virtual cv::Mat create_hough_lines_and_angles();

			/// Create a mosaic using a thumbnail of all the images in @ref mats.  This is mostly for debug or logging purposes.
			virtual cv::Mat create_mosaic(const bool show_labels = true);

			virtual cv::Mat create_threshold(const EThresholdType type);
	};
}


/// Provide a simple text name for the given mat image type.
inline std::string to_string(const vz::Imagination::EMatType type)
{
	return vz::Imagination().to_string(type);
}


/// Provide a simple text name for the given threshold type.
inline std::string to_string(const vz::Imagination::EThresholdType type)
{
	return vz::Imagination().to_string(type);
}
