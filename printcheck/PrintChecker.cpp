/*! @file PrintChecker.cpp @author Gabor Szijarto */
#include "PrintChecker.hpp"

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
	cv::Mat PrintChecker::process( const std::filesystem::path& ref, const std::filesystem::path& scan)
	{
		_ref = printcheck::read( ref);
		auto tst = printcheck::read( scan);
		if (_ref.empty() || tst.empty())
		{
			return {};
		}
		auto aligned = printcheck::align_orb( _ref, tst);
		_error = printcheck::diff_pixel( _ref, aligned.Aligned);
		_error.setTo(0, ~aligned.Mask);
		return (cv::Mat)_ref;
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
		if (_ref.empty() || _error.empty()) { return {}; }

		auto mask = printcheck::mask_error( _error, limit);
		auto [blended, map] = printcheck::blend_error( _ref, _error, mask);
		_errormap = map;
		return blended;
	}

}
