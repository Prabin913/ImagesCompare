/*! @file PrintChecker.cpp
 *  @brief Implementation of the PrintChecker class for comparing images
 *  @author Gabor Szijarto
 */

#include "PrintChecker.hpp"
#include "vzTypes.hpp"
#include <opencv2/dnn.hpp>
#include <filesystem>
#include <iostream>

import printcheck.cv;
import printcheck.align;
import printcheck.difference;

#define LOG_FILE

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

//         cv::resize(_ref, _ref, cv::Size(), 0.6, 0.6);
//         cv::resize(_scanned, _scanned, _ref.size());

#ifdef LOG_FILE
        cv::imwrite("ref_resize.png", _ref); 
        cv::imwrite("scanned_resize.png", _scanned); 
#endif

        auto aligned = printcheck::align_orb(_ref, _scanned);
        _scanned = aligned.Aligned.clone();

#ifdef LOG_FILE
        cv::imwrite("align_scanned.png", _scanned);
#endif

		rows = _scanned.rows;
		cols = _scanned.cols;

#if 1
        /*
		//////////////////////////////////////////////////////////////////////////
		// Get origin offset
		constexpr auto compute = [](const cv::Mat& src1, const cv::Mat& src2)
			{
				cv::Mat diff;
				cv::absdiff(src1, src2, diff); // Compute absolute difference
				return cv::mean(diff.mul(diff))[0]; // Mean of squared differences
			};

		
		double prev_mse = std::numeric_limits<double>::max();
		cv::Mat gray1;
		cv::Mat gray2;
		cvtColor(_ref, gray1, cv::COLOR_BGR2GRAY);
		cvtColor(_scanned, gray2, cv::COLOR_BGR2GRAY);

		int offsetX = 0, offsetY = 0;
		const int neighborhoodSize = 32;
		for (int dy = 0; dy <= neighborhoodSize; ++dy)
		{
			for (int dx = 0; dx <= neighborhoodSize; ++dx)
			{
				int l = dx;
				int t = dy;
				int r = cols;
				int b = rows;

				cv::Rect rect1(0, 0, r - l, b - t);
				cv::Rect rect2(l, t, r - l, b - t);
				double mse1 = compute(gray1(rect1), gray2(rect2));
				double mse2 = compute(gray1(rect2), gray2(rect1));
				double mse = cv::min(mse1, mse2);
				if (mse < prev_mse) {
					prev_mse = mse;

					offsetX = dx; offsetY = dy;
					if (mse1 > mse2) matIdx = 1;
				}
			}
		}

        if (offsetX != 0 || offsetY != 0)
        {
            cv::Mat trans = cv::Mat::zeros(rows, cols, _ref.type());
            if (matIdx) {
                _scanned(cv::Rect(offsetX, offsetY, cols - offsetX, rows - offsetY)).copyTo(trans(cv::Rect(0, 0, cols - offsetX, rows - offsetY)));
                _scanned = trans;
            }
            else {
                _ref(cv::Rect(offsetX, offsetY, cols - offsetX, rows - offsetY)).copyTo(trans(cv::Rect(0, 0, cols - offsetX, rows - offsetY)));
                _ref = trans;
            }
        }
		

		//cv::imwrite("edges_trans.png", edges1);
		//cv::imwrite("scanned_edges_trans.png", edges2);
		//////////////////////////////////////////////////////////////////////////
        */

        const int block_dimens = 16;
		cv::Mat mask = cv::Mat::zeros(rows, cols, CV_8UC1);

		for (int y = 0; y < rows; y+= block_dimens/2)
		{
			for (int x = 0; x < cols; x += block_dimens / 2)
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

                mask.at<uchar>(y, x) = 255 - (uchar)(255.0 * computeSSIM(patch1, patch2));
			}
		}
#ifdef LOG_FILE
        cv::imwrite("mask.png", mask); 
#endif
        _mask = mask;

#else
        cv::Mat mask = applyComparison(_ref, _scanned, 500, 500);
        //cv::imwrite("mask.png", mask);
        

        std::vector<cv::Rect> boxes = findContours(mask, 25);
        *diff = 0.0;
        for (const cv::Rect& bbox : boxes)
        {
            enlargeRect(const_cast<cv::Rect&>(bbox), 5, cols, rows);

            cv::Mat patch1 = _ref(bbox);
            cv::Mat patch2 = _scanned(bbox);
            double sim = computeSSIM(patch1, patch2);
            if (sim < 0.5)
            {
                continue;
            }
            else
            {
                cv::rectangle(_scanned, bbox, cv::Scalar(0, 255, 0), 2, 1);
            }
            (*diff)++;
        }
#endif

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
    cv::Mat PrintChecker::applyLimit(int limit, double* diff)
    {
		// thresholding
        cv::Mat mask = _mask;
		cv::Mat maskBinary;
		cv::threshold(mask, maskBinary, limit, 255, cv::THRESH_BINARY);

#ifdef LOG_FILE
        cv::imwrite("mask_filtered1.png", maskBinary); 
#endif 

		std::vector<cv::Point> whitePixels;
		cv::findNonZero(maskBinary, whitePixels);
		if (!whitePixels.empty()) {
			for (const auto& point : whitePixels)
			{
				uchar val = maskBinary.at<uchar>(point.y, point.x);
				if (val == 0) continue;

				for (int dx = -8; dx <= 8; dx++) {
					for (int dy = -8; dy <= 8; dy++) {
						if (point.x + dx >= 0 && point.x + dx < maskBinary.cols && point.y + dy >= 0 && point.y + dy < maskBinary.rows) {
							maskBinary.at<uchar>(point.y + dy, point.x + dx) = val;
						}
					}
				}
			}
		}
#ifdef LOG_FILE
		cv::imwrite("mask_filterd1_expand.png", maskBinary); 
#endif

		// Find contours
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(maskBinary, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		// Draw contours
		cv::Mat contourImg = cv::Mat::zeros(maskBinary.size(), CV_8UC1);
		for (size_t i = 0; i < contours.size(); i++) {
			drawContours(contourImg, contours, (int)i, 255, 2, cv::LINE_8, hierarchy, 0);
		}

		cv::findNonZero(maskBinary, whitePixels);
        if (diff)
            *diff = whitePixels.size() * 1.0 / (maskBinary.cols * maskBinary.rows) * 100.0;
#ifdef LOG_FILE
		cv::imwrite("mask_filtered1_contour.png", contourImg); 
#endif

        cv::Mat annotated;
        cv::Mat scanned_alpha;
        cv::resize(_scanned, scanned_alpha, contourImg.size());
		
        cv::findNonZero(contourImg, whitePixels);
		for (const auto& point : whitePixels)
		{
			cv::Vec3b& pixel = scanned_alpha.at<cv::Vec3b>(point.y, point.x);
			pixel[0] = 0;   // Blue
			pixel[1] = 0;   // Green
			pixel[2] = 255; // Red
		}
        return scanned_alpha;

//         if (scanned_alpha.size() == contourImg.size() && scanned_alpha.type() == contourImg.type())
//             cv::addWeighted(scanned_alpha, 1.0, contourImg, 1.0, 0.0, annotated);

//        return annotated;
    }
}