/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: vzTypes.hpp 2969 2020-05-04 16:23:18Z stephane $
 */

#pragma once

#include <map>
#include <set>
#include <list>
#include <vector>
#include <string>
#include <cmath>
#include <opencv2/opencv.hpp>

/** @file
 * vzTypes contains several simple commonly-used types in the vz namespace and vz::Imagination library.
 * They should be obvious, and are defined here simply for convenience.
 */


namespace vz
{
	/// Vector of OpenCV @p cv::Mat.
	typedef std::vector<cv::Mat> VMats;

	/// Vector of text strings.
	typedef std::vector<std::string> VStr;

	/// A single contour is a vector of many points.  @see @ref vz::find_contours()
	typedef std::vector<cv::Point> Contour;

	/// Many contours are then combined to create a vector of contours.  @see @ref vz::find_contours()
	typedef std::vector<Contour> VContours;

	/// Vector of rotated rectangles.  @see @ref vz::convert_contours_to_rotated_rects()
	typedef std::vector<cv::RotatedRect> VRotatedRects;

	/// Vector of Vec4i  @see @ref vz::find_hough_lines()
	typedef std::vector<cv::Vec4i> VV4i;

	/// Vector of int.
	typedef std::vector<int> VInt;

	/// Set of doubles.
	typedef std::set<double> SDouble;

	/// Vector of doubles.  @see @ref vz::k_means()
	typedef std::vector<double> VDouble;

	/// Vector of @ref vz::VDouble.  @see @ref vz::k_means_vector()
	typedef std::vector<VDouble> VVDouble;

	/// Vector of size_t.  @see @ref vz::k_means()
	typedef std::vector<size_t> VSizet;

	/// Map of strings where both the key and the value are strings.
	typedef std::map<std::string, std::string> MStr;
}
