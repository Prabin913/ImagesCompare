module;
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <vector>
#include <ranges>
#include <iostream>
#include "..\utils.h"

export module printcheck.align;

namespace rg = std::ranges;
using namespace cv;
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
        Mat Aligned; //!< Aligned image
        Mat Mask; //!< Areas that need to be accounted for error calculation
    };

    /*!
     *  @details
     *    Returns the aligned version of the test image to match the reference image.
     *  @param  ref
     *    Reference image
     *  @param  tst
     *    Test image
     *  @return
     *    Aligned image data
     */
    AlignData align_orb(const Mat& ref, const Mat& tst)
    {
        constexpr auto compute = [](const Mat& img)
            {
                static auto orb = ORB::create(10'000);
                Mat gray;
                cvtColor(img, gray, COLOR_BGR2GRAY);
                OrbDescriptor od;
                orb->detectAndCompute(gray, {}, od.Keypoints, od.Descriptors);
                WriteLogFile(L"ORB successfully found %d descriptors",od.Keypoints.size());
                return od;
            };

        const auto ref_orb = compute(ref);
        const auto tst_orb = compute(tst);

        BFMatcher bfm(NORM_HAMMING, true);
        vector<DMatch> matches;
        bfm.match(ref_orb.Descriptors, tst_orb.Descriptors, matches);
        WriteLogFile(L"Found #%d matches",matches.size());
        
        vector<Point2f> srcs, dsts;
        srcs.reserve(matches.size());
        dsts.reserve(matches.size());
        rg::sort(matches, {}, &DMatch::distance);
        for (const auto& match : matches)
        {
            srcs.emplace_back(ref_orb.Keypoints[match.queryIdx].pt);
            dsts.emplace_back(tst_orb.Keypoints[match.trainIdx].pt);
        }

        auto hom = findHomography(dsts, srcs, RANSAC);
        WriteLogFile(L"Homography found: %d",hom);

        Mat aligned;
        warpPerspective(tst, aligned, hom, ref.size());
        Mat valid(ref.size(), CV_8UC1, Scalar{ 255 });
        warpPerspective(valid, valid, hom, valid.size());
        return { aligned, valid };
    }
}