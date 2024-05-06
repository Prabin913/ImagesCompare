/* VzImagination (C) 2018-2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id: Intervals.cpp 2766 2019-05-04 01:34:03Z stephane $
 */

#include "Intervals.hpp"
#include <algorithm>
#include <numeric>


vz::VVDouble vz::intervals(VDouble & data, const double threshold_multiplier)
{
	if (threshold_multiplier <= 0)
	{
		/// @throw std::invalid_argument if @p threshold_multiplier is <= 0
		throw std::invalid_argument("invalid threshold multiplier");
	}

	VVDouble vv;
	if (data.empty())
	{
		return vv;
	}

	std::sort(data.begin(), data.end());

	// find the difference between each number and its predecessor
	VDouble diffs;
	std::adjacent_difference(data.begin(), data.end(), std::back_inserter(diffs));
	// remember the very first item doesn't have a predecessor, meaning the difference is always 100% which we need to ignore

	// convert the differences to percentages (because the difference between 1 and 3 should be considered much more significant than the difference between 1001 and 1003)
	for (size_t idx = 0; idx < diffs.size(); idx ++)
	{
		if (data[idx] == 0.0)
		{
			diffs[idx] = 0.0;
		}
		else
		{
			diffs[idx] = std::abs(diffs[idx] / data[idx]);
		}
	}

	// remember the first item is 100% (aka 1.0) so start the total at -1.0 to counteract that bogus number we'd like to ignore
	const double total_diffs = std::accumulate(diffs.begin(), diffs.end(), -1.0);

	// find the average of all differences (divider should be "size() - 1" but then that exposes a divide-by-zero problem if the data has 1 item)
	const double average = total_diffs / static_cast<double>(diffs.size());

	const double interval_threshold = threshold_multiplier * average;

	VDouble v;
	for (size_t idx = 0; idx < data.size(); idx ++)
	{
		if (diffs.at(idx) != 0.0 && diffs.at(idx) >= interval_threshold)
		{
			// if we get here, then we've determined that we need to start a new interval

			// (the very first time through the loop, "v" is empty in which case we do nothing)
			if (v.empty() == false)
			{
				// remember the previous interval by pushing all the values back
				vv.push_back(v);
				v.clear();	// start a clean new interval
			}
		}

		v.push_back(data.at(idx));
	}
	vv.push_back(v); // remember the very last interval

	return vv;
}


vz::VDouble vz::popular_values_from_intervals(const vz::VVDouble & vv, const double threshold)
{
	// find which interval group has the most values
	size_t max_size = 0;
	for (const auto & v : vv)
	{
		if (v.size() > max_size)
		{
			max_size = v.size();
		}
	}

	vz::VDouble results;

	// only consider groups that are >= 1/2 of that maximum size
	for (const auto & v : vv)
	{
		if (v.size() >= threshold * max_size)
		{
			// compute the average for this grouping
			double average = 0.0;
			for (const double & d : v)
			{
				average += d;
			}
			average /= static_cast<double>(v.size());
			results.push_back(average);
		}
	}

	return results;
}


double vz::best_value_from_intervals(const vz::VVDouble & vv)
{
	double average = 0.0;

	// find which interval group has the most values
	size_t max_size = 0;
	for (const auto & v : vv)
	{
		if (v.size() > max_size)
		{
			max_size = v.size();
		}
	}

	// now that we know the largest group, find it again and compute the average within that group
	for (const auto & v : vv)
	{
		if (v.size() == max_size)
		{
			// compute the average of all values in this grouping
			for (const double & d : v)
			{
				average += d;
			}
			average /= static_cast<double>(max_size);
			break;
		}
	}

	return average;
}


std::string vz::to_string(const vz::VVDouble & vv)
{
	std::stringstream ss;

	ss << vv.size() << " intervals";
	size_t counter = 0;

	for (auto & v : vv)
	{
		counter ++;

		double average = 0.0;
		for (const double & d : v)
		{
			average += d;
		}
		average /= (v.empty() ? 1.0 : (double)v.size());

		// midrange is the lowest + highest values divided by 2
		const double midrange = (*v.begin() + *v.rbegin()) / 2.0;

		// median is the central point of the data set
		const double median = v[v.size() / 2];

		ss	<< std::endl
			<< "-> interval #" << counter << "/" << vv.size()
			<< ": " << v.size() << " values"
			<< ", average="		<< average
			<< ", midrange="	<< midrange
			<< ", median="		<< median
			<< ", values:";

		double previous_value = std::numeric_limits<double>::min();
		size_t count = 0;

		for (const double & d : v)
		{
			if (d == previous_value)
			{
				count ++;
			}
			else
			{
				// this is a new value -- print out the previous one, then remember this new one

				if (count > 1)
				{
					ss << " [x" << count << "]";
				}
				ss << " " << d;
				previous_value = d;
				count = 1;
			}
		}
		if (count > 1)
		{
			ss << " [x" << count << "]";
		}
	}

	return ss.str();
}
