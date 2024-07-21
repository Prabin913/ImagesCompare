module;
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/dnn.hpp>
#include "..\utils.h"

// #include <g3log/g3log.hpp>

#include <iostream>

export module printcheck.difference;

using cv::Mat, cv::absdiff, cv::cvtColor, cv::Rect;

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
        WriteLogFile(L"calculating error mask with limit: %d",limit);
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
//            bool foundWhite = false;

            // Check neighborhood pixel by pixel
//             for (int dy = -halfSize; dy <= halfSize && !foundWhite; ++dy)
//             {
//                 for (int dx = -halfSize; dx <= halfSize && !foundWhite; ++dx)
//                 {
//                     for (int ddy = -1; ddy <= 1 && !foundWhite; ++ddy)
//                     {
//                         for (int ddx = -1; ddx <= 1 && !foundWhite; ++ddx)
//                         {
// 							int nx = point.x + dx + ddx;
// 							int ny = point.y + dy + ddy;
// 
// 							// Check if the neighboring pixel is within bounds
// 							if (nx >= 0 && nx < refBinary.cols && ny >= 0 && ny < refBinary.rows)
// 							{
// 								if (refBinary.at<uchar>(ny, nx) == 255)
// 								{
// 									foundWhite = true;
// 								}
// 							}
//                         }
//                     }
//                 }
//             }
// 
//             if (foundWhite)
//             {
//                 output.at<uchar>(point.y, point.x) = 0;
//             }
            if (point.x >= 0 && point.x < refBinary.cols && point.y >= 0 && point.y < refBinary.rows)
            {
				if (refBinary.at<uchar>(point.y, point.x) == 255)
				{
					output.at<uchar>(point.y, point.x) = 0;
				}
            }			
        }

        return output;
    }

    std::vector<cv::Rect> findContours(cv::Mat mask, float minArea)
    {
        int rows = mask.rows;
        int cols = mask.cols;

        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        std::vector<cv::Rect> boxes;
        for (size_t i = 0; i < contours.size(); i++) 
        {
            double area = contourArea(contours[i]);
            if (area <= minArea) 
            {
                continue;
            }

            cv::Rect rect = boundingRect(contours[i]);
            boxes.push_back(rect);
        }
        return boxes;
    }

	struct SSIMData
	{
		double  score;
        cv::Mat diff;
	};
 
    SSIMData computeSSIM(const cv::Mat& img1, const cv::Mat& img2)
    {
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

        return { cv::mean(ssim_map)[0], ssim_map};
    }

    bool enlargeRect(cv::Rect& rect, int a, int imgWidth, int imgHeight) 
    {
        // Adjust the position and size of the rectangle
        rect.x -= a;
        rect.y -= a;
        rect.width += (a * 2);
        rect.height += (a * 2);

        // Ensure the rectangle stays within the image boundaries
        rect.x = max(0, rect.x);
        rect.y = max(0, rect.y);
        rect.width = min(rect.width, imgWidth - rect.x);
        rect.height = min(rect.height, imgHeight - rect.y);

        // Check if the resulting rectangle is valid
        if (rect.width <= 0 || rect.height <= 0) 
        {
            return false;
        }

        return true;
    }

    cv::Mat computeEdges(const Mat& image) 
    {
        cv::Mat gray, edges;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, 100, 200);
        return edges;
    }


    Mat applyComparison(Mat& img1, Mat& img2_aligned, int patchSize, int stepSize) 
    {
        if (img1.size() != img2_aligned.size() || img1.type() != img2_aligned.type()) 
        {
            std::cerr << "Images must have the same size and type" << std::endl;
            return Mat();
        }


        Mat edges1 = computeEdges(img1);
        Mat edges2 = computeEdges(img2_aligned);
       /* Mat edges1 = detectEdgesWithHED(model_hed, img1, 500, 500);
        Mat edges2 = detectEdgesWithHED(model_hed, img2_aligned, 500, 500);*/

        cv::imwrite("edges.png", edges1);
        cv::imwrite("scanned_edges.png", edges2);
        int rows = img1.rows;
        int cols = img1.cols;

        //////////////////////////////////////////////////////////////////////////
        // Get origin offset
		constexpr auto compute = [](const Mat& src1, const Mat& src2)
			{
				cv::Mat diff;
				cv::absdiff(src1, src2, diff); // Compute absolute difference
				return cv::mean(diff.mul(diff))[0]; // Mean of squared differences
			};

		std::vector<cv::Point> whitePixels;
		cv::findNonZero(edges2, whitePixels);

		char matIdx = 0;
        double prev_mse = 10000;// std::numeric_limits<double>::max();

        int offsetX = 0, offsetY = 0;
        const int neighborhoodSize = 30;
		const auto& white_point = whitePixels[0];
		for (int dy = -neighborhoodSize; dy <= neighborhoodSize; ++dy)
		{
			for (int dx = -neighborhoodSize; dx <= neighborhoodSize; ++dx)
			{
				if (white_point.x + dx < 0 || white_point.x + dx >= edges2.cols || white_point.x + dx >= edges1.cols ||
					white_point.y + dy < 0 || white_point.y + dy >= edges2.rows || white_point.y + dy >= edges1.rows)
					continue;

				cv::Rect rect1(white_point.x, white_point.y, neighborhoodSize * 2, neighborhoodSize * 2);
				cv::Rect rect2(white_point.x + dx, white_point.y + dy, neighborhoodSize * 2, neighborhoodSize * 2);
				double mse1 = compute(edges1(rect1), edges2(rect2));
				double mse2 = compute(edges1(rect2), edges2(rect1));
				double mse = min(mse1, mse2); //cv::min(mse1, mse2);
				if (mse < prev_mse) 
                {
					prev_mse = mse;

					offsetX = dx; offsetY = dy;
					if (mse1 > mse2) matIdx = 1;
				}
			}
		}

		cv::Mat trans = Mat::zeros(rows, cols, CV_8UC1);
        if (matIdx) 
        {
            edges2(cv::Rect(offsetX, offsetY, cols - offsetX, rows - offsetY)).copyTo(trans(cv::Rect(0, 0, cols - offsetX, rows - offsetY)));
            edges2 = trans;
        }
        else 
        {
            edges1(cv::Rect(offsetX, offsetY, cols - offsetX, rows - offsetY)).copyTo(trans(cv::Rect(0, 0, cols - offsetX, rows - offsetY)));
            edges1 = trans;
        }

		//cv::imwrite("edges_trans.png", edges1);
		//cv::imwrite("scanned_edges_trans.png", edges2);
        //////////////////////////////////////////////////////////////////////////

        cv::Mat mask = Mat::zeros(rows, cols, CV_8UC1);

        int cnt = 0;
        for (int y = 0; y <= rows - patchSize; y += stepSize) 
        {
            for (int x = 0; x <= cols - patchSize; x += stepSize) 
            {
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

	double calculateMSE(const cv::Mat& I1, const cv::Mat& I2) {
        cv::Mat s1;
		absdiff(I1, I2, s1);       // |I1 - I2|
		s1.convertTo(s1, CV_32F);  // Convert to float to avoid overflow
		s1 = s1.mul(s1);           // (|I1 - I2|)^2

		cv::Scalar s = sum(s1);        // Sum all the elements

		double mse = (s[0] + s[1] + s[2]) / (double)(I1.channels() * I1.total());
		return mse;
	}

	void removeShadowNormalize(cv::Mat const& src, cv::Mat& result2_norm_img) {
		std::vector<cv::Mat> channels;
		cv::split(src, channels);

		cv::Mat zero = cv::Mat::zeros(src.size(), CV_8UC1);

		cv::Mat kernel;
		cv::Mat diff_img[3];
		cv::Mat norm_img[3];
		for (int i = 0; i < 3; i++) {
            cv::normalize(channels[i], norm_img[i], 0, 255, cv::NORM_MINMAX, CV_8UC1, cv::noArray());
		}

		std::vector<cv::Mat> R2B1 = { norm_img[0], zero, zero };
		std::vector<cv::Mat> R2G1 = { zero, norm_img[1], zero };
		std::vector<cv::Mat> R2R1 = { zero, zero, norm_img[2] };

		cv::Mat result2_B;
		cv::Mat result2_G;
		cv::Mat result2_R;

		cv::merge(R2B1, result2_B);
		cv::merge(R2G1, result2_G);
		cv::merge(R2R1, result2_R);

		cv::bitwise_or(result2_B, result2_G, result2_G);
		cv::bitwise_or(result2_G, result2_R, result2_norm_img);
	}
}