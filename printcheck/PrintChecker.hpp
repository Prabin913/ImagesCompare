/*! @file PrintChecker.hpp
 *  @author Gabor Szijarto
 */
#ifndef PRINTCHECKLEAN_PRINTCHECKER_HPP
#define PRINTCHECKLEAN_PRINTCHECKER_HPP

#include <opencv2/core.hpp>
#include <filesystem>

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
			//cv::Mat PrintChecker::process( const std::filesystem::path& ref, const std::filesystem::path& scan, double *diff)
			cv::Mat process(const std::filesystem::path& ref, const std::filesystem::path& scan, double* diff);

				//!  update visualization with the new requested limit
			cv::Mat applyLimit( int, int, double*);

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

			//!  stores the loaded scanned image
			cv::Mat _scanned;

			//!  stores the scanned image
			cv::Mat _mask;

				//!  stores the last processed calculated error
			cv::Mat _error;

				//!  stores the colormap converted version of \ref _error
			cv::Mat _errormap;
	};

}

#endif
