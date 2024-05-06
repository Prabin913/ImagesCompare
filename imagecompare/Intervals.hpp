/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: Intervals.hpp 2741 2019-04-09 17:04:20Z stephane $
 */

#pragma once

#include "vzTypes.hpp"

/** @file
 * Intervals detection on 1-D data.
 */


namespace vz
{
	/** Automatically determine some intervals in a vector of @p double values.  Based on some example code discovered at
	 * https://stackoverflow.com/questions/17479944/partitioning-an-float-array-into-similar-segments-clustering
	 * while looking into segmentation and natural break optimizations.
	 *
	 * @note This should produce better results than attempting to use @ref vz::k_means() on 1-D data.
	 *
	 * @param [in,out] data The input data vector is sorted by @p %intervals().
	 * @param [in] threshold_multiplier Used to calculate the threshold when determining where to start a new interval.
	 * The larger the threshold, the smaller the number of intervals.  A threshold multiplier near @p 0.0 will cause every
	 * value to be in a new interval.
	 *
	 * For example, given this data:
	 * ~~~~
	 * vz::VDouble data = { 1.1, 1.2, 1.3, 5.7, 5.8, 5.9, 12.0 };
	 * ~~~~
	 * Calling @p %vz::intervals() might return something similar to this:
	 * ~~~~
	 * results == { {1.1, 1.2, 1.3}, {5.7, 5.8, 5.9}, {12.0} );
	 * ~~~~
	 *
	 * @see @ref vz::k_means()
	 * @see Jenks Natural Breaks Optimization
	 * @see Kernel Density Estimation (KDE)
	 */
	VVDouble intervals(VDouble & data, const double threshold_multiplier = 2.0);

	/** Find one or more possible values based on the given intervals.  If multiple values are returned, they will be sorted
	 * from most-likely to least-likely as determined by the size of the interval groupings.
	 *
	 * @param [in] vv The intervals returned by @ref vz::intervals().
	 * @param [in] threshold Helps determine how many groupings to consider.  If set to 1.0, then only the largest grouping
	 * will be considered, while a value of 0.5 will take into consideration any grouping that is at least 1/2 the size of
	 * the largest interval.
	 *
	 * For example, given this data:
	 * ~~~~
	 * ?
	 * ~~~~
	 * Calling @p %vz::popular_values_from_intervals() might return something similar to this:
	 * ~~~~
	 * ?
	 * ~~~~
	 */
	VDouble popular_values_from_intervals(const VVDouble & vv, const double threshold = 0.5);

	/**
	 * Returns the average of the largest grouping within the intervals.
	 *
	 * For example, given this data:
	 * ~~~~
	 * ?
	 * ~~~~
	 * Calling @p %vz::best_value_from_intervals() might return something similar to this:
	 * ~~~~
	 * ?
	 * ~~~~
	 */
	double best_value_from_intervals(const VVDouble & vv);

	/** Convert the given interval to a text string.  The resulting string is meant for debug purposes, not serialization.
	 *
	 * For example:
	 * ~~~~{.txt}
	 * 6 intervals
	 * -> interval #1/6: 6 values, average=1.35, midrange=1.35, median=1.4, values: 1.1 1.2 1.3 1.4 1.5 1.6
	 * -> interval #2/6: 3 values, average=5.8, midrange=5.8, median=5.8, values: 5.7 5.8 5.9
	 * -> interval #3/6: 4 values, average=9.5, midrange=9.5, median=9.6, values: 9.2 9.4 9.6 9.8
	 * -> interval #4/6: 27 values, average=15.05, midrange=15.05, median=15.05, values: 15.01 15.01 15.01 15.02 15.02 15.02 15.03 15.03 15.03 15.04 15.04 15.04 15.05 15.05 15.05 15.06 15.06 15.06 15.07 15.07 15.07 15.08 15.08 15.08 15.09 15.09 15.09
	 * -> interval #5/6: 5 values, average=32, midrange=32, median=32, values: 30 31 32 33 34
	 * -> interval #6/6: 1 values, average=99, midrange=99, median=99, values: 99
	 * ~~~~
	 *
	 * @li average is the arithmetic mean (sum of every value divided by the number of values)
	 * @li midrange is the sum of the largest and smallest values divided by 2
	 * @li median is the central point of the data set
	 */
	std::string to_string(const VVDouble & vv);
}
