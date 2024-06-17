// VZ Image Compare (C) 2020 Stephane Charette <stephanecharette@gmail.com>
// MIT license applies.  See "license.txt" for details.
// $Id: VzImgCmp.cpp 2964 2020-05-04 03:14:36Z stephane $

#include <VzImgCmp.hpp>
#include <fstream>

std::ofstream log_file;


vz::ImgCmp::ImgCmp()
{
	reset();

	return;
}


vz::ImgCmp::ImgCmp(cv::Mat master)
{
	reset();

	set_master_image(master);

	return;
}


vz::ImgCmp::~ImgCmp()
{
	return;
}


vz::ImgCmp & vz::ImgCmp::reset()
{
	annotation_colour = cv::Scalar(0, 0, 255); // pure red

	master_original			.clear();
	master_resized			.clear();
	candy_original			.clear();
	candy_resized			.clear();
	differences_contours	.clear();
	differences				= cv::Mat();
	differences_threshold	= cv::Mat();
	annotated_candy 		= cv::Mat();
	annotation_thickness	= 2;
	dilate_and_erode		= 3;
	resized_image_scale		= 0.25;
	min_contour_area		= 5.0;
	flags					= Flags::kNone;
	set_flag(Flags::kAnnotateAddGreenBorder);
	set_flag(Flags::kAnnotateAddRedBorder);
	set_flag(Flags::kDrawContour);

	verify_flags();

	return *this;
}


std::string vz::ImgCmp::version() const
{
	return VZIC_VERSION;
}


vz::ImgCmp & vz::ImgCmp::set_flag(const Flags f, const bool verify)
{
	flags = static_cast<Flags>(flags | f);

	// if we've set one of these flags, then we need to clear the corresponding one
	std::map<Flags, Flags> m =
	{
		{Flags::kDiffResized		, Flags::kDiffOriginalSize	},
		{Flags::kDiffOriginalSize	, Flags::kDiffResized		},

		{Flags::kDiffGreyscale		, Flags::kDiffColour		},
		{Flags::kDiffColour			, Flags::kDiffGreyscale		},

		{Flags::kThresholdTriangle	, Flags::kThresholdOtsu		},
		{Flags::kThresholdOtsu		, Flags::kThresholdTriangle	},

		{Flags::kAnnotateOverGrey	, Flags::kAnnotateOverColour},
		{Flags::kAnnotateOverColour	, Flags::kAnnotateOverGrey	},
	};
	if (m.count(f) == 1)
	{
		flags = static_cast<Flags>(flags & ~m.at(f));
	}

	if (verify)
	{
		verify_flags();
	}

	return *this;
}


vz::ImgCmp & vz::ImgCmp::clear_flag(const Flags f, const bool verify)
{
	flags = static_cast<Flags>(flags & ~f);

	// if we've cleared one of these flags, then we need to set the corresponding one
	std::map<Flags, Flags> m =
	{
		{Flags::kDiffResized		, Flags::kDiffOriginalSize	},
		{Flags::kDiffOriginalSize	, Flags::kDiffResized		},

		{Flags::kDiffGreyscale		, Flags::kDiffColour		},
		{Flags::kDiffColour			, Flags::kDiffGreyscale		},

		{Flags::kThresholdTriangle	, Flags::kThresholdOtsu		},
		{Flags::kThresholdOtsu		, Flags::kThresholdTriangle	},

		{Flags::kAnnotateOverGrey	, Flags::kAnnotateOverColour},
		{Flags::kAnnotateOverColour	, Flags::kAnnotateOverGrey	},
	};
	if (m.count(f) == 1)
	{
		flags = static_cast<Flags>(flags | m.at(f));
	}

	if (verify)
	{
		verify_flags();
	}

	return *this;
}


vz::ImgCmp & vz::ImgCmp::verify_flags()
{
	// exactly 1 option per group can be enabled
	const std::vector<std::vector<Flags>> groupings =
	{
		{Flags::kDiffResized		, Flags::kDiffOriginalSize	},
		{Flags::kDiffGreyscale		, Flags::kDiffColour		},
		{Flags::kThresholdTriangle	, Flags::kThresholdOtsu		},
		{Flags::kAnnotateOverGrey	, Flags::kAnnotateOverColour},
	};

	for (const auto group : groupings)
	{
		// check to see how many flags from this group have been set
		size_t number_of_flags_set_in_this_group = 0;
		for (const auto f : group)
		{
			if (flags & f)
			{
				number_of_flags_set_in_this_group ++;
			}
		}

		// we know *exactly* 1 flag from this group should be set
		if (number_of_flags_set_in_this_group == 0)
		{
			// since none are set, default to the first one
			set_flag(group[0]);
		}
		else if (number_of_flags_set_in_this_group > 1)
		{
			// too many flags have been set, so remove all of them except for the first one
			bool found_first_flag = false;
			for (const auto f : group)
			{
				if ((flags & f) && found_first_flag == false)
				{
					found_first_flag = true;
				}
				else
				{
					clear_flag(f);
				}
			}
		}
	}

	// check several other settings to ensure we're using sane values

	resized_image_scale	= std::clamp(resized_image_scale, 0.1	, 4.0		);
	dilate_and_erode	= std::clamp(dilate_and_erode	, 0		, 50		);
	min_contour_area	= std::clamp(min_contour_area	, 0.0	, 999999.0	);

	return *this;
}


vz::VStr vz::ImgCmp::debug_save()
{
	VStr v;

	std::stringstream ss;
	ss	<< std::time(nullptr) << "-"
		<< (flags & Flags::kDiffResized			? "resized_" + std::to_string((int)std::round(100.0 * resized_image_scale)) + "_pct-" : "")
		<< (flags & Flags::kDiffOriginalSize	? "full_size-"			: "")
		<< differences_contours.size()			<< "_contours-"
		<< (int)std::round(min_contour_area)	<< "_minarea-"
		<< dilate_and_erode						<< "_dilate_erode-"
		<< (flags & Flags::kDiffColour			? "colour_diff-"		: "")
		<< (flags & Flags::kDiffGreyscale		? "greyscale_diff-"		: "")
		<< (flags & Flags::kThresholdTriangle	? "triangle_threshold-"	: "")
		<< (flags & Flags::kThresholdOtsu		? "otsu_threshold-"		: "")
		<< (flags & Flags::kDrawContour			? "draw_contours-"		: "")
		<< (flags & Flags::kDrawRectangle		? "draw_rectangles-"	: "")
		;
	const std::string prefix = ss.str();

	std::string fn;
	fn = prefix + "1-master_original.png"	; v.push_back(fn); cv::imwrite(fn, master_original	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "2-master_resized.png"	; v.push_back(fn); cv::imwrite(fn, master_resized	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "3-candy_original.png"	; v.push_back(fn); cv::imwrite(fn, candy_original	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "4-candy_resized.png"		; v.push_back(fn); cv::imwrite(fn, candy_resized	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "5-differences.png"		; v.push_back(fn); cv::imwrite(fn, differences				, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "6-diff_threshold.png"	; v.push_back(fn); cv::imwrite(fn, differences_threshold	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "7-annotated.png"			; v.push_back(fn); cv::imwrite(fn, annotated_candy			, {cv::IMWRITE_PNG_COMPRESSION, 9});

	return v;
}


#define DEBUG 0

void display_dbg(std::string win_name, cv::Mat img)
{
#if DEBUG
	cv::Mat resized;
	cv::resize(img, resized, img.size() / 6);
	cv::imshow(win_name, resized);
#endif
}

void write_dbg(std::string filename, cv::Mat img)
{
#if DEBUG
	std::string path = "../" + filename + ".png";
	std::cout << "Saving to : " << path << std::endl;
	cv::imwrite(path, img);
#endif
	return;
}

cv::Ptr<cv::FeatureDetector> detector = cv::ORB::create(2000, 1.2, 8, 91, 0, 2, cv::ORB::ScoreType::HARRIS_SCORE, 91, 10);
cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMINGLUT);
std::vector<cv::KeyPoint> keypoints_master;
cv::Mat descriptors_master;
float processing_width_limit = 1500;
#define INTERP_METHOD cv::INTER_AREA

cv::Mat scale_input_img(cv::Mat img)
{
	cv::Mat resized;
	// if the image doesn't exceed limit, return as is
	if (img.size().width < processing_width_limit)
		return img;

	float aspect_ratio = processing_width_limit / img.size().width;
	cv::resize(img, resized,
		{ int(img.size().width * aspect_ratio), int(img.size().height * aspect_ratio) }, 0, 0, INTERP_METHOD);
	return resized;
}

vz::ImgCmp & vz::ImgCmp::set_master_image(cv::Mat img)
{
	verify_flags();
	master_original			.clear();
	master_resized			.clear();
	candy_original			.clear();
	candy_resized			.clear();
	differences_contours	.clear();
	differences				= cv::Mat();
	differences_threshold	= cv::Mat();
	annotated_candy			= cv::Mat();

	if (img.empty()			||
		img.channels() != 3	||
		img.cols < 10		||
		img.rows < 10		)
	{
		/// @throws std::invalid_argument if the new master image seems to be invalid
		throw std::invalid_argument("master image is invalid");
	}

	master_original.set_original(img);

	cv::Mat tmp = vz::resize(img, resized_image_scale);
	master_resized.set_original(tmp);

	// Compute image features used later for image alignment
	// We need to only do it once for master image
	auto type = vz::Imagination::EMatType::kOriginalImage;
	if (flags & Flags::kDiffGreyscale)
	{
		type = vz::Imagination::EMatType::kO2BlurGreyscale;
	}
	cv::Mat master;
	if (flags & Flags::kDiffResized)
		master = master_resized.get(type);
	else
		master = master_original.get(type);

	auto m_s = master.size();
	cv::Mat detection_mask = cv::Mat(m_s, CV_8UC1);
	detection_mask.setTo(255);
	int m_ratio = 7;
	cv::rectangle(detection_mask, cv::Point{ m_s.width / m_ratio, m_s.height / m_ratio },
		cv::Point{ m_s.width * (m_ratio - 1) / m_ratio, m_s.height * (m_ratio - 1) / m_ratio },
		cv::Scalar{ 0 }, cv::FILLED);
	write_dbg("detection_mask", detection_mask);

	cv::Mat master_small = scale_input_img(master);
	auto start = std::chrono::steady_clock().now();
	detector->detectAndCompute(master_small, cv::Mat(), keypoints_master, descriptors_master);
	log_file << "Master detection took: " << std::chrono::duration{ std::chrono::steady_clock().now() - start }.count() << std::endl;

	return *this;
}


size_t vz::ImgCmp::compare(cv::Mat img)
{
	verify_flags();
	candy_original			.clear();
	candy_resized			.clear();
	differences_contours	.clear();
	differences				= cv::Mat();
	differences_threshold	= cv::Mat();
	annotated_candy			= cv::Mat();

	if (master_original.empty())
	{
		/// @throw std::logic_error if the master image has not yet been defined
		throw std::logic_error("cannot compare image when the master image has not yet been defined");
	}

	if (img.empty()			||
		img.channels() != 3	||
		img.cols < 10		||
		img.rows < 10		)
	{
		/// @throws std::invalid_argument if the new image seems to be invalid
		throw std::invalid_argument("image is invalid");
	}

	candy_original = img;
	candy_resized = vz::resize(img, resized_image_scale);

	diff_images();
	get_contours();

	return differences_contours.size();
}

// add a row to affine matrix so we can multiply them
cv::Mat_<float> expand_dim(cv::Mat_<float> mat) {
	// initialize with zeros
	cv::Mat_<float> ret(3,3);
	ret.setTo(0);

	// simply copy 6 elements of source 2x3 matrix
	for (int r = 0; r < 2; r++)
		for (int c = 0; c < 3; c++)
			ret(r, c) = float(mat(r, c));

	// add 1 as last element
	ret(2, 2) = 1.f;
	return ret;
}

// we want to apply A transformation before B
cv::Mat_<float> combine_affine_transforms(cv::Mat_<float> A, cv::Mat_<float> B)
{
	cv::Mat_<float> ret(2, 3);
	A = expand_dim(A);
	B = expand_dim(B);
	cv::Mat_<float> mul = B * A;
	// simply copy 6 elements of source 2x3 matrix
	for (int r = 0; r < 2; r++)
		for (int c = 0; c < 3; c++)
			ret(r, c) = float(mul(r, c));
	return ret;
}

void vz::ImgCmp::align_images(cv::Mat& master, cv::Mat& candy)
{
	// align images using the image features
	std::vector<cv::KeyPoint> keypoints_candy;
	cv::Mat descriptors_candy;

	auto c_s = candy.size();
	int m_ratio = 7;
	cv::Mat candy_detection_mask = cv::Mat(c_s, CV_8UC1);
	candy_detection_mask.setTo(255);
	cv::rectangle(candy_detection_mask, cv::Point{ c_s.width / m_ratio, c_s.height / m_ratio },
		cv::Point{ c_s.width * (m_ratio - 1) / m_ratio, c_s.height * (m_ratio - 1) / m_ratio },
		cv::Scalar{ 0 }, cv::FILLED);
	write_dbg("candy_detection_mask", candy_detection_mask);

	auto start = std::chrono::steady_clock().now();
	detector->detectAndCompute(master, cv::Mat(), keypoints_candy, descriptors_candy);
	log_file << "Candy detection took: " << std::chrono::duration{ std::chrono::steady_clock().now() - start }.count() << std::endl;

	cv::Mat m_keypoints, c_keypoints;
	cv::drawKeypoints(master, keypoints_master, m_keypoints, cv::Scalar{ 255,0,255 }, cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::drawKeypoints(candy, keypoints_candy, c_keypoints, cv::Scalar{ 255,0,255 }, cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	write_dbg("master_keypoints", m_keypoints);
	write_dbg("candy_keypoints", c_keypoints);
	
	start = std::chrono::steady_clock().now();
#if 0
	std::vector<std::vector<cv::DMatch>> matches;

	matcher->knnMatch(descriptors_master, descriptors_candy, matches, 2);
	log_file << "Matching took: " << std::chrono::duration{ std::chrono::steady_clock().now() - start }.count() << std::endl;

	const float ratio_thresh = 0.3f;
	std::vector<cv::DMatch> good_matches;
	for (size_t i = 0; i < matches.size(); i++)
	{
		if (matches[i][0].distance < ratio_thresh * matches[i][1].distance)
			good_matches.push_back(matches[i][0]);
	}
#else
	std::vector<cv::DMatch> good_matches;
	matcher->match(descriptors_master, descriptors_candy, good_matches);
#endif

	// localize the object
	std::vector<cv::Point2f> obj;
	std::vector<cv::Point2f> scene;
	cv::Mat master_good_matches, candy_good_matches;
	master_good_matches = master.clone();
	candy_good_matches = candy.clone();

	std::vector<cv::DMatch> good_and_close_matches;

	double maximal_distance = 200;
	for (size_t i = 0; i < good_matches.size(); i++)
	{
		auto m_point = keypoints_master[good_matches[i].queryIdx].pt;
		auto obj_point = keypoints_candy[good_matches[i].trainIdx].pt;

		// filter out pairs that have very different x,y coordinates
		if (cv::norm(m_point - obj_point) > maximal_distance)
			continue;

		good_and_close_matches.push_back(good_matches[i]);

		obj.push_back(m_point);
		scene.push_back(obj_point);
		cv::circle(master_good_matches, keypoints_master[good_matches[i].queryIdx].pt, 5, cv::Scalar{ 255,255,0 }, 5);
		cv::circle(candy_good_matches, keypoints_candy[good_matches[i].trainIdx].pt, 5, cv::Scalar{ 255,255,0 }, 5);
	}

	// draw good matches
	cv::Mat match_lines;
	cv::drawMatches(master, keypoints_master, candy, keypoints_candy,
		good_and_close_matches, match_lines);
	write_dbg("match_lines", match_lines);

	log_file << "Found good matches: " << std::to_string(good_matches.size()) << std::endl;

	write_dbg("master_good_matches", master_good_matches);
	write_dbg("candy_good_matches", candy_good_matches);

	cv::Mat_<float> rough_warp_matrix(2,3);
	//warp_matrix = cv::estimateAffinePartial2D(scene, obj);
	rough_warp_matrix = cv::estimateAffine2D(scene, obj);
	// scale back the warp matrix
	if (rough_warp_matrix.empty()) {
		// It means we couldn't estimate a transformation
		return;
	}

	// warp the candy image to align with the master
	// fill the borders with white color
	cv::Mat candy_warped;
	cv::Mat master_gray, candy_gray;
	cv::Scalar white{ 255,255,255 };
	cv::cvtColor(candy, candy_gray, cv::COLOR_BGR2GRAY);
	cv::warpAffine(candy_gray, candy_warped, rough_warp_matrix, master.size(),
		INTERP_METHOD, cv::BORDER_CONSTANT, white);

	cv::cvtColor(master, master_gray, cv::COLOR_BGR2GRAY);

	cv::Mat_<float> fine_warp_matrix;
	start = std::chrono::steady_clock().now();
	cv::findTransformECC(candy_warped, master_gray, fine_warp_matrix, cv::MOTION_AFFINE);
	log_file << "findTransformECC: " << std::chrono::duration{ std::chrono::steady_clock().now() - start }.count() << std::endl;

	auto warp_combined = combine_affine_transforms(rough_warp_matrix, fine_warp_matrix);
	// Apply scale to the translation elements so we can use it on big image
	//warp_combined(0, 2) *= alignment_scale;
	//warp_combined(1, 2) *= alignment_scale;

	cv::Mat candy_warped_big;
	// appy both transformations using matrix multiplication
	cv::warpAffine(candy, candy_warped_big, warp_combined, master.size(),
		INTERP_METHOD, cv::BORDER_CONSTANT, white);

	candy = candy_warped_big;
}



vz::ImgCmp & vz::ImgCmp::diff_images()
{
	cv::Mat img1;
	cv::Mat img2;
	auto type = vz::Imagination::EMatType::kOriginalImage;
	if (flags & Flags::kDiffGreyscale)
	{
		type = vz::Imagination::EMatType::kO2BlurGreyscale;
	}
	if (flags & Flags::kDiffResized)
	{
		img1 = master_resized	.get(type);
		img2 = candy_resized	.get(type);
	}
	else
	{
		img1 = master_original	.get(type);
		img2 = candy_original	.get(type);
	}

	log_file.open("../log.txt", 'w');


	img1 = scale_input_img(img1);
	img2 = scale_input_img(img2);

	auto start = std::chrono::steady_clock().now();
	align_images(img1, img2);
	log_file << "Alignment took: " << std::chrono::duration{ std::chrono::steady_clock().now() - start }.count() << std::endl;;
	write_dbg("candy_aligned", img2);
	candy_aligned = img2.clone();

	start = std::chrono::steady_clock().now();
	cv::absdiff(img1, img2, differences);
	log_file << "Difference took: " << std::chrono::duration{ std::chrono::steady_clock().now() - start }.count() << std::endl;
	write_dbg("difference", differences);

	cv::Mat gray_differences;
	cv::cvtColor(differences, gray_differences, cv::COLOR_BGR2GRAY);
	write_dbg("difference_gray", gray_differences);

	cv::threshold(gray_differences, differences_threshold, 100, 255, cv::THRESH_BINARY);
	write_dbg("difference_thresh", differences_threshold);

	//cv::Mat element = cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(2, 2));
	//cv::erode(differences_threshold, differences_threshold, element);
	//write_dbg("difference_thresh_morph", differences_threshold);

	log_file.close();


	return *this;
}


vz::ImgCmp & vz::ImgCmp::get_contours()
{
	differences_contours.clear();

	for (const auto & contour : vz::find_contours(differences_threshold))
	{
		const double area = cv::contourArea(contour);
		if (area >= min_contour_area)
		{
			differences_contours.push_back(contour);
//			std::cout << "-> difference #" << differences_contours.size() << " has an area measuring " << area << " square pixels" << std::endl;
		}
		else
		{
//			std::cout << "-> skipping difference because the area only measures " << area << std::endl;
		}
	}

	return *this;
}





cv::Mat vz::ImgCmp::annotate()
{
	annotated_candy = candy_aligned.get().clone();

	if (flags & Flags::kAnnotateOverGrey)
	{
		cv::Mat mat1;
		cv::Mat mat2;
		cv::cvtColor(annotated_candy, mat1, cv::COLOR_BGR2GRAY);
		cv::cvtColor(mat1, mat2, cv::COLOR_GRAY2BGR);

		// copy only the masked differences over from the colour image into mat2
		annotated_candy.copyTo(mat2, differences_threshold);
		annotated_candy = mat2;
	}

	if (differences_contours.empty() && flags & Flags::kAnnotateAddGreenBorder)
	{
		cv::Rect r(0, 0, annotated_candy.cols, annotated_candy.rows);
		cv::rectangle(annotated_candy, r, cv::Scalar(0, 255, 0), annotation_thickness, cv::LINE_AA);
	}
	else if (differences_contours.empty() == false && flags & Flags::kAnnotateAddRedBorder)
	{
		cv::Rect r(0, 0, annotated_candy.cols, annotated_candy.rows);
		cv::rectangle(annotated_candy, r, cv::Scalar(0, 0, 255), annotation_thickness, cv::LINE_AA);
	}

	if (flags & Flags::kDrawContour)
	{
		for (auto & contour : differences_contours)
		{
			cv::polylines(annotated_candy, contour, true, annotation_colour, annotation_thickness, cv::LINE_AA);
		}
	}

	if (flags & Flags::kDrawRectangle)
	{
		for (auto & rr : vz::convert_contours_to_rotated_rects(differences_contours))
		{
			cv::Point2f points[4];
			rr.points(points);
			for (int i = 0; i < 4; i++)
			{
				cv::line(annotated_candy, points[i], points[(i + 1) % 4], annotation_colour, annotation_thickness, cv::LINE_AA);
			}
		}
	}

	write_dbg("annotated_candy", annotated_candy);

	return annotated_candy;
}
