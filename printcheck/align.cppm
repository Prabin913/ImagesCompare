module;
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
// #include <g3log/g3log.hpp>
#include <vector>
#include <ranges>

#include <iostream>
#define LOG(level) std::clog

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
	 *    Helper structure to store information about the regions of image that should be ignored
	 *    from error calculation after alignment.
	 */
	struct AlignData
	{
			//!  aligned image
		Mat Aligned;

			//!  areas that need to be accounted for error calculation
		Mat Mask;
	};

	/*!
	 *  @details
	 *    Returns the aligned version of the tst image to match reference
	 *  @param  ref
	 *    reference image
	 *  @param  tst
	 *    test image
	 *  @return
	 *    aligned image
	 */
	AlignData align_orb( const Mat& ref, const Mat& tst)
	{
		constexpr auto compute = []( const Mat& img)
			{
				static auto orb = ORB::create( 10'000);
				Mat gray;
				cvtColor( img, gray, COLOR_BGR2GRAY);
				OrbDescriptor od;
				orb->detectAndCompute( gray, {}, od.Keypoints, od.Descriptors);
				LOG(INFO) << "Orb successfully found " << od.Keypoints.size() << " descriptors";
				return od;
			};
		const auto ref_orb = compute( ref);
		const auto tst_orb = compute( tst);

		BFMatcher bfm( NORM_HAMMING, true);
		vector<DMatch> matches;
		bfm.match( ref_orb.Descriptors, tst_orb.Descriptors, matches);
		LOG(INFO) << "Found #" << matches.size() << "matches";

		vector<Point2f> srcs, dsts;
		srcs.reserve( matches.size()), dsts.reserve(matches.size());
		rg::sort( matches, {}, &DMatch::distance);
		for ( const auto& match : matches)
		{
			srcs.emplace_back( ref_orb.Keypoints[ match.queryIdx].pt );
			dsts.emplace_back( tst_orb.Keypoints[ match.trainIdx].pt );
		}
		auto hom = findHomography( dsts, srcs, RANSAC);
		LOG(INFO) << "Homography found: " << hom;

		Mat aligned;
		warpPerspective( tst, aligned, hom, ref.size());
		Mat valid( ref.size(), CV_8UC1, Scalar{255} );
		warpPerspective( tst, aligned, hom, ref.size());
		warpPerspective( valid, valid, hom, valid.size());
		return {aligned, valid};
	}

}
