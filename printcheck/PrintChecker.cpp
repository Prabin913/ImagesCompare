/*! @file PrintChecker.cpp @author Gabor Szijarto */
#include "PrintChecker.hpp"

import printcheck.cv;
import printcheck.align;
import printcheck.difference;

static cv::Mat getDifferenceBetweenImageWithSSIM(cv::Mat first, cv::Mat second, double* score);
cv::Mat finalWork;
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
		//std::string s1(ref.begin(),ref.end());
		//std::string s2(scan.begin(),scan.end());
		std::string s1 = ref.string();
		std::string s2 = scan.string();

		cv::Mat refer = printcheck::read(ref);
		cv::Mat tst = printcheck::read( scan);
		if (refer.empty() || tst.empty())
		{
			return {};
		}
		cv::Mat result = getDifferenceBetweenImageWithSSIM(refer,tst,diff);
		finalWork = result;
		return result;
		//auto aligned = printcheck::align_orb( _ref, tst);
		//_error = printcheck::diff_pixel( _ref, aligned.Aligned);
		//_error.setTo(0, ~aligned.Mask);

		//// Calculate total number of pixels and number of different pixels
		//int totalPixels = _ref.rows * _ref.cols * _ref.channels();
		//cv::Scalar diffSum = cv::sum(_error);
		//double numDifferentPixels = diffSum[0] / 255.0; // Assuming grayscale difference image

		//// Calculate percentage of different pixels
		//diff = (numDifferentPixels / totalPixels) * 100.0;


		//return (cv::Mat)_ref;
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
		return finalWork;


	}

}

// Code added by Ismayil X
#include <sstream>
#include "VzImgCmp.hpp"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "PrintChecker.hpp"
using namespace cv;
using namespace std;

static double calculateSSIM(const Mat& img1, const Mat& img2,Mat &diff)
{
	const double C1 = 6.5025, C2 = 58.5225;
	Mat img1_float, img2_float;
	img1.convertTo(img1_float, CV_32F);
	img2.convertTo(img2_float, CV_32F);
	Mat mu1, mu2;
	GaussianBlur(img1_float, mu1, Size(11, 11), 1.5);
	GaussianBlur(img2_float, mu2, Size(11, 11), 1.5);
	Mat mu1_sq = mu1.mul(mu1);
	Mat mu2_sq = mu2.mul(mu2);
	Mat mu1_mu2 = mu1.mul(mu2);
	Mat sigma1_sq, sigma2_sq, sigma12;
	GaussianBlur(img1_float.mul(img1_float), sigma1_sq, Size(11, 11), 1.5);
	sigma1_sq -= mu1_sq;
	GaussianBlur(img2_float.mul(img2_float), sigma2_sq, Size(11, 11), 1.5);
	sigma2_sq -= mu2_sq;
	GaussianBlur(img1_float.mul(img2_float), sigma12, Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;
	Mat t1, t2, t3;
	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2);
	t1 = mu1_sq + mu2_sq + C1;
	t2 = sigma1_sq + sigma2_sq + C2;
	t1 = t1.mul(t2);
	Mat ssim_map;
	divide(t3, t1, ssim_map);
	Scalar mssim = mean(ssim_map);
	ssim_map.convertTo(diff, CV_8U, 255);
	return mssim[0];


}
static void SSIMAligner(Mat img1, Mat img2, Mat* aligned)
{
	Ptr<ORB> orb = ORB::create();
	vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;

	orb->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
	orb->detectAndCompute(img2, noArray(), keypoints2, descriptors2);

	// Match descriptors using BFMatcher
	BFMatcher matcher(NORM_HAMMING);
	vector<DMatch> matches;
	matcher.match(descriptors1, descriptors2, matches);

	// Sort matches by distance (best matches first)
	sort(matches.begin(), matches.end());

	// Keep only the top matches
	const int numGoodMatches = static_cast<int>(matches.size() * 0.15);
	matches.erase(matches.begin() + numGoodMatches, matches.end());

	// Draw matches for visualization
	/*Mat imgMatches;
	drawMatches(img1, keypoints1, img2, keypoints2, matches, imgMatches);*/
	//imwrite("matches.png", imgMatches);

	// Extract location of good matches
	vector<Point2f> points1, points2;
	for (size_t i = 0; i < matches.size(); i++)
	{
		points1.push_back(keypoints1[matches[i].queryIdx].pt);
		points2.push_back(keypoints2[matches[i].trainIdx].pt);
	}

	// Find homography
	Mat homography = findHomography(points2, points1, RANSAC);

	// Warp img2 to align with img1

	warpPerspective(img2, *aligned, homography, img1.size());

	// Save the aligned image
	//imwrite("aligned_image.png", img2Aligned);

	// Display the results
	/*namedWindow("Matches", WINDOW_NORMAL);
	imshow("Matches", imgMatches);

	namedWindow("Aligned Image", WINDOW_NORMAL);
	imshow("Aligned Image", img2Aligned);
	waitKey(0);*/
}



static cv::Mat getDifferenceBetweenImageWithSSIM(cv::Mat first, cv::Mat second, double* score)
{
	cv::Size size1 = first.size();
	cv::Size size2 = second.size();

	if (size1 != size2)
	{
		std::cout << "The images are of different sizes. Resizing image2 to match image1." << std::endl;
		SSIMAligner(first, second, &second);  // Assuming SSIMAligner is defined elsewhere
		cv::resize(second, second, size1);
	}
	else
	{
		std::cout << "The images are of the same size." << std::endl;
	}

	cv::Mat first_gray, second_gray;
	cv::cvtColor(first, first_gray, cv::COLOR_BGR2GRAY);
	cv::cvtColor(second, second_gray, cv::COLOR_BGR2GRAY);

	cv::Mat diff;
	*score = calculateSSIM(first_gray, second_gray, diff) * 100;  // Assuming calculateSSIM is defined elsewhere
	std::cout << "Similarity Score: " << *score << "%" << std::endl;

	Mat difff, diffGray, thresh;
    absdiff(first, second, difff);
    cvtColor(difff, diffGray, COLOR_BGR2GRAY);
    threshold(diffGray, thresh, 30, 255, THRESH_BINARY);

    // Create a mask with red color where differences are found
    Mat redMask = Mat::zeros(first.size(), first.type());
    redMask.setTo(Scalar(0, 0, 255), thresh);

    // Apply the red mask to the original image
    first.setTo(Scalar(0, 0, 255), thresh);
	return first;
}

//static cv::Mat getDifferenceBetweenImageWithSSIM(cv::Mat first, cv::Mat second, double* score)
//{
//	/*Mat first = imread(imgfirst);
//	Mat second = imread(imgsecond);
//
//	if (first.empty() || second.empty())
//	{
//		cout << "Could not open or find the images!" << endl;
//		Mat empt;
//		return empt;
//	}*/
//
//	cv::Size size1 = first.size();
//	cv::Size size2 = second.size();
//
//	// Compare the sizes and resize image2 if needed
//	if (size1 != size2) 
//	{
//		std::cout << "The images are of different sizes. Resizing image2 to match image1." << std::endl;
//		SSIMAligner(first, second, &second);
//		cv::resize(second, second, size1);
//	}
//	else
//	{
//		std::cout << "The images are of the same size." << std::endl;
//	}
//	//resize(second, second, first.size());
//	Mat first_gray, second_gray;
//	cvtColor(first, first_gray, COLOR_BGR2GRAY);
//	cvtColor(second, second_gray, COLOR_BGR2GRAY);
//
//	Mat diff;
//	*score = calculateSSIM(first_gray, second_gray, diff) * 100;
//	cout << "Similarity Score: " << *score << "%" << endl;
//
//	Mat thresh;
//	threshold(diff, thresh, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
//
//	vector<vector<Point>> contours;
//	findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
//
//	Mat mask = Mat::zeros(first.size(), CV_8UC3);
//	Mat filled = second.clone();
//
//	for (const auto& c : contours)
//	{
//		double area = contourArea(c);
//		if (area > 100) 
//		{
//			Rect bounding_rect = boundingRect(c);
//			rectangle(first, bounding_rect, Scalar(36, 255, 12), 2);
//			rectangle(second, bounding_rect, Scalar(36, 255, 12), 2);
//			drawContours(mask, vector<vector<Point>>{c}, -1, Scalar(0, 255, 0), -1);
//			drawContours(filled, vector<vector<Point>>{c}, -1, Scalar(0, 255, 0), -1);
//		}
//	}
//
//	//imshow("first", first);
//	//imshow("second", second);
//	//imshow("diff", diff);
//	//imwrite("diffa.png", diff);
//	//imshow("mask", mask);
//	/*imwrite("maska.png", mask);
//	imwrite("filled.png", filled);
//	waitKey(0);*/
//	return filled;
//
//}