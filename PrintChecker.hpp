/*! @file PrintChecker.hpp
 *  @author Gabor Szijarto
 */
#ifndef PRINTCHECKLEAN_PRINTCHECKER_HPP
#define PRINTCHECKLEAN_PRINTCHECKER_HPP

#include <opencv2/core.hpp>
#include <filesystem>

 // Code added by Ismayil X
static cv::Mat getDifferenceBetweenImageWithSSIM(std::string imgfirst, std::string imgsecond, double* score);


namespace printcheck
{

	/*!
	 *  @details
	 *    Wrapper class that hold together functionalities provided by the library.
	 *  @warning
	 *    No proper checks provided as this is just a quick solution to be shared for testing..
	 */
	class PrintChecker
	{
		public:
		//  Functionality
				//!  process the given reference image and the scanned version of the same content
			cv::Mat process( const std::filesystem::path&, const std::filesystem::path&, double &diff);

				//!  update visualization with the new requested limit
			cv::Mat applyLimit( int);

			// my own tests
			cv::Mat test(const std::filesystem::path& ref, const std::filesystem::path& scan);

		//  Getters
				//!  returns with a deep copy of the calculated error map
			cv::Mat error() const { return _error.clone(); }

				//!  returns with a deep copy of the colormap version of error map
			cv::Mat errormap() const { return _errormap.clone(); }

		private:
		//  Private Members
				//!  currently applied threshold for error visualization [0-255]
			int _limit = 0u;

				//!  stores the loaded last reference image, stored as we need it in \ref applyLimit
			cv::Mat _ref;

				//!  stores the last processed calculated error
			cv::Mat _error;

				//!  stores the colormap converted version of \ref _error
			cv::Mat _errormap;
	};

}

#endif
