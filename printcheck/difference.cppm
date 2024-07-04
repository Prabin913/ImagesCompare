module;
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
// #include <g3log/g3log.hpp>

#include <iostream>
#define LOG(level) std::clog

export module printcheck.difference;

using cv::Mat, cv::absdiff, cv::cvtColor;

export namespace printcheck
{

	/*!
	 *  @details
	 *    calculates the mask for thresholded error visualization.
	 *    The mask shows the area to be kept for visualization!
	 *  @param  err
	 *    Error image visualized
	 *  @param  limit
	 *    Error values smaller than the limit will be masked out
	 */
	Mat mask_error( const Mat& err, int limit)
	{
		LOG(INFO) << "calculating error mask with limit: " << limit;
		Mat gray;
		cvtColor( err, gray, cv::COLOR_BGR2GRAY);
		return gray >= limit;
	}

	/*!
	 *  @details
	 *    Calculates pixel level absolute difference!
	 *    Keeps the channel information.
	 */
	Mat diff_pixel( const Mat& ref, const Mat& tst)
	{
		Mat res;
		absdiff( ref, tst, res);
		return res;
	}

}
