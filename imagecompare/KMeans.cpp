/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: KMeans.cpp 2741 2019-04-09 17:04:20Z stephane $
 */

#include "KMeans.hpp"


size_t vz::k_means(const vz::VDouble & data, size_t k, VSizet & assignments, VDouble & centroids)
{
	size_t iteration = 0;
	assignments.clear();
	centroids.clear();

	if (k < 1)
	{
		/// @throw std::invalid_argument if @p k (number of clusters) is invalid
		throw std::invalid_argument("invalid number of clusters");
	}

	if (data.empty())
	{
		return 0;
	}

	// pick a few spots from the data to use as initial centroids
	VDouble new_centroids;
	VDouble old_centroids;
	if (true)
	{
		// create a sorted version of the data which at the same time will also excludes duplicates.
		// This will help us better pick relevant initial centroids spread across the entire range.
		// (Particularly important to us since this k-means runs on 1-D data, and a wide ranging
		// set of initial centroids seems to be key to making it work correctly.)
		SDouble s(data.begin(), data.end());

		// obviously we cannot have more clusters than we have data points in the input
		k = std::min(s.size(), k);

		new_centroids.reserve(k);
		const size_t step = s.size() / k;
		auto iter = s.begin();
		while (new_centroids.size() < k)
		{
			new_centroids.push_back(*iter);
			std::advance(iter, step);
		}
	}

	// Each data point needs to remember the cluster to which it has been assigned.  This is done
	// via "assignments", which stores a cluster index corresponding to every single data point.
	VSizet new_assignments(data.size());
	VSizet old_assignments(data.size());
	size_t unchanged_count = 0;

	for (iteration = 0; iteration < 99; iteration++)
	{
		// remember the assignments and centroids so we can compare the old and new results
		new_assignments.swap(old_assignments);
		old_centroids = new_centroids;

		for (size_t idx = 0; idx < data.size(); idx++)
		{
			// STEP #1:  compute the distance from each data point to each cluster centroid
			double best_distance = std::numeric_limits<double>::max();
			size_t best_cluster = 0;
			for (size_t cluster = 0; cluster < k; ++cluster)
			{
				const double distance = std::pow(data[idx] - old_centroids[cluster], 2);
				if (distance < best_distance)
				{
					best_distance = distance;
					best_cluster = cluster;
				}
			}

			// STEP #2:  assign each point to the centroid it is closest to
			new_assignments[idx] = best_cluster;
		}

		// now that all the data points have been assigned to a cluster, go through and sum up the new totals for each cluster
		new_centroids = VDouble(k, 0.0);
		VSizet counts(k, 0);
		for (size_t data_point_idx = 0; data_point_idx < data.size(); data_point_idx ++)
		{
			const size_t cluster_idx = new_assignments[data_point_idx];
			new_centroids[cluster_idx] += data[data_point_idx];
			counts[cluster_idx] ++;
		}

		// divide the individual sums by the number of data items in each cluster to get new centroids
		for (size_t cluster_idx = 0; cluster_idx < k; cluster_idx ++)
		{
			// watch out for divide-by-zero in case a cluster has zero data points assigned to it
			const double count = std::max(counts[cluster_idx], (size_t)1UL);
			new_centroids[cluster_idx] = new_centroids[cluster_idx] / count;
		}

		if (old_assignments == new_assignments)
		{
			// if we hit several iterations where the assignments haven't changed,
			// then we'll want to break out of the loop early, so track the number
			// of times the assignments have been the same
			unchanged_count ++;
		}
		else
		{
			unchanged_count = 0;
		}

		// check to see if we've reached a plateau and no longer making progress

		if (unchanged_count > 3)
		{
			break;
		}

		if (old_centroids == new_centroids && old_assignments == new_assignments)
		{
			break;
		}
	}

	assignments	.swap(new_assignments	);
	centroids	.swap(new_centroids		);

	// iterations are zero-based, so +1 to find out exactly how many iterations were completed
	return iteration + 1;
}


size_t vz::k_means_vector(const VDouble & data, size_t k, vz::VVDouble & results, VDouble & centroids)
{
	results.clear();
	centroids.clear();

	VSizet assignments;
	const size_t iterations = k_means(data, k, assignments, centroids);

	// combine the "data" and the "assignments" into a vector-of-vector-of-doubles
	results = VVDouble(centroids.size());
	for (size_t data_idx = 0; data_idx < data.size(); data_idx ++)
	{
		const size_t cluster_idx = assignments.at(data_idx);

		results.at(cluster_idx).push_back(data.at(data_idx));
	}

	return iterations;
}
