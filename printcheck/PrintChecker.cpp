/*! @file PrintChecker.cpp @author Gabor Szijarto */
#include "PrintChecker.hpp"
#include "vzTypes.hpp"


import printcheck.cv;
import printcheck.align;
import printcheck.difference;

namespace printcheck
{
/**************************************************************************************************/
	//  Functionality

	/*!
	 *  @details
	 *    Compares the provided reference & scanned back images against each other!
	 *  @param  ref
	 *    reference image to be opened for processing
	 *  @param  scan
	 *    path of the scanned back image to be opened for comparison
	 *  @return
	 *    Returns with the visualization or empty Mat object in case of failure.
	 */
	cv::Mat PrintChecker::process( const std::filesystem::path& ref, const std::filesystem::path& scan, double *diff)
	{
		_ref = printcheck::read( ref);
		_scanned = printcheck::read( scan);

		cv::resize(_ref, _ref, cv::Size(), 0.6, 0.6);
		cv::resize(_scanned, _scanned, _ref.size());

		if (_ref.empty() || _scanned.empty())
		{
			return {};
		}
		/*cv::Mat result = getDifferenceBetweenImageWithSSIM(refer,tst,diff);
		finalWork = result;
		return result;*/


		auto aligned = printcheck::align_orb( _ref, _scanned);
		_scanned = aligned.Aligned.clone();

		//_error = printcheck::diff_pixel( _ref, aligned.Aligned);
		//_error.setTo(0, ~aligned.Mask);

		cv::Mat mask = applyComparison(_ref, _scanned, 500, 500);

		int rows = _scanned.rows;
		int cols = _scanned.cols;
		
		std::vector<cv::Rect> boxes = findContours(mask, 25);
		*diff = 0.0;
		for (cv::Rect bbox : boxes) {

			enlargeRect(bbox, 5, cols, rows);

			cv::Mat patch1 = _ref(bbox);
			cv::Mat patch2 = _scanned(bbox);
			double sim = computeSSIM(patch1, patch2);
			if ( sim < 0.5)
			{
				//cv::rectangle(_scanned, bbox, cv::Scalar(255, 0, 0), 2, 1);
				continue;
			}
			else {
				cv::rectangle(_scanned, bbox, cv::Scalar(0, 255, 0), 2, 1);
			}
			(*diff)++;
		}

		//// Calculate total number of pixels and number of different pixels
		//int totalPixels = _ref.rows * _ref.cols * _ref.channels();
		//cv::Scalar diffSum = cv::sum(_error);
		//double numDifferentPixels = diffSum[0] / 255.0; // Assuming grayscale difference image

		//// Calculate percentage of different pixels
		//diff = (numDifferentPixels / totalPixels) * 100.0;
		std::cout << "PrintChecker::process is OK" << std::endl;
		return _scanned;
	}

	/*!
	 *  @pre
	 *    \ref process must have been called before calling this one..
	 *  @post
	 *    \ref _errormap is updated
	 *  @param  limit
	 *    New limit value to be applied!
	 *    Must be in the range [0-255]
	 *  @return
	 *    Returns with the visualization with the applied error threshold
	 */
	cv::Mat PrintChecker::applyLimit( int limit)
	{
		/*if (_ref.empty() || _error.empty()) { return {}; }

		auto mask = printcheck::mask_error( _error, limit);
		auto [blended, map] = printcheck::blend_error( _ref, _error, mask);
		_errormap = map;
		return blended;*/
		return _scanned;


	}

}
