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

        cv::resize(_ref, _ref, cv::Size(), 0.6, 0.6);
        cv::resize(_scanned, _scanned, _ref.size());

        auto aligned = printcheck::align_orb(_ref, _scanned);
        _scanned = aligned.Aligned.clone();

        cv::Mat mask = applyComparison(_ref, _scanned, 500, 500);

        int rows = _scanned.rows;
        int cols = _scanned.cols;

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
    cv::Mat PrintChecker::applyLimit(int limit)
    {
        return _scanned;
    }
}