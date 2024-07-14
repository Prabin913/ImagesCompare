/*! @file PrintChecker.cpp
 *  @brief Implementation of the PrintChecker class for comparing images
 *  @author Gabor Szijarto
 */

#include "PrintChecker.hpp"
#include "vzTypes.hpp"
#include <opencv2/dnn.hpp>
#include <filesystem>
#include <iostream>
#include "..\constants.h"
import printcheck.cv;
import printcheck.align;
import printcheck.difference;

namespace printcheck
{
    /*!
     *  @details
     *    Compares the provided reference and scanned back images against each other.
     *  @param  ref
     *    Reference image path for processing
     *  @param  scan
     *    Scanned back image path for comparison
     *  @param  diff
     *    Pointer to store the difference metric
     *  @return
     *    Returns a visualization of the differences or an empty Mat object in case of failure.
     */
    cv::Mat PrintChecker::process(const std::filesystem::path& ref, const std::filesystem::path& scan, double* diff)
    {
        _ref = printcheck::read(ref);
        _scanned = printcheck::read(scan);

        if (_ref.empty() || _scanned.empty())
        {
            return {};
        }

        char matIdx = 0;
        if (_ref.cols * _ref.rows < _scanned.cols * _scanned.rows) {
            cv::resize(_scanned, _scanned, _ref.size());
        }
        else {
            cv::resize(_ref, _ref, _scanned.size());
        }
		int rows = _ref.rows ;
		int cols = _ref.cols;

        dbg_dump("0_ref_resize.png", _ref);
        dbg_dump("0_scanned_resize.png", _scanned);

        auto aligned = printcheck::align_orb(_ref, _scanned);
        _scanned = aligned.Aligned.clone();
        dbg_dump("1_align_scanned.png", _scanned);

		rows = _scanned.rows;
		cols = _scanned.cols;

        const int block_dimens = 16;
        _mask = cv::Mat::zeros(rows/block_dimens + 1, cols/block_dimens + 1, CV_8UC1);

		for (int y = 0; y < rows; y+= block_dimens)
		{
			for (int x = 0; x < cols; x += block_dimens)
			{
                int l = cv::max(x - block_dimens, 0);
                int t = cv::max(y - block_dimens, 0);
                int r = cv::min(x + block_dimens, cols);
                int b = cv::min(y + block_dimens, rows);
				//cv::Rect rect(cv::Point(t, l), cv::Point(b, r));
                cv::Rect rect(l, t, r - l, b - t);

				cv::Mat patch1;
				cv::Mat patch2;
                cvtColor(_ref(rect), patch1, cv::COLOR_BGR2GRAY);
                cvtColor(_scanned(rect), patch2, cv::COLOR_BGR2GRAY);

                _mask.at<uchar>(y / block_dimens, x / block_dimens) = 255 - (uchar)(255.0 * computeSSIM(patch1, patch2));
			}
		}
        dbg_dump("2_mask.png", _mask);

        std::cout << "PrintChecker::process is OK" << std::endl;
        return _scanned;
    }

    /*!
     *  @pre
     *    \ref process must have been called before calling this one.
     *  @post
     *    \ref _errormap is updated.
     *  @param  limit
     *    New limit value to be applied. Must be in the range [0-255].
     *  @return
     *    Returns a visualization with the applied error threshold.
     */
    cv::Mat PrintChecker::applyLimit(int limit, int color, double* diff)
    {
		// thresholding
        cv::Mat maskBinary;
		cv::threshold(_mask, maskBinary, limit, 255, cv::THRESH_BINARY);
        dbg_dump("3_mask_filtered.png", maskBinary);
		
        std::vector<cv::Point> whitePixels;
		cv::findNonZero(maskBinary, whitePixels);
		if (diff)
			*diff = whitePixels.size() * 1.0 / (maskBinary.cols * maskBinary.rows) * 100.0;

		cv::resize(maskBinary, maskBinary, _scanned.size());
		dbg_dump("3_mask_filtered_expand.png", maskBinary);
        		
		// Find contours
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(maskBinary, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		// Draw contours
		cv::Mat contourImg = cv::Mat::zeros(maskBinary.size(), CV_8UC1);
		for (size_t i = 0; i < contours.size(); i++) {
			drawContours(contourImg, contours, (int)i, 255, 2, cv::LINE_8, hierarchy, 0);
		}

        
        dbg_dump("3_mask_filtered_contour.png", contourImg);        

        cv::Mat annotated = _scanned.clone();
        
        cv::findNonZero(contourImg, whitePixels);
		for (const auto& point : whitePixels)
		{
			cv::Vec3b& pixel = annotated.at<cv::Vec3b>(point.y, point.x);
            switch (color)
            {
                case COLOR_PURPLE:
                    pixel[0] = 255;   // Blue
                    pixel[1] = 0;   // Green
                    pixel[2] = 255; // Red
                    break;
                case COLOR_BLUE:
                    pixel[0] = 255;   // Blue
                    pixel[1] = 0;   // Green
                    pixel[2] = 0; // Red
                    break;
                case COLOR_RED:
                    pixel[0] = 0;   // Blue
                    pixel[1] = 0;   // Green
                    pixel[2] = 255; // Red
                    break;
            }
		}
        return annotated;
    }
}