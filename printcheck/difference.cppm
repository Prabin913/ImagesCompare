module;
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
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

    cv::Mat compareEdgeImages(const cv::Mat& reference, const cv::Mat& scanned, int neighborhoodSize)
    {
        // Convert images to binary masks
        cv::Mat refBinary, scannedBinary;
        cv::threshold(reference, refBinary, 50, 255, cv::THRESH_BINARY);
        cv::threshold(scanned, scannedBinary, 50, 255, cv::THRESH_BINARY);
        // Create output image
        cv::Mat output = scannedBinary.clone();

        // Find all white pixels in the scanned image
        std::vector<cv::Point> whitePixels;
        cv::findNonZero(scannedBinary, whitePixels);

        int halfSize = neighborhoodSize / 2;

        // Process each white pixel
        for (const auto& point : whitePixels)
        {
            bool foundWhite = false;

            // Check neighborhood pixel by pixel
            for (int dy = -halfSize; dy <= halfSize && !foundWhite; ++dy)
            {
                for (int dx = -halfSize; dx <= halfSize && !foundWhite; ++dx)
                {
                    int nx = point.x + dx;
                    int ny = point.y + dy;

                    // Check if the neighboring pixel is within bounds
                    if (nx >= 0 && nx < refBinary.cols && ny >= 0 && ny < refBinary.rows)
                    {
                        if (refBinary.at<uchar>(ny, nx) == 255)
                        {
                            foundWhite = true;
                        }
                    }
                }
            }

            if (foundWhite)
            {
                output.at<uchar>(point.y, point.x) = 0;
            }
        }

        return output;
    }

    std::vector<cv::Rect> findContours(Mat mask, float minArea)
    {
        int rows = mask.rows;
        int cols = mask.cols;

        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        std::vector<cv::Rect> boxes;
        for (size_t i = 0; i < contours.size(); i++) {
            double area = contourArea(contours[i]);
            if (area <= minArea) {
                continue;
            }

            cv::Rect rect = boundingRect(contours[i]);
            boxes.push_back(rect);
        }
        return boxes;
    }

 
    double computeSSIM(const cv::Mat& img1, const cv::Mat& img2) {
        const double C1 = 6.5025, C2 = 58.5225;

        cv::Mat I1, I2;
        img1.convertTo(I1, CV_32F);
        img2.convertTo(I2, CV_32F);

        cv::Mat I1_2 = I1.mul(I1);
        cv::Mat I2_2 = I2.mul(I2);
        cv::Mat I1_I2 = I1.mul(I2);

        cv::Mat mu1, mu2;
        cv::GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
        cv::GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);

        cv::Mat mu1_2 = mu1.mul(mu1);
        cv::Mat mu2_2 = mu2.mul(mu2);
        cv::Mat mu1_mu2 = mu1.mul(mu2);

        cv::Mat sigma1_2, sigma2_2, sigma12;
        cv::GaussianBlur(I1_2, sigma1_2, cv::Size(11, 11), 1.5);
        sigma1_2 -= mu1_2;
        cv::GaussianBlur(I2_2, sigma2_2, cv::Size(11, 11), 1.5);
        sigma2_2 -= mu2_2;
        cv::GaussianBlur(I1_I2, sigma12, cv::Size(11, 11), 1.5);
        sigma12 -= mu1_mu2;

        cv::Mat t1 = 2 * mu1_mu2 + C1;
        cv::Mat t2 = 2 * sigma12 + C2;
        cv::Mat t3 = t1.mul(t2);

        t1 = mu1_2 + mu2_2 + C1;
        t2 = sigma1_2 + sigma2_2 + C2;
        t1 = t1.mul(t2);

        cv::Mat ssim_map;
        cv::divide(t3, t1, ssim_map);
        return cv::mean(ssim_map)[0];
    }

    bool enlargeRect(cv::Rect& rect, int a, int imgWidth, int imgHeight) {
        // Adjust the position and size of the rectangle
        rect.x -= a;
        rect.y -= a;
        rect.width += (a * 2);
        rect.height += (a * 2);

        // Ensure the rectangle stays within the image boundaries
        rect.x = std::max(0, rect.x);
        rect.y = std::max(0, rect.y);
        rect.width = std::min(rect.width, imgWidth - rect.x);
        rect.height = std::min(rect.height, imgHeight - rect.y);

        // Check if the resulting rectangle is valid
        if (rect.width <= 0 || rect.height <= 0) {
            return false;
        }

        return true;
    }

    Mat applyComparison(Mat& img1, Mat& img2_aligned, int patchSize, int stepSize) {
        if (img1.size() != img2_aligned.size() || img1.type() != img2_aligned.type()) {
            std::cerr << "Images must have the same size and type" << std::endl;
            return Mat();
        }

        cv::Mat gray1, gray2;
        cv::cvtColor(img1, gray1, cv::COLOR_BGR2GRAY);
        cv::cvtColor(img2_aligned, gray2, cv::COLOR_BGR2GRAY);

        cv::Mat edges1, edges2;
        cv::Canny(gray1, edges1, 50, 150, 3, true); 
        cv::Canny(gray2, edges2, 50, 150, 3, true);
        cv::imwrite("edges.png", edges1);
        cv::imwrite("scanned_edges.png", edges2);
        int rows = img1.rows;
        int cols = img1.cols;

        cv::Mat mask = Mat::zeros(rows, cols, CV_8UC1);

        int cnt = 0;
        for (int y = 0; y <= rows - patchSize; y += stepSize) {
            for (int x = 0; x <= cols - patchSize; x += stepSize) {
                cv::Rect patchRect(x, y, patchSize, patchSize);
                cv::Mat patch1 = edges1(patchRect);
                cv::Mat patch2 = edges2(patchRect);

                Mat processed = compareEdgeImages(patch1, patch2, 30);
                processed.copyTo(mask(patchRect));
                // imshow("patch1", patch1);
                // imshow("patch2", patch2);
                // imshow("processed", processed);
                // waitKey(0);

            }
        }

        return mask.clone();
    }


}
