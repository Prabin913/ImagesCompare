module;
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <vector>
#include <ranges>
#include <iostream>
#include "..\utils.h"

export module printcheck.align;

namespace rg = std::ranges;
using namespace cv;
using namespace cv::xfeatures2d;
using std::vector;

struct OrbDescriptor
{
    vector<KeyPoint> Keypoints;
    Mat Descriptors;
};

export namespace printcheck
{
    /*!
     *  @details
     *    Helper structure to store information about the regions of the image that should be ignored
     *    from error calculation after alignment.
     */
    struct AlignData
    {
        Mat Homography; 
        Mat Matched; 
    };

	AlignData align_orb(const Mat& ref, const Mat& tst)
	{
		constexpr auto compute = [](const Mat& img)
			{
				static auto orb = ORB::create();
				Mat workImg = img;
				if (workImg.channels() != 1)
					cvtColor(workImg, workImg, COLOR_BGR2GRAY);
				OrbDescriptor od;
				orb->detectAndCompute(workImg, {}, od.Keypoints, od.Descriptors);
				WriteLogFile(L"ORB successfully found %d descriptors", od.Keypoints.size());
				return od;
			};

		const auto ref_orb = compute(ref);
		const auto tst_orb = compute(tst);

		BFMatcher bfm(NORM_HAMMING, true);
		vector<DMatch> matches;
		bfm.match(ref_orb.Descriptors, tst_orb.Descriptors, matches);
		WriteLogFile(L"Found #%d matches", matches.size());

		vector<Point2f> srcs, dsts;
		srcs.reserve(matches.size());
		dsts.reserve(matches.size());
		rg::sort(matches, {}, &DMatch::distance);

		std::vector<DMatch> good_matches;
		const float ratio_thresh = 0.7f;
		for (size_t i = 0; i < matches.size() * ratio_thresh + 1; i++)
		{
			good_matches.push_back(matches[i]);
		}

		// Draw top matches
#ifdef DRAW_MATCH
		Mat imMatches;
		drawMatches(ref, ref_orb.Keypoints, tst, tst_orb.Keypoints, good_matches, imMatches);
#endif

		for (const auto& match : good_matches)
		{
			srcs.emplace_back(ref_orb.Keypoints[match.queryIdx].pt);
			dsts.emplace_back(tst_orb.Keypoints[match.trainIdx].pt);
		}

		auto hom = findHomography(dsts, srcs, RANSAC);
		WriteLogFile(L"Homography found: %d", hom);
				
		return { hom, hom };
	}

//#define DRAW_MATCH
    AlignData align_surf(const Mat& img_object, const Mat& img_scene)
	{
		if (img_object.channels() != 1)
			cvtColor(img_object, img_object, COLOR_BGR2GRAY);
		if (img_scene.channels() != 1)
			cvtColor(img_scene, img_scene, COLOR_BGR2GRAY);

		if (img_object.empty() || img_scene.empty()) {
            return { Mat(), Mat() };
		}

		//-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
		int minHessian = 400;
		Ptr<SURF> detector = SURF::create(minHessian);
		std::vector<KeyPoint> keypoints_object, keypoints_scene;
		Mat descriptors_object, descriptors_scene;
		detector->detectAndCompute(img_object, noArray(), keypoints_object, descriptors_object);
		detector->detectAndCompute(img_scene, noArray(), keypoints_scene, descriptors_scene);

		//-- Step 2: Matching descriptor vectors with a FLANN based matcher
		// Since SURF is a floating-point descriptor NORM_L2 is used
		Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
		std::vector< std::vector<DMatch> > knn_matches;
		matcher->knnMatch(descriptors_object, descriptors_scene, knn_matches, 2);

		//-- Filter matches using the Lowe's ratio test
		const float ratio_thresh = 0.75f;
		std::vector<DMatch> good_matches;
		for (size_t i = 0; i < knn_matches.size(); i++)
		{
			if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
			{
				good_matches.push_back(knn_matches[i][0]);
			}
		}

#ifdef DRAW_MATCH
		//-- Draw matches
		Mat img_matches;
		drawMatches(img_object, keypoints_object, img_scene, keypoints_scene, good_matches, img_matches, Scalar::all(-1),
			Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
#endif
		
		//-- Localize the object
		std::vector<Point2f> obj;
		std::vector<Point2f> scene;
		for (size_t i = 0; i < good_matches.size(); i++)
		{
			//-- Get the keypoints from the good matches
			obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
			scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
		}
		Mat H = findHomography(obj, scene, RANSAC);

#ifdef DRAW_MATCH
		//-- Get the corners from the image_1 ( the object to be "detected" )
		std::vector<Point2f> obj_corners(4);
		obj_corners[0] = Point2f(0, 0);
		obj_corners[1] = Point2f((float)img_object.cols, 0);
		obj_corners[2] = Point2f((float)img_object.cols, (float)img_object.rows);
		obj_corners[3] = Point2f(0, (float)img_object.rows);
		std::vector<Point2f> scene_corners(4);
		perspectiveTransform(obj_corners, scene_corners, H);
		//-- Draw lines between the corners (the mapped object in the scene - image_2 )
		line(img_matches, scene_corners[0] + Point2f((float)img_object.cols, 0),
			scene_corners[1] + Point2f((float)img_object.cols, 0), Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[1] + Point2f((float)img_object.cols, 0),
			scene_corners[2] + Point2f((float)img_object.cols, 0), Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[2] + Point2f((float)img_object.cols, 0),
			scene_corners[3] + Point2f((float)img_object.cols, 0), Scalar(0, 255, 0), 4);
		line(img_matches, scene_corners[3] + Point2f((float)img_object.cols, 0),
			scene_corners[0] + Point2f((float)img_object.cols, 0), Scalar(0, 255, 0), 4);
#endif

#ifdef DRAW_MATCH
        return { H, img_matches };
#else
		return { H, H };
#endif
	}
}

