/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: KMeans.hpp 2741 2019-04-09 17:04:20Z stephane $
 */

#pragma once

#include "vzTypes.hpp"

/** @file
 * Simple k-mean and cluster detection functions.
 */


namespace vz
{
	/** Perform k-means clustering on a vector of @p doubles.  Unfortunately, k-means requires the caller to know the exact
	 * number of clusters.
	 *
	 * @warning Running k-means on single dimension data (such as a vector of doubles) is not the recommended way to use
	 * k-means cluster detection.  Especially with a data set that contains wide gaps between the data, or data with a large
	 * number of entries at one end of the range, the results are unlikely to be very useful.  Instead, you probably want to
	 * use @ref vz::intervals().
	 *
	 * @note This implementation is based on two tutorials about k-means:
	 * - John McCullock:
	 *   - http://mnemstudio.org/clustering-k-means-introduction.htm
	 *   - http://mnemstudio.org/clustering-k-means-example-1.htm
	 *   - http://mnemstudio.org/clustering-k-means-example-2.htm
	 * - Peter Goldsborough (peter at goldsborough.me):
	 *   - http://www.goldsborough.me/c++/python/cuda/2017/09/10/20-32-46-exploring_k-means_in_python,_c++_and_cuda/
	 *   - https://github.com/goldsborough/k-means
	 *
	 * @param [in] data Vector of @p doubles.
	 * @param [in] k The number of clusters.  The result may have fewer than this if there isn't enough input data.
	 * @param [out] assignments A vector of indexes indicating the cluster into which a data point was assigned.  Indexes will range from zero to @p k - 1.
	 * @param [out] centroids The final centroids as determined by k-means -- the location of each cluster.
	 * @returns Returns the number of iterations required.
	 *
	 * For example, given this data:
	 * ~~~~
	 * vz::VDouble data = { 1.1, 1.2, 1.3, 5.7, 5.8, 5.9 };
	 * ~~~~
	 * Calling @p %vz::k_means() with @p k=2 might return something similar to the following after 2 iterations:
	 * ~~~~
	 * assignments == { 0, 0, 0, 1, 1, 1 };
	 * centroids   == { 1.2, 5.8 };
	 * ~~~~
	 *
	 * @see @ref vz::k_means_vector()
	 */
	size_t k_means(const VDouble & data, size_t k, VSizet & assignments, VDouble & centroids);

	/** Similar to @ref vz::k_means(), but the results that would have been returned in @p assignments are de-referenced
	 * and formatted as a vector of clusters.
	 *
	 * For example, given this data:
	 * ~~~~
	 * vz::VDouble data = { 1.1, 1.2, 1.3, 5.7, 5.8, 5.9 };
	 * ~~~~
	 * Calling @p %k_means_split() with @p k=2 might return something similar to the following after 2 iterations:
	 * ~~~~
	 * results   == { {1.1, 1.2, 1.3}, {5.7, 5.8, 5.9} };
	 * centroids == { 1.2, 5.8 };
	 * ~~~~
	 *
	 * @see @ref vz::k_means()
	 */
	size_t k_means_vector(const VDouble & data, size_t k, vz::VVDouble & results, VDouble & centroids);
}
