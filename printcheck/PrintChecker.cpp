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
#include "..\utils.h"
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
	void PrintChecker::process(const std::filesystem::path& ref, const std::filesystem::path& scan)
	{
		cv::Mat ref_img, scan_img;

		// load
		_ref = printcheck::read(ref);
		_scanned = printcheck::read(scan);

		if (_ref.empty() || _scanned.empty()) {
			return;
		}

		// resize
		if (_ref.cols * _ref.rows < _scanned.cols * _scanned.rows) {
			cv::resize(_scanned, _scanned, _ref.size());
		}
		else {
			cv::resize(_ref, _ref, _scanned.size());
		}

		cv::Mat tmpImg;
		const int BLOCKS_DIVIDER = 2;
		const int BLOCKS_GAP = 20;
		double ssim_score = 0;
		_mask = cv::Mat::zeros(_scanned.size(), CV_8UC1);
		for (int i = 0; i < BLOCKS_DIVIDER; i++) {
			for (int j = 0; j < BLOCKS_DIVIDER; j++) {
				int l = cvFloor(i * _scanned.size().width / 2.0);
				int t = cvFloor(j * _scanned.size().height / 2.0);
				int r = cvFloor((i + 1) * _scanned.size().width / 2.0);
				int b = cvFloor((j + 1) * _scanned.size().height / 2.0);

				cv::Rect subRect(l, t, r - l, b - t);

				cv::Mat subScanned = _scanned(subRect);

				// apply filter
				removeShadowNormalize(_ref(subRect).clone(), ref_img);
				removeShadowNormalize(_scanned(subRect).clone(), scan_img);
				dbg_dump("0_ref_rfilter.png", ref_img);
				dbg_dump("0_scanned_rfilter.png", scan_img);

				// aligning
				if (ref_img.channels() != 1)
					cvtColor(ref_img, ref_img, cv::COLOR_BGR2GRAY);
				if (scan_img.channels() != 1)
					cvtColor(scan_img, scan_img, cv::COLOR_BGR2GRAY);
				
				auto aligned = align_surf(scan_img, ref_img);
				cv::warpPerspective(scan_img, scan_img, aligned.Homography, ref_img.size(), 1, cv::BORDER_CONSTANT, 255);
				cv::warpPerspective(subScanned, subScanned, aligned.Homography, ref_img.size(), 1, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
				dbg_dump("1_align_scan.png", scan_img);
				dbg_dump("1_align_match.png", aligned.Matched);

				// compute global value
				auto ssim = computeSSIM(ref_img, scan_img);
				cv::Mat mask = cv::Mat::zeros(ssim.diff.size(), CV_8UC1);
				for (int y = BLOCKS_GAP; y < ssim.diff.rows - BLOCKS_GAP; y++) {
					for (int x = BLOCKS_GAP; x < ssim.diff.cols - BLOCKS_GAP; x++) {
						mask.at<uchar>(y, x) = 255 - (uchar)(ssim.diff.at<float>(y, x) * 255);
					}
				}

				dbg_dump("2_mask_sub.png", mask);
				subScanned.copyTo(_scanned(subRect));
				mask.copyTo(_mask(subRect));
				ssim_score += ssim.score;
			}
		}

		dbg_dump("2_mask.png", _mask);

		cv::threshold(_mask, tmpImg, (int)(ssim_score / BLOCKS_DIVIDER / BLOCKS_DIVIDER * 255), 255, cv::THRESH_BINARY);
		cv::resize(tmpImg, _mask, _scanned.size());
		dbg_dump("3_mask_filtered.png", _mask);

		WriteLogFile(L"PrintChecker::process is OK");
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
		try
		{
			// Thresholding
			cv::Mat maskBinary = _mask;
			
			std::vector<cv::Point> whitePixels;
			cv::findNonZero(maskBinary, whitePixels);
			int diff_size = whitePixels.size();

			// Find contours
			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::findContours(maskBinary, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

			// Draw contours
			cv::Mat contourImg = cv::Mat::zeros(maskBinary.size(), CV_8UC1);
			for (size_t i = 0; i < contours.size(); i++)
			{
				if (cv::contourArea(contours[i]) < (int)(limit / 3)) {
					diff_size -= contours[i].size();
					continue;
				}
				drawContours(contourImg, contours, (int)i, 255, 2, cv::LINE_8, hierarchy, 0);
			}
			if (diff)
				*diff = diff_size * 1.0 / (maskBinary.cols * maskBinary.rows) * 100.0;

			dbg_dump("3_mask_filtered_contour.png", contourImg);

			cv::Mat annotated = _scanned.clone();

			// Apply color to annotated image based on specified color
			cv::findNonZero(contourImg, whitePixels);
			for (const auto& point : whitePixels)
			{
				cv::Vec3b& pixel = annotated.at<cv::Vec3b>(point.y, point.x);
				switch (color)
				{
				case COLOR_PURPLE:
					pixel[0] = 255;   // Blue
					pixel[1] = 0;     // Green
					pixel[2] = 255;   // Red
					break;
				case COLOR_BLUE:
					pixel[0] = 255;   // Blue
					pixel[1] = 0;     // Green
					pixel[2] = 0;     // Red
					break;
				case COLOR_RED:
					pixel[0] = 0;     // Blue
					pixel[1] = 0;     // Green
					pixel[2] = 255;   // Red
					break;
				}
			}

			return annotated;
		}
		catch (const cv::Exception& ex)
		{
			//dbg_dump(L"OpenCV exception in applyLimit: %S", ex.what());
			// Handle the exception as per your application's requirements
			// For example, return an empty Mat or rethrow the exception
			//throw; // Rethrow the exception to propagate it further
		}
		catch (const std::exception& ex)
		{
			//dbg_dump(L"Standard exception in applyLimit: %S", ex.what());
			// Handle the exception as per your application's requirements
			// For example, return an empty Mat or rethrow the exception
			//throw; // Rethrow the exception to propagate it further
		}
		catch (...)
		{
			//dbg_dump(L"Unknown exception occurred in applyLimit.");
			// Handle the exception as per your application's requirements
			// For example, return an empty Mat or rethrow the exception
			//throw; // Rethrow the exception to propagate it further
		}
	}
}