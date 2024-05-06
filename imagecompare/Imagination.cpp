/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: Imagination.cpp 2769 2019-05-15 11:44:56Z stephane $
 */

#include "Imagination.hpp"
#include "Intervals.hpp"
#include "MatManip.hpp"

#define M_PI	3.141592f

vz::Imagination::Imagination()
{
	clear();

	return;
}


vz::Imagination::Imagination(const std::string & original_image_filename) :
	vz::Imagination()
{
	set_original(original_image_filename);

	return;
}


vz::Imagination::Imagination(const cv::Mat & original_image) :
	vz::Imagination()
{
	set_original(original_image);

	return;
}


vz::Imagination::~Imagination()
{
	return;
}


vz::Imagination::EMatType vz::Imagination::to_type(const std::string & name) const
{
	for (int idx = 0; idx <= static_cast<int>(EMatType::kMax); idx ++)
	{
		const EMatType type = static_cast<EMatType>(idx);
		if (to_string(type) == name)
		{
			return type;
		}
	}

	return EMatType::kUserDefined;
}


std::string vz::Imagination::to_string(const EMatType type) const
{
	switch (type)
	{
		case EMatType::kInvalid:				return "invalid";
		case EMatType::kOriginalImage:			return "original";
		case EMatType::kO2Greyscale:			return "greyscale";
		case EMatType::kO2GaussianBlur:			return "gaussian_blur";
		case EMatType::kO2BlurGreyscale:		return "blur_greyscale";
		case EMatType::kO2SimpleColourBalance:	return "simple_colour_balance";
		case EMatType::kO2Invert:				return "invert";
		case EMatType::kThreshold:				return "threshold";
		case EMatType::kErode:					return "erode";
		case EMatType::kErodeAndDilate:			return "erode_and_dilate";
		case EMatType::kCannyEdgeDetection:		return "canny_edge_detection";
		case EMatType::kFindContours:			return "find_contours";
		case EMatType::kFindContoursIntervals:	return "find_contours_intervals";
		case EMatType::kO2Histogram:			return "histogram";
		case EMatType::kHoughLines:				return "hough_lines";
		case EMatType::kO2AngleCorrected:		return "angle_corrected";
		case EMatType::kMosaic:					return "mosaic";
		case EMatType::kO2DoG:					return "dog";
		case EMatType::kThresholdOtsu:			return "threshold_otsu";
		case EMatType::kThresholdTriangle:		return "threshold_triangle";
		case EMatType::kThresholdGaussian:		return "threshold_gaussian";
		case EMatType::kThresholdMeanC:			return "threshold_meanc";
		case EMatType::kThresholdDoG:			return "threshold_dog";
		case EMatType::kUserDefined:			return "user_defined";
		case EMatType::kMax:					return "max";

		// do not put a "default:" in this switch statement, otherwise
		// the compiler wont warn us when we add a new type to EMatType
	}

	return "unknown";
}


std::string vz::Imagination::to_string(const EThresholdType type) const
{
	switch (type)
	{
		case EThresholdType::kInvalid:				return "invalid";
		case EThresholdType::kOther:				return "other";
		case EThresholdType::kOtsu:					return "otsu";
		case EThresholdType::kTriangle:				return "triangle";
		case EThresholdType::kAutoGaussian:			return "auto_gaussian";
		case EThresholdType::kAutoMeanC:			return "auto_mean_c";
		case EThresholdType::kDifferenceOfGaussian:	return "difference_of_gaussian";
		case EThresholdType::kMax:					return "max";
	}

	return "unknown";
}


bool vz::Imagination::operator==(const vz::Imagination & rhs) const
{
	if (empty() && rhs.empty())
	{
		// compare two empty objects, so obviously they're equal
		return true;
	}

	if (empty() != rhs.empty())
	{
		// compare an empty and non-empty image, so obviously they're different
		return false;
	}

	// if we get here, we have a more complex scenario -- both of the objects have an original image which must be compared

	const std::string original_image_name = to_string(EMatType::kOriginalImage);

	const cv::Mat & lhs_mat = this->mats.at(original_image_name);
	const cv::Mat & rhs_mat = rhs.	mats.at(original_image_name);

	return vz::compare_exact(lhs_mat, rhs_mat);
}


vz::Imagination & vz::Imagination::clear()
{
	mats							.clear();
	contours						.clear();
	contours_intervals				.clear();
	hough_lines						.clear();
	hough_lines_angles				.clear();
	hough_lines_angle_intervals		.clear();
	hough_lines_correction_angle	= 0.0;
	threshold_value					= 0.0;
	threshold_type					= EThresholdType::kInvalid;

	return *this;
}


vz::Imagination & vz::Imagination::clear(const EMatType type)
{
	cv::Mat mat;
	return replace(type, mat);
}


vz::Imagination & vz::Imagination::clear(const std::string & name)
{
	cv::Mat mat;
	return replace(name, mat);
}


vz::Imagination & vz::Imagination::set_original(const std::string & original_image_filename)
{
	clear();

	cv::Mat img = cv::imread(original_image_filename, cv::IMREAD_COLOR);
	if (img.empty())
	{
		/// @throw std::runtime_error if the image fails to load (file does not exist?)
		throw std::runtime_error("failed to read \"" + original_image_filename + "\"");
	}

	return set_original(img);
}


vz::Imagination & vz::Imagination::set_original(const cv::Mat & original_image)
{
	clear();

	/// @note If the @p cv::Mat is empty, then this is equivalent to calling @ref clear().
	if (original_image.empty() == false)
	{
		mats[to_string(EMatType::kOriginalImage)] = original_image.clone();
	}

	return *this;
}


vz::Imagination & vz::Imagination::set_original(const EMatType type)
{
	// make sure the image we need extendeds beyond the scope of the call to set_original(),
	// otherwise the image will be deleted when clear() is called within set_original()
	cv::Mat new_image = get(type);

	return set_original(new_image);
}


cv::Mat & vz::Imagination::get(const EMatType type)
{
	const std::string name = to_string(type);

	if (!exists(name))
	{
		// if we get here, then the requested image type doesn't exist yet so we need to create it

		switch (type)
		{
			case EMatType::kInvalid:
			case EMatType::kUserDefined:
			case EMatType::kMax:
			{
				/// @throw std::out_of_range if the type is invalid
				throw std::out_of_range("cannot get an image corresponding to type \"" + name + "\"");
			}
			case EMatType::kOriginalImage:
			{
				/// @throw std::logic_error if the original image has not been set
				throw std::logic_error("original image has not been set");
			}
			case EMatType::kCannyEdgeDetection:
			{
				// make sure we get the threshold value first prior to calling canny edge detection
				cv::Mat src = get(EMatType::kThreshold);

				// use that same threshold value to run canny edge detection
				mats[name] = vz::apply_canny_edge_detection(src, threshold_value);
				break;
			}
			case EMatType::kFindContours:
			{
				find_contours();
				mats[name] = vz::add_contours_to_image(get(EMatType::kOriginalImage), contours);
				break;
			}
			case EMatType::kFindContoursIntervals:
			{
				find_contours();
				mats[name] = vz::add_contours_and_limits_to_image(get(EMatType::kOriginalImage), contours);
				break;
			}
			case EMatType::kO2AngleCorrected:
			{
				get(EMatType::kHoughLines);
				mats[name] = vz::rotate(get(EMatType::kOriginalImage), hough_lines_correction_angle);
				break;
			}
			case EMatType::kO2Greyscale:			mats[name] = vz::to_greyscale(					get(EMatType::kOriginalImage));					break;
			case EMatType::kO2GaussianBlur:			mats[name] = vz::apply_gaussian_blur(			get(EMatType::kOriginalImage));					break;
			case EMatType::kO2BlurGreyscale:		mats[name] = vz::to_greyscale(					get(EMatType::kO2GaussianBlur));				break;
			case EMatType::kO2SimpleColourBalance:	mats[name] = vz::apply_simple_colour_balance(	get(EMatType::kOriginalImage));					break;
			case EMatType::kO2Invert:				mats[name] = vz::invert(						get(EMatType::kOriginalImage));					break;
			case EMatType::kErode:					mats[name] = vz::erode(							get(EMatType::kThreshold));						break;
			case EMatType::kErodeAndDilate:			mats[name] = vz::dilate(						get(EMatType::kErode));							break;
			case EMatType::kO2Histogram:			mats[name] = vz::plot_histogram(				get(EMatType::kOriginalImage));					break;
			case EMatType::kO2DoG:					mats[name] = vz::apply_DoG(						get(EMatType::kOriginalImage));					break;
			case EMatType::kThreshold:				mats[name] = create_threshold();																break;
			case EMatType::kHoughLines:				mats[name] = create_hough_lines_and_angles();													break;
			case EMatType::kMosaic:					mats[name] = create_mosaic();																	break;
			case EMatType::kThresholdOtsu:			mats[name] = create_threshold(EThresholdType::kOtsu);											break;
			case EMatType::kThresholdTriangle:		mats[name] = create_threshold(EThresholdType::kTriangle);										break;
			case EMatType::kThresholdGaussian:		mats[name] = create_threshold(EThresholdType::kAutoGaussian);									break;
			case EMatType::kThresholdMeanC:			mats[name] = create_threshold(EThresholdType::kAutoMeanC);										break;
			case EMatType::kThresholdDoG:			mats[name] = create_threshold(EThresholdType::kDifferenceOfGaussian);							break;
		}

		// by the time we get here, the requested image type should now exist in the "mats" image cache

		if (!exists(name))
		{
			/// @throw std::logic_error if the specified image type failed to load
			throw std::logic_error("failed to get or create image type \"" + name + "\"");
		}
	}

	return mats.at(name);
}


cv::Mat & vz::Imagination::get(const std::string & name)
{
	if (!exists(name))
	{
		// see if we can generate the requested image

		const EMatType type = to_type(name);
		if (type != EMatType::kInvalid && type != EMatType::kUserDefined)
		{
			return get(type);
		}
	}

	if (exists(name))
	{
		return mats.at(name);
	}

	/// @throw std::logic_error if the specified image type does not exist
	throw std::logic_error("failed to get or create image type \"" + name + "\"");
}


cv::Mat & vz::Imagination::get(const EMatType type, const int parm)
{
	const std::string name = to_string(type);

	// only a few types are currently supported by this method; the rest will fall through to the other get() method
	switch (type)
	{
		case EMatType::kO2GaussianBlur:		mats[name] = vz::apply_gaussian_blur(		get(EMatType::kOriginalImage)	, parm);	break;
		case EMatType::kO2BlurGreyscale:	mats[name] = vz::to_greyscale(				get(EMatType::kO2GaussianBlur	, parm));	break;
		case EMatType::kErode:				mats[name] = vz::erode(						get(EMatType::kThreshold)		, parm);	break;
		case EMatType::kErodeAndDilate:		mats[name] = vz::dilate(					get(EMatType::kErode, parm)		, parm);	break;
		case EMatType::kCannyEdgeDetection:	mats[name] = vz::apply_canny_edge_detection(get(EMatType::kThreshold)		, parm);	break;
		default:																													break;
	}

	return get(type);
}


vz::Imagination & vz::Imagination::replace(const EMatType type, cv::Mat & image)
{
	return replace(to_string(type), image);
}


vz::Imagination & vz::Imagination::replace(const std::string & name, cv::Mat & image)
{
	const EMatType type = to_type(name);

	if (type == EMatType::kThreshold)
	{
		threshold_value = 0.0;
		threshold_type = EThresholdType::kOther;
	}

	if (type == EMatType::kOriginalImage)
	{
		set_original(image);
	}
	else if (image.empty())
	{
		mats.erase(name);
	}
	else
	{
		mats[name] = image;
	}

	return *this;
}


std::string vz::Imagination::save(const EMatType type, const std::string & prefix)
{
	return save(to_string(type), prefix);
}


std::string vz::Imagination::save(const std::string & name, const std::string & prefix)
{
	const EMatType type = to_type(name);

	std::stringstream ss;
	ss << prefix << std::setw(2) << std::setfill('0') << static_cast<int>(type) << "_" << name << ".jpg";
	const std::string filename = ss.str();

	cv::Mat mat = get(name);
	cv::imwrite(filename, mat, {cv::IMWRITE_JPEG_QUALITY, 75, cv::IMWRITE_JPEG_OPTIMIZE, 1});

	return filename;
}


vz::VStr vz::Imagination::save_all(const std::string & prefix)
{
	VStr v;

	// generate all of the "standard" images
	for (int idx=0; idx<static_cast<int>(EMatType::kMax); idx++)
	{
		const EMatType type = static_cast<EMatType>(idx);
		if (type == EMatType::kInvalid		||
			type == EMatType::kUserDefined	||
			type == EMatType::kMosaic		)
		{
			continue;
		}
		get(type);
	}

	// wait until we've generated every image before we create the final mosaic
	get(EMatType::kMosaic);

	// iterate over *every* image, including all of the user-defined ones
	for ( auto iter : mats)
	{
		const std::string & name = iter.first;

		v.push_back(save(name, prefix));
	}

	return v;
}


vz::VContours vz::Imagination::find_contours()
{
	if (contours.empty())
	{
		contours = vz::find_contours(get(EMatType::kCannyEdgeDetection));
	}

	return contours;
}


cv::Mat vz::Imagination::create_threshold(void)
{
#if 0
	cv::Mat balanced	= get(EMatType::kO2SimpleColourBalance);
	cv::Mat blurred		= vz::apply_gaussian_blur(balanced);
	cv::Mat greyscale	= vz::to_greyscale(blurred);

	// since we took the time to create the blurred and greyscale images, see if we should add them to the cache,
	// even though these images are not based on the original but instead based on the colour-balanced image
	if (not exists(EMatType::kO2GaussianBlur))
	{
		mats[to_string(EMatType::kO2GaussianBlur)] = blurred;
	}
	if (not exists(EMatType::kO2BlurGreyscale))
	{
		mats[to_string(EMatType::kO2BlurGreyscale)] = greyscale;
	}
#endif

	// get the various types of threshold images to prime the mat cache
	get(EMatType::kThresholdOtsu	);
	get(EMatType::kThresholdTriangle);
	get(EMatType::kThresholdGaussian);
	get(EMatType::kThresholdMeanC	);
	get(EMatType::kThresholdDoG		);

	cv::Mat dst;
	const int w = dst.cols;
	const int h = dst.rows;
	const cv::Rect roi(w/4, h/4, w/2, h/2); // rectangle covers the middle 1/2 of the image

	// first we try the "OTSU" method
	dst = get(EMatType::kThresholdOtsu	);

	// examine the middle of the image to try and determine if the threshold returned is acceptable;
	// we decide this by looking at the ratio of black and white pixels in the middle of the image

	// check to see if the image looks usable
	double white_count = cv::countNonZero(cv::Mat(dst, roi));
	double total_count = roi.area();
	double white_percentage = white_count / total_count;
	if (white_percentage < 0.1 || white_percentage > 0.9)
	{
		// if we get here, then the image seems to be either all-white or all-black,
		// so try one of the other methods instead which hopefully provides better results

		// next one we try is the "TRIANGLE" method
		dst = get(EMatType::kThresholdTriangle);
		threshold_type = EThresholdType::kTriangle;

		white_count = cv::countNonZero(cv::Mat(dst, roi));
		white_percentage = white_count / total_count;
		if (white_percentage < 0.1 || white_percentage > 0.9)
		{
			dst = get(EMatType::kThresholdGaussian);
			threshold_type = EThresholdType::kAutoGaussian;
		}
	}

	// Remember, what we want in the end is WHITE OBJECT on a BLACK BACKGROUND!

	white_count			= cv::countNonZero(dst);
	total_count			= dst.rows * dst.cols;
	white_percentage	= white_count / total_count;
	if (white_percentage > 0.3)
	{
		// if we get here, then we might have a black object on a white background,
		// so invert the threshold since we instead want a light object on a dark background
		dst = vz::invert(dst);
	}

	// now that we've created a threshold binary image, store it in the cache for possible future use
	mats[to_string(EMatType::kThreshold)] = dst;

	return dst;
}


cv::Mat vz::Imagination::create_hough_lines_and_angles()
{
	hough_lines					.clear();
	hough_lines_angles			.clear();
	hough_lines_angle_intervals	.clear();
	hough_lines_correction_angle = 0.0;

	cv::Mat dst = vz::find_hough_lines(get(EMatType::kThreshold), hough_lines);
	mats[to_string(EMatType::kHoughLines)] = dst;

	const double rads_to_degs = 180.0 / M_PI;

	for (const auto & v4i : hough_lines)
	{
		const double dx		= v4i[0] - v4i[2];
		const double dy		= v4i[1] - v4i[3];
		const double rads	= std::atan2(dy, dx);
		const double degs	= rads * rads_to_degs;
		hough_lines_angles.push_back(degs);
	}

	hough_lines_angle_intervals		= vz::intervals(hough_lines_angles);

//	std::cout << "hough line angle intervals: " << vz::to_string(hough_lines_angle_intervals) << std::endl;

	hough_lines_correction_angle	= vz::best_value_from_intervals(hough_lines_angle_intervals);

	return dst;
}


cv::Mat vz::Imagination::create_mosaic(const bool show_labels)
{
	// first we need to determine how many (and exactly which) images are available

	std::multimap<EMatType, std::string> all_names;

	for (const auto iter : mats)
	{
		const std::string & name = iter.first;
		const EMatType type = to_type(name);

		// the mosaic should not include itself, so skip it
		if (type == EMatType::kMosaic)
		{
			continue;
		}

		// if we get here, then we know we have an image corresponding to this type

		all_names.insert( { type, name } );
	}

	if (all_names.empty())
	{
		return cv::Mat();
	}

	// we need to make a grid with all the images...but how many images per row should we use?
	int cols = 0;
	switch (all_names.size())
	{
		case 1:		cols = 1; break;
		case 2:		cols = 2; break;
		case 3:		cols = 3; break;
		case 4:		cols = 2; break;
		case 5:		cols = 3; break;
		case 6:		cols = 3; break;
		case 7:		cols = 4; break;
		case 8:		cols = 4; break;
		case 9:		cols = 3; break;
		case 10:	cols = 5; break;
		case 11:	cols = 4; break;
		case 12:	cols = 4; break;
		case 13:	cols = 5; break;
		case 14:	cols = 5; break;
		case 15:	cols = 5; break;
		case 16:	cols = 4; break;
		case 17:	cols = 6; break;
		case 18:	cols = 6; break;
		case 19:	cols = 5; break;
		case 20:	cols = 5; break;
		default:	cols = 6; break;
	}

	int rows = all_names.size() / cols;
	if (cols * rows < (int)all_names.size())
	{
		rows ++;
	}

	// every image will be resized to the same width/height
	cv::Mat src = get(EMatType::kOriginalImage);
	if (src.cols > 300 || src.rows > 200)
	{
		src = vz::resize(src, 300, 0);
	}
	const auto w = src.cols;
	const auto h = src.rows;

	// single-pixel blank lines between the images are called "spacers"
	const int number_of_h_spacers = rows + 1;
	const int number_of_v_spacers = cols + 1;

	// remember where the next image needs to be inserted
	int x_idx = 0;
	int y_idx = 0;

	// now that we know the size of each image and the number of cols/rows, we can create the mosaic image
	cv::Mat output(h * rows + number_of_h_spacers, w * cols + number_of_v_spacers, CV_8UC3, cv::Scalar(0, 0, 0));

	for (const auto iter : all_names)
	{
		const EMatType type			= iter.first;
		const std::string & name	= iter.second;

		src = get(name);

		switch(src.type())
		{
			case CV_8UC3:
			{
				// most likely scenario -- nothing to do
				break;
			}
			case CV_8UC1:
			{
				// 8-bit single channel (grayscale) needs to be converted to 3-channel colour image
				cv::Mat tmp;
				cv::cvtColor(src, tmp, cv::COLOR_GRAY2BGR);
				src = tmp;
				break;
			}
			default:
			{
				// this will probably cause a problem when we call copyTo() but let's try and see what happens
				break;
			}
		}

		if (src.cols != w || src.rows != h)
		{
			src = vz::resize(src, w, h);
		}

		if (show_labels)
		{
			// draw a box and put the name of the image into it
			const std::string	label			= std::to_string(static_cast<int>(type)) + "_" + name;
			const int			font_face		= cv::FONT_HERSHEY_SIMPLEX;
			const double		font_scale		= 0.4;
			const int			font_thickness	= 1;
			int					font_baseline	= 0;
			const auto			text_size		= cv::getTextSize(label, font_face, font_scale, font_thickness, &font_baseline);
			font_baseline += font_thickness;

			const cv::Point p(5, 5 + text_size.height);
			const cv::Rect r( cv::Point(p.x - 2, p.y - text_size.height - 2), cv::Size(text_size.width + 4, text_size.height + font_baseline + 4));
			cv::rectangle(src, r, cv::Scalar(0, 0, 0), cv::FILLED);	// black filled rectangle...
			cv::rectangle(src, r, cv::Scalar(0, 0, 255), 1);		// ...with a thin red border

			cv::putText(src, label, p, font_face, font_scale, cv::Scalar(255, 255, 0), font_thickness, cv::LINE_AA);
		}

		// create a region of interest into which we'll copy this resized image to the mosaic
		// (+1 to both width and height because we want to leave single-pixel spacers between images)
		const int x = 1 + x_idx * (w + 1);
		const int y = 1 + y_idx * (h + 1);
		cv::Rect roi(x, y, w, h);
		src.copyTo(output(roi));

		// move our row/col indexes to the location of the next image
		x_idx ++;
		if (x_idx >= cols)
		{
			x_idx = 0;
			y_idx ++;
		}
	}

	// draw the spacers in a colour other than black, since threshold images already contain lots of black which can make the spacers difficult to see
	for (int idx = 0; idx < number_of_h_spacers; idx ++)
	{
		const int x1 = 0;
		const int x2 = output.cols;
		const int y1 = idx * (h + 1);
		const int y2 = y1;

		cv::line(output, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255));
	}
	for (int idx = 0; idx < number_of_v_spacers; idx ++)
	{
		const int x1 = idx * (w + 1);
		const int x2 = x1;
		const int y1 = 0;
		const int y2 = output.rows;

		cv::line(output, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255));
	}

	return output;
}


cv::Mat vz::Imagination::create_threshold(const EThresholdType type)
{
	cv::Mat dst;

	cv::Mat greyscale_and_blurred = get(EMatType::kO2BlurGreyscale);

	switch(type)
	{
		case EThresholdType::kInvalid:
		case EThresholdType::kOther:
		case EThresholdType::kMax:
		{
			/// @throw std::invalid_argument if the threshold type is invalid
			throw std::invalid_argument("cannot create threshold with type \"" + to_string(type) + "\"");
		}

		case EThresholdType::kOtsu:
		{
			threshold_value = cv::threshold(greyscale_and_blurred, dst, 64, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
			break;
		}
		case EThresholdType::kTriangle:
		{
			threshold_value = cv::threshold(greyscale_and_blurred, dst, 64, 255, cv::THRESH_BINARY | cv::THRESH_TRIANGLE);
			break;
		}
		case EThresholdType::kAutoGaussian:
		{
			const int block_size = 5;
			const double constant_to_subtract = 9.0;
			cv::adaptiveThreshold(greyscale_and_blurred, dst, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, block_size, constant_to_subtract);
			break;
		}
		case EThresholdType::kAutoMeanC:
		{
			const int block_size = 5;
			const double constant_to_subtract = 9.0;
			cv::adaptiveThreshold(greyscale_and_blurred, dst, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, block_size, constant_to_subtract);
			break;
		}
		case EThresholdType::kDifferenceOfGaussian:
		{
			dst = vz::apply_DoG_threshold(get(EMatType::kOriginalImage));
			break;
		}
	}

	if (dst.empty())
	{
		/// @throw std::logic_error if the threshold image wasn't created (internal error)
		throw std::logic_error("failed to create threshold image with type \"" + to_string(type) + "\"");
	}

	return dst;
}
