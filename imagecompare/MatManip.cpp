/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: MatManip.cpp 2969 2020-05-04 16:23:18Z stephane $
 */

#include "MatManip.hpp"
#include "Intervals.hpp"


bool vz::compare_exact(const cv::Mat & lhs, const cv::Mat & rhs)
{
	if (lhs.empty() && rhs.empty())
	{
		// compare two empty objects, so obviously they're equal
		return true;
	}

	if (lhs.empty() != rhs.empty())
	{
		// compare an empty and non-empty image, so obviously they're different
		return false;
	}

	if (lhs.size() != rhs.size())
	{
		// images are not the same size
		return false;
	}

	if (lhs.type() != rhs.type())
	{
		// types (int vs float for example) are not the same
		return false;
	}

	// if we get here, we have a more complex scenario -- both of the images must be compared pixel-by-pixel

	cv::Mat output;
	cv::bitwise_xor(lhs, rhs, output);

	// the output might have multiple channels, so before we can call cv::countNonZero() we need to simplify it to a single channel
	if (output.channels() > 1)
	{
		cv::cvtColor(output, output, cv::COLOR_BGR2GRAY);
	}

	if (cv::countNonZero(output) > 0)
	{
		// there are differences in the images if we get here
		return false;
	}

	// if we get here then there are zero differences between the two images

	return true;
}


cv::Mat vz::to_greyscale(const cv::Mat & src)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot convert an empty image to greyscale");
	}

	if (src.channels() == 1)
	{
		cv::Mat dst = src.clone();
		return dst;
	}

	if (src.channels() == 3)
	{
		cv::Mat dst;
		cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
		return dst;
	}

	/// @throw std::invalid_argument if the image has an unsupported number of channels
	throw std::invalid_argument("cannot convert an image with " + std::to_string(src.channels()) + " channels");
}


cv::Mat vz::apply_gaussian_blur(const cv::Mat & src, const int kernel_size)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot blur an empty image");
	}

	if (kernel_size < 1 || (kernel_size % 2) == 0)
	{
		/// @throw std::invalid_argument if the kernel size is < 1 or an even number
		throw std::invalid_argument("invalid kernel size of " + std::to_string(kernel_size));

	}

	cv::Size k(kernel_size, kernel_size);

	cv::Mat dst;
	cv::GaussianBlur(src, dst, k, 0.0, 0.0);

	return dst;
}


cv::Mat vz::apply_blur_and_convert_to_greyscale(const cv::Mat & src, const int kernel_size)
{
	return to_greyscale(apply_gaussian_blur(src, kernel_size));
}


cv::Mat vz::apply_simple_colour_balance(const cv::Mat & src)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot apply colour balance to an empty image");
	}

	const float full_percent = 1.0;
	const float half_percent = full_percent / 200.0f;

	vz::VMats tmpsplit;
	cv::split(src, tmpsplit);

	for (size_t idx = 0; idx < tmpsplit.size(); idx++)
	{
		// find the low and high percentile values (based on the input percentile)
		cv::Mat flat;
		tmpsplit[idx].reshape(1,1).copyTo(flat);
		cv::sort(flat, flat, cv::SORT_EVERY_ROW + cv::SORT_ASCENDING);
		const int lo = flat.at<uchar>(cvFloor(((float)flat.cols) * (0.0 + half_percent)));
		const int hi = flat.at<uchar>( cvCeil(((float)flat.cols) * (1.0 - half_percent)));

		// saturate below the low percentile and above the high percentile
		tmpsplit[idx].setTo(lo, tmpsplit[idx] < lo);
		tmpsplit[idx].setTo(hi, tmpsplit[idx] > hi);

		// scale the channel
		cv::normalize(tmpsplit[idx], tmpsplit[idx], 0, 255, cv::NORM_MINMAX);
	}

	cv::Mat output;
	cv::merge(tmpsplit, output);

	return output;
}


cv::Mat vz::erode(const cv::Mat & src, const int iterations)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot apply erosion to an empty image");
	}

	cv::Mat dst;
	cv::erode(src, dst, cv::Mat(), cv::Point(-1, -1), iterations);

	return dst;
}


cv::Mat vz::dilate(const cv::Mat & src, const int iterations)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot apply dilation to an empty image");
	}

	cv::Mat dst;
	cv::dilate(src, dst, cv::Mat(), cv::Point(-1, -1), iterations);

	return dst;
}


cv::Mat vz::invert(const cv::Mat & src)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot invert an empty image");
	}

	cv::Mat dst;
	cv::bitwise_not(src, dst);

	return dst;
}


cv::Mat vz::apply_canny_edge_detection(const cv::Mat & src, const double canny_threshold)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot apply canny edge detection to an empty image");
	}

	cv::Mat dst;
	cv::Canny(src, dst, canny_threshold, 3.0 * canny_threshold, 3, true);

	return dst;
}


vz::VContours vz::find_contours(const cv::Mat & src)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot find contours in an empty image");
	}

	vz::VContours contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	return contours;
}


cv::Mat vz::add_contours_to_image(const cv::Mat & src, const VContours & contours, const cv::Scalar & colour)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot add contours to an empty image");
	}

	cv::Mat dst;
	if (src.channels() == 1)
	{
		cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);
	}
	else
	{
		dst = src.clone();
	}

	// first, highlight all of the contours
	for (auto & contour : contours)
	{
		cv::polylines(dst, contour, true, colour, 1, cv::LINE_AA);
	}

	// next, look for the significant contours and number them
	for (size_t idx = 0; idx < contours.size(); idx++)
	{
		const Contour & c = contours.at(idx);
		const double area = cv::contourArea(c);
		if (area <= 105.0)
		{
			// skip areas which are too small
			continue;
		}

		if (area > 100.0)
		{
//std::cout << "IDX=" << idx << ", AREA=" << area << std::endl;
//cv::polylines(dst, c, true, cv::Scalar { 0, 0, 255 }, 5, cv::LINE_AA);

			const cv::RotatedRect rr = cv::minAreaRect(c);

			const cv::Point center = cv::Point(rr.center);
			const cv::Point offset = cv::Point(10, -5);
			cv::putText(dst, std::to_string(idx), center - offset, cv::FONT_HERSHEY_SIMPLEX, 2.5, colour);

#if 0
			// draw a box around this contour
			cv::Point2f rect_points[4];
			rr.points(rect_points);
			for (int i = 0; i < 4; i++)
			{
				cv::line(dst, rect_points[i], rect_points[(i+1)%4], cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
			}

			// put a cicle in the very middle of this contour
			cv::circle(dst, center, 5, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);

			// finding the "moment" is the same as finding the center above
			cv::Moments moments = cv::moments(c);
			cv::Point2f m(moments.m10/moments.m00, moments.m01/moments.m00);
			cv::circle(dst, center, 5, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
#endif
		}
	}

	return dst;
}


cv::Mat vz::add_contours_and_limits_to_image(const cv::Mat & src, const vz::VContours & contours)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot add contours to an empty image");
	}

	const std::vector<cv::Scalar> colours
	{
		cv::Scalar(0, 0, 255),
		cv::Scalar(0, 255, 0),
		cv::Scalar(255, 0, 0),
		cv::Scalar(255, 255, 0),
		cv::Scalar(0, 255, 255),
		cv::Scalar(255, 0, 255),
		cv::Scalar(255, 255, 255)
	};

	// find all the contour areas
	vz::VDouble data;
	for (const auto & c : contours)
	{
		const double area = cv::contourArea(c);
		data.push_back(area);
	}
	vz::VVDouble vv = vz::intervals(data);

	// now go through all the contours again, but to draw them on the mat
	cv::Mat dst = src.clone();
//	for (const auto & c : contours)
	for (size_t idx = 0; idx < contours.size(); idx++)
	{
		const Contour & c = contours.at(idx);
		const double area = cv::contourArea(c);

		if (area <= 5.0)
		{
			// skip areas which are too small
			continue;
		}

		// find the right colour to use by finding the interval to which we belong
		auto colour = cv::Scalar(255, 255, 255);
		for (int idx = vv.size() - 1; idx >= 0; idx --)
		{
			if (area > vv[idx][0])
			{
				colour = colours.at(idx % colours.size());
				break;
			}
		}

		cv::polylines(dst, c, true, colour, 1, cv::LINE_AA);

		const cv::RotatedRect rr = cv::minAreaRect(c);
		const cv::Point center = cv::Point(rr.center);
		const cv::Point offset = cv::Point(10, -5);
		cv::putText(dst, std::to_string(idx), center - offset, cv::FONT_HERSHEY_SIMPLEX, 0.5, colour);
	}

	return dst;
}


cv::Mat vz::plot_histogram(const cv::Mat & src, const int width, const int height, const cv::Scalar background_colour)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot plot histogram for an empty image");
	}

	const size_t number_of_channels = src.channels();
	if (number_of_channels != 1 && number_of_channels != 3)
	{
		/// @throw std::invalid_argument if the image is neither greyscale nor 3-channel RGB
		throw std::invalid_argument("cannot plot histogram for " + std::to_string(number_of_channels) + "-channel image");
	}

	if (width < 10 || height < 10)
	{
		/// @throw std::invalid_argument if the output dimensions are out of range
		throw std::invalid_argument("cannot create histogram with invalid dimensions of " + std::to_string(width) + "x" + std::to_string(height));
	}

	vz::VMats split;
	cv::split(src, split);

	const int		histogram_size	= 256; // the number of "bins"
	const float		range[]			= {0, 256}; // upper bound is exclusive, meaning 0-255
	const float *	ranges			= {range};
	const bool		uniform			= true;
	const bool		accumulate		= false;
	cv::Mat			mask;

	// prepare the destination image
	const int margin		= 3;
	const int min_y			= margin;
	const int max_y			= height - margin;
	const int thickness		= 1;
	const int line_type		= cv::LINE_AA;
	const float bin_width	= static_cast<float>(width) / static_cast<float>(histogram_size);
	cv::Mat dst(height, width, CV_8UC3, background_colour); // create the output image, starting with a pure colour

	cv::Scalar colours[] =
	{
		{255, 0, 0},	// blue
		{0, 255, 0},	// green
		{0, 0, 255}		// red
	};
	if (number_of_channels == 1)
	{
		// for greyscale and black + white images, we only have a single colour channel, so
		// ignore the RGB colour definitions and use either black or white for the histogram

		colours[0] = (background_colour == cv::Scalar(0, 0, 0)) ? cv::Scalar(255, 255, 255) : cv::Scalar(0, 0, 0);
	}

	// iterate through all the channels in this image
	for (size_t idx=0; idx < split.size(); idx++)
	{
		const cv::Scalar colour = colours[idx % 3];

		cv::Mat & m = split[idx];

		cv::Mat histogram;
		cv::calcHist(&m, 1, 0, mask, histogram, 1, &histogram_size, &ranges, uniform, accumulate);

		cv::normalize(histogram, histogram, 0, dst.rows, cv::NORM_MINMAX);

		for (int i = 1; i < histogram_size; i++)
		{
			const int x1 = std::round(bin_width * (i - 1));
			const int x2 = std::round(bin_width * (i - 0));

#if __cplusplus > 201700
			const int y1 = std::clamp(height - static_cast<int>(std::round(histogram.at<float>(i - 1))), min_y, max_y);
			const int y2 = std::clamp(height - static_cast<int>(std::round(histogram.at<float>(i - 0))), min_y, max_y);
#else
			int y1 = height - static_cast<int>(std::round(histogram.at<float>(i - 1)));
			int y2 = height - static_cast<int>(std::round(histogram.at<float>(i - 0)));
			if (y1 < min_y) { y1 = min_y; }
			if (y1 > max_y) { y1 = max_y; }
			if (y2 < min_y) { y2 = min_y; }
			if (y2 > max_y) { y2 = max_y; }
#endif

			cv::line(dst, cv::Point(x1, y1), cv::Point(x2, y2), colour, thickness, line_type);
		}
	}

	return dst;
}


cv::Mat vz::find_hough_lines(const cv::Mat & src, vz::VV4i & lines)
{
	lines.clear();

	cv::HoughLinesP(
		src,
		lines,
		1,			// distance resolution of the accumulator in pixels
		CV_PI/360,	// angle resolution of the accumulator in radians
		80,			// accumulator threshold parameter
		5,			// minimum line length; line segments shorter than that are rejected
		10);		// maximum allowed gap between points on the same line to link them

	// draw the lines onto the image
	cv::Mat dst;
	cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);

	const int thickness = 1;
	const int line_type = cv::LINE_AA;
	for(size_t i = 0; i < lines.size(); i++)
	{
		cv::line(dst,
				 cv::Point(lines[i][0], lines[i][1]),
				 cv::Point(lines[i][2], lines[i][3]),
				 cv::Scalar(0,0,255),
				 thickness, line_type);
	}

	return dst;
}


cv::Mat vz::rotate(const cv::Mat & src, double degrees)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot rotate an empty image");
	}

	// normalize the angle to 0.0-360.0
	while (degrees < 0.0)
	{
		degrees += 360.0;
	}
	degrees = std::fmod(degrees, 360.0);

	// deal with a few simple cases first

	if (degrees == 0.0 || degrees == 360.0)
	{
		return src.clone();
	}
	if (degrees == 90.0) // or -270.0
	{
		cv::Mat dst;
		cv::rotate(src, dst, cv::ROTATE_90_COUNTERCLOCKWISE);
		return dst;
	}
	if (degrees == 180.0) // or -180.0
	{
		cv::Mat dst;
		cv::rotate(src, dst, cv::ROTATE_180);
		return dst;
	}
	if (degrees == 270.0) // or -90.0
	{
		cv::Mat dst;
		cv::rotate(src, dst, cv::ROTATE_90_CLOCKWISE);
		return dst;
	}

	// if we get here, then the rotation is more complex than 0/90/180/270 degrees

	const float x = src.cols / 2.0f;
	const float y = src.rows / 2.0f;
	const cv::Point2f center(x, y);

	cv::Mat rotation = cv::getRotationMatrix2D(center, degrees, 1.0);

	// resize the mat so we don't cut any corners when rotating
	// https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c
	// https://github.com/milq/cvrotate2D/blob/master/cvrotate2D.cpp

	cv::Rect2f bbox = cv::RotatedRect(center, src.size(), degrees).boundingRect2f();
	rotation.at<double>(0,2) += bbox.width  / 2.0 - center.x;
	rotation.at<double>(1,2) += bbox.height / 2.0 - center.y;

	cv::Mat dst;
	cv::warpAffine(src, dst, rotation, bbox.size());

	return dst;
}


cv::Mat vz::resize(const cv::Mat & src, int width, int height)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot resize an empty image");
	}

	if (width < 0 || height < 0)
	{
		/// @throw std::invalid_argument if either the width or the height is negative
		throw std::invalid_argument("width or height cannot be less than zero");
	}

	if (width == 0 && height == 0)
	{
		/// @throw std::invalid_argument if both width and hight are invalid
		throw std::invalid_argument("width and height cannot both be invalid");
	}

	const cv::Size old_size = src.size();

	if (width == 0)
	{
		// we have the new height, but need to calculate the new width while keeping the aspect ratio the same
		//		new_width / new_height = old_width / old_height
		// multiply by new_height on both sides:
		//		new_width = new_height * old_width / old_height
		width = std::round( double(height) * double(old_size.width) / double(old_size.height) );
	}

	if (height == 0)
	{
		// we have the new width, but need to calculate the new height while keeping the aspect ratio the same
		//		new_height / new_width = old_height / old_width
		// multiply by new_width on both sides:
		//		new_height = new_width * old_height / old_width
		height = std::round( double(width) * double(old_size.height) / double(old_size.width) );
	}

	cv::Mat dst;

	if (width < 1 || height < 1)
	{
		// image dimensions are too small to return a valid image, so return an empty image
		return dst;
	}

	const cv::Size new_size = {width, height};

	enum cv::InterpolationFlags method = cv::INTER_AREA;
	if (new_size.width > old_size.width)
	{
		method = cv::INTER_LINEAR;
	}

	cv::resize(src, dst, new_size, 0, 0, method);

	return dst;
}


cv::Mat vz::resize(const cv::Mat & src, const double scale)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot resize an empty image");
	}

	if (scale < 0.0)
	{
		/// @throw std::invalid_argument if the scale is less than zero
		throw std::invalid_argument("scale cannot be less than zero");
	}

	const cv::Size old_size	= src.size();
	const int new_width		= std::round(scale * (double)old_size.width);
	const int new_height	= std::round(scale * (double)old_size.height);

	cv::Mat dst;

	if (new_width < 1 || new_height < 1)
	{
		// image dimensions are too small to return a valid image
		return dst;
	}

	return resize(src, new_width, new_height);
}


cv::Mat vz::letterbox(const cv::Mat & src, int width, int height, const cv::Scalar & colour)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot letterbox an empty image");
	}

	if (width < 1 || height < 1)
	{
		/// @throw std::invalid_argument if either the width or the height is less than 1
		throw std::invalid_argument("width or height cannot be less than one");
	}

	// find out if we should scale horizontally or vertically
	const double scale1	= double(width)		/ double(src.cols);
	const double scale2	= double(height)	/ double(src.rows);
	const double scale	= std::min(scale1, scale2);

	cv::Mat dst = resize(src, scale);

	if (dst.cols == width && dst.rows == height)
	{
		// image is already the perfect size, so no need to apply letterboxing
		return dst;
	}

	// otherwise, if we get here, then we need to apply some left-right or top-bottom letterboxing

	cv::Mat letterbox(height, width, dst.type(), colour);

	const int x = (width  - dst.cols) / 2;
	const int y = (height - dst.rows) / 2;
	cv::Rect roi(cv::Point(x, y), dst.size());
	dst.copyTo(letterbox(roi));

	return letterbox;
}


cv::Mat vz::apply_DoG(const cv::Mat & src, const double k, const double sigma, const double gamma)
{
	if (src.empty())
	{
		/// @throw std::invalid_argument if the image is empty
		throw std::invalid_argument("cannot apply DoG to an empty image");
	}

	cv::Mat greyscale_image = to_greyscale(src);

	const double sigma1 = sigma;
	const double sigma2 = sigma * k;

	cv::Mat img1;
	cv::GaussianBlur(greyscale_image, img1, cv::Size(0, 0), sigma1, sigma1);

	cv::Mat img2;
	cv::GaussianBlur(greyscale_image, img2, cv::Size(0, 0), sigma2, sigma2);

	cv::Mat result = img1 - (gamma * img2);

	return result;
}


cv::Mat vz::apply_DoG_threshold(const cv::Mat & src, const double k, const double sigma, const double gamma)
{
	cv::Mat result = apply_DoG(src, k, sigma, gamma);

	// remember the output of DoG is single-channel greyscale image,
	// which we'll convert to a single channel black-and-white image

	// calculate some nonsense magical number based on the mean so we know what threshold to apply
	const int mean = std::round(cv::mean(result)[0] / 10.0);

	// in-place conversion of DoG to binary threshold image
	for (int r = 0; r < result.rows; r ++)
	{
		auto * row_ptr = result.ptr(r);

		for (int c = 0; c < result.cols; c ++)
		{
			auto & pixel = row_ptr[c];

			if (pixel > mean)
			{
				pixel = 255;
			}
			else
			{
				pixel = 0;
			}
		}
	}

	return result;
}


vz::VRotatedRects vz::convert_contours_to_rotated_rects(const vz::VContours & contours)
{
	VRotatedRects vrr;

	for (const auto & contour : contours)
	{
		const cv::RotatedRect rr = cv::minAreaRect(contour);
		vrr.push_back(rr);
	}

	return vrr;
}
